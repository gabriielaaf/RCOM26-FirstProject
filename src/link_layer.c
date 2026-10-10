// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "alarm_action.h"
#include "control_values.h"
#include "serial_port.h"
#include "state.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256

static unsigned char expectedNs = 0;
static unsigned char txNs = 0;
static unsigned int txTimeout = 0;
static int txRetransmissions = 0;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////

static int sendSupervisionFrame(unsigned char address, unsigned char control) {

  unsigned char buf[5] = {0};

  unsigned char flag = 0x7E;

  buf[0] = flag;
  buf[1] = address;
  buf[2] = control;
  buf[3] = buf[1] ^ buf[2];
  buf[4] = flag;

  int bytes = writeBytesSerialPort(buf, 5);

  if (bytes != 5)
    return -1;

  printf("%d bytes written to serial port\n", bytes);

  return bytes;
}

static int readSupervisionFrame(unsigned char senderAddress,
                                unsigned char *control,
                                SupervisionState *state) {

  unsigned char byte;

  int result;

  do {

    result = readByteSerialPort(&byte);

    if (result == 1)
      *state = nextSupervisionState(*state, byte, senderAddress, control);

    if (*state == STOP)
      return 0;

    if (*state == INVALID)
      return -2;

  } while (result == 1);

  if (result < 0 && errno != EINTR)
    return -2;

  return -1;
}

static int readInformationFrame(unsigned char *frame, int maxSize,
                                unsigned char *control) {
  unsigned char byte;
  InformationState state = I_START;
  int frameSize = 0;

  while (1) {

    int result = readByteSerialPort(&byte);

    if (result == 0)
      continue;

    if (result < 0)
      return -1;

    InformationState previousState = state;

    state = nextInformationState(state, byte, 0x03, control);

    if (state == I_START || state == I_FLAG_RCV)
      frameSize = 0;

    if ((previousState == I_BCC_OK || previousState == I_DATA_RCV) &&
        state == I_DATA_RCV) {

      if (frameSize >= maxSize)
        return -1;

      frame[frameSize++] = byte;
    }

    if (state == I_STOP)
      return frameSize;
  }
}

static int byteStuffing(const unsigned char *frame, int frameSize,
                        unsigned char *stuffed) {

  int j = 0;

  for (int i = 0; i < frameSize; i++) {

    if (frame[i] == FLAG) {
      stuffed[j++] = ESC;
      stuffed[j++] = 0x5E;

    } else if (frame[i] == ESC) {
      stuffed[j++] = ESC;
      stuffed[j++] = 0x5D;

    } else {
      stuffed[j++] = frame[i];
    }
  }

  return j;
}

static int byteDestuffing(const unsigned char *frame, int frameSize,
                          unsigned char *destuffed, int maxSize) {
  int j = 0;

  for (int i = 0; i < frameSize; i++) {
    unsigned char byte;

    if (frame[i] == ESC) {
      if (i + 1 >= frameSize)
        return -1;

      i++;

      if (frame[i] == 0x5E)
        byte = FLAG;
      else if (frame[i] == 0x5D)
        byte = ESC;
      else
        return -1;
    } else {
      byte = frame[i];
    }

    if (j >= maxSize)
      return -1;

    destuffed[j++] = byte;
  }

  return j;
}

int llOpenTx(LinkLayer llParameters) {

  if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0) {
    perror("openSerialPort");
    return -1;
  }

  printf("Serial port %s opened\n", llParameters.serialPort);

  configureAlarm();

  int r = FALSE;

  for (int _ = 0; _ <= llParameters.nRetransmissions; _++) {

    if (sendSupervisionFrame(0x03, SET) == -1) {
      closeSerialPort();
      return -1;
    }
    startAlarm(llParameters.timeout);

    SupervisionState state = START;

    unsigned char control = 0;

    while (isAlarmEnabled()) {
      int result = readSupervisionFrame(0x03, &control, &state);

      if (result == 0) {

        if (control == UA) {
          stopAlarm();
          r = TRUE;
          break;
        }

        state = START;
      } else if (result == -2) {
        stopAlarm();
        closeSerialPort();
        return -1;
      }
    }

    if (r)
      break;
  }

  if (!r) {
    printf("Coudlnt recieve UA\n");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
    }
    return -1;
  }

  txTimeout = llParameters.timeout;
  txRetransmissions = llParameters.nRetransmissions;
  txNs = 0;

  return 0;
}

int llOpenRx(LinkLayer llParameters) {

  if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0) {
    perror("openSerialPort");
    return -1;
  }

  printf("Serial port %s opened\n", llParameters.serialPort);

  SupervisionState state = START;
  unsigned char control = 0;

  while (1) {
    int result = readSupervisionFrame(0x03, &control, &state);

    if (result == 0) {
      if (control == SET)
        break;

      state = START;
    } else if (result == -2) {
      closeSerialPort();
      return -1;
    }
  }

  if (sendSupervisionFrame(0x03, UA) == -1) {
    closeSerialPort();
    return -1;
  }

  expectedNs = 0;

  return 0;
}

////////////////////////////////////////////////
// LLSEND
////////////////////////////////////////////////
int llSend(const unsigned char *buf, int bufSize) {

  if (bufSize < 0 || bufSize > BUF_SIZE)
    return -1;

  unsigned char data[BUF_SIZE + 1];
  unsigned char stuffed[2 * (BUF_SIZE + 1)];
  unsigned char frame[5 + 2 * (BUF_SIZE + 1)];

  unsigned char bcc2 = 0;

  for (int i = 0; i < bufSize; i++) {
    data[i] = buf[i];
    bcc2 ^= data[i];
  }

  data[bufSize] = bcc2;

  int stuffSize = byteStuffing(data, bufSize + 1, stuffed);

  frame[0] = FLAG;
  frame[1] = 0x03;
  frame[2] = (txNs == 0) ? I0 : I1;
  frame[3] = frame[1] ^ frame[2];

  for (int i = 0; i < stuffSize; i++)
    frame[i + 4] = stuffed[i];

  frame[4 + stuffSize] = FLAG;
  int frameSize = 5 + stuffSize;

  unsigned char expectedAck = (txNs == 0) ? RR1 : RR0;
  unsigned char reject = (txNs == 0) ? REJ0 : REJ1;

  for (int attempt = 0; attempt <= txRetransmissions; attempt++) {
    int bytes = writeBytesSerialPort(frame, frameSize);

    if (bytes != frameSize)
      return -1;

    startAlarm(txTimeout);
    SupervisionState state = START;
    unsigned char control = 0;

    while (isAlarmEnabled()) {
      int result = readSupervisionFrame(0x01, &control, &state);

      if (result == -2) {
        stopAlarm();
        return -1;
      }

      if (result == 0) {
        if (control == expectedAck) {
          stopAlarm();
          txNs ^= 1;
          return bufSize;
        }

        if (control == reject) {
          stopAlarm();
          break;
        }

        state = START;
      }
    }

    stopAlarm();
  }

  return -1;
}

////////////////////////////////////////////////
// LLRECEIVE
////////////////////////////////////////////////
int llReceive(unsigned char *packet) {

  unsigned char frame[2 * (BUF_SIZE + 1)];
  unsigned char control = 0;
  unsigned char deStuffed[BUF_SIZE + 1];

  while (1) {
    int frameSize = readInformationFrame(frame, sizeof(frame), &control);

    if (frameSize < 0)
      return -1;

    unsigned char recievedNs = ((control >> 6) & 1);

    if (recievedNs != expectedNs) {
      unsigned char rr = (expectedNs == 0 ? RR0 : RR1);

      if (sendSupervisionFrame(0x01, rr) <= 0)
        return -1;

      continue;
    }

    int deStuffedSize =
        byteDestuffing(frame, frameSize, deStuffed, sizeof(deStuffed));
    unsigned char rej = (expectedNs == 0 ? REJ0 : REJ1);

    if (deStuffedSize < 1) {
      if (sendSupervisionFrame(0x01, rej) <= 0)
        return -1;

      continue;
    }

    unsigned char calculateBCC2 = 0;

    for (int i = 0; i < deStuffedSize - 1; i++)
      calculateBCC2 ^= deStuffed[i];

    if (calculateBCC2 != deStuffed[deStuffedSize - 1]) {
      if (sendSupervisionFrame(0x01, rej) <= 0)
        return -1;

      continue;
    }

    int packetSize = deStuffedSize - 1;

    for (int i = 0; i < packetSize; i++)
      packet[i] = deStuffed[i];

    expectedNs ^= 1;

    unsigned char rr = (expectedNs == 0 ? RR0 : RR1);

    if (sendSupervisionFrame(0x01, rr) <= 0)
      return -1;

    return packetSize;
  }
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llCloseTx() {

  configureAlarm();

  unsigned char control;

  int r = 0;

  for (int _ = 0; _ <= txRetransmissions; _++) {

    if (sendSupervisionFrame(0x03, DISC) < 0) {
      return -1;
    }

    startAlarm(txTimeout);
    SupervisionState state = START;

    while (isAlarmEnabled()) {

      int result = readSupervisionFrame(0x01, &control, &state);

      if (result == 0) {

        if (control == DISC) {
          stopAlarm();
          r = 1;
          break;
        }

        state = START;
      } else if (result == -2) {
        stopAlarm();
        closeSerialPort();
        return -1;
      }
    }

    if (r) break;

  }

  if (!r) {

    printf("Coudlnt recieve DISC\n");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
    }
    return -1;
  }

  return closeSerialPort();
}

int llCloseRx() {
  // TODO: Implement this function

  return 0;
}

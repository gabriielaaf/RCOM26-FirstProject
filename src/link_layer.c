// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "alarm_action.h"
#include "control_values.h"
#include "serial_port.h"
#include "state.h"
#include <stdio.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256

static unsigned char expectedNs = 0;

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
                                unsigned char senderControl,
                                SupervisionState *state) {

  unsigned char byte;

  int result;

  do {

    result = readByteSerialPort(&byte);

    if (result == 1)
      *state = nextSupervisionState(state, byte, senderAddress, senderControl);

    if (*state == STOP)
      return 0;

    if (*state == INVALID)
      return -1;

  } while (result == 1);

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
                          unsigned char *destuffed) {
  int j = 0;

  for (int i = 0; i < frameSize; i++) {

    if (frame[i] == ESC) {

      if (i + 1 >= frameSize)
        return -1;

      i++;

      if (frame[i] == 0x5E)
        destuffed[j++] = 0x7E;

      else if (frame[i] == 0x5D)
        destuffed[j++] = ESC;

      else
        return -1;

    } else {
      destuffed[j++] = frame[i];
    }
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

    while (isAlarmEnabled()) {
      if (readSupervisionFrame(0x03, UA, &state) == 0) {
        stopAlarm();
        r = TRUE;
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

  return 0;
}

int llOpenRx(LinkLayer llParameters) {

  if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0) {
    perror("openSerialPort");
    return -1;
  }

  printf("Serial port %s opened\n", llParameters.serialPort);

  SupervisionState state = START;

  do {

  } while (readSupervisionFrame(0x03, SET, &state) == -1);

  if (sendSupervisionFrame(0x03, UA) == -1) {
    closeSerialPort();
    return -1;
  }

  return 0;
}

////////////////////////////////////////////////
// LLSEND
////////////////////////////////////////////////
int llSend(const unsigned char *buf, int bufSize) {
  // TODO: Implement this function

  return 0;
}

////////////////////////////////////////////////
// LLRECEIVE
////////////////////////////////////////////////
int llReceive(unsigned char *packet) {

  unsigned char frame[BUF_SIZE];
  unsigned char control = 0;
  unsigned char deStuffed[BUF_SIZE];

  while (1) {

    int frameSize = readInformationFrame(frame, BUF_SIZE, &control);

    if (frameSize < 0)
      return -1;

    unsigned char recievedNs = ((control >> 6) & 1);

    if (recievedNs != expectedNs) {
      unsigned char rr = (expectedNs == 0 ? RR0 : RR1);

      if (sendSupervisionFrame(0x01, rr) <= 0) return -1;

      continue;
    }

    int deStuffedSize = byteDestuffing(frame, frameSize, deStuffed);

    unsigned char rej = (expectedNs == 0 ? REJ0 : REJ1);

    if (deStuffedSize < 1) {

      if (sendSupervisionFrame(0x01, rej) <= 0) return -1;

      continue;
    }

    unsigned char calculateBCC2 = 0;

    for (int i = 0; i < deStuffedSize - 1; i++) {
      calculateBCC2 ^= deStuffed[i];
    }

    if (calculateBCC2 != deStuffed[deStuffedSize - 1]) {

      if (sendSupervisionFrame(0x01, rej) <= 0) return -1;
      continue;
    }

    int packetSize = deStuffedSize - 1;

    for (int i = 0; i < packetSize; i++) {
      packet[i] = deStuffed[i];
    }

    expectedNs ^= 1;

    unsigned char rr = (expectedNs == 0 ? RR0 : RR1);

    if (sendSupervisionFrame(0x01, rr) <= 0)
      return - 1;

    return packetSize;
  }

  return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llCloseTx() {
  // TODO: Implement this function

  return 0;
}

int llCloseRx() {
  // TODO: Implement this function

  return 0;
}

// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

#include "alarm_action.h"
#include <stdio.h>
#include <unistd.h>
#define SET 0x03
#define UA 0x07

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256

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
                                unsigned char senderControl) {

  unsigned char response[5];
  int flag = 0x7E;

  for (int i = 0; i < 5;) {
    int result = readByteSerialPort(&response[i]);

    if (result < 0) {
      return -1;
    }

    if (result == 1)
      i++;
  }

  if (response[0] == flag && response[4] == flag &&
      response[1] == senderAddress && response[2] == senderControl &&
      response[3] == (response[1] ^ response[2])) {
    return 0;
  }

  return -1;
}

int llOpenTx(LinkLayer llParameters) {

  if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0) {
    perror("openSerialPort");
    return -1;
  }

  printf("Serial port %s opened\n", llParameters.serialPort);

  if (sendSupervisionFrame(0x03, SET) == -1) {
    closeSerialPort();
    return -1;
  }

  configureAlarm();

  int r = FALSE;

  for (int _ = 0; _ < 3; _++) {
    startAlarm(3);

    while (isAlarmEnabled) {
      if (readSupervisionFrame(0x03, UA) == 0) {
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
      return -1;
    }
  } else {
    printf("Successfully read UA\n");
  }

  printf("Serial port %s closed\n", llParameters.serialPort);

  return 0;
}

int llOpenRx(LinkLayer llParameters) {

  if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0) {
    perror("openSerialPort");
    return -1;
  }

  printf("Serial port %s opened\n", llParameters.serialPort);


  do {

  } while (readSupervisionFrame(0x03, SET) == -1);
  

  if (sendSupervisionFrame(0x03, UA) == -1) {
    closeSerialPort();
    return -1;
  }

  // Close serial port
  if (closeSerialPort() < 0) {
    perror("closeSerialPort");
    return -1;
  }

  printf("Serial port %s closed\n", llParameters.serialPort);

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
  // TODO: Implement this function

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

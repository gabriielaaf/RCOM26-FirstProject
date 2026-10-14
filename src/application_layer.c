// RCOM 2026/2027
//
// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include "packet_values.h"

#include <stdio.h>
#include <string.h>

int byteSize(unsigned long num) {
  int numBytes = sizeof(unsigned long) - 1;

  while (numBytes > 0 && !((num) >> (numBytes * 8)) & 0x0FF) {
    numBytes--;
  }

  numBytes++;
  return numBytes;
}

int buildControlPacket(unsigned char *packet,
                       unsigned char control, const char *filename,
                       long fileSize) {

  int numBytes = byteSize(fileSize);

  packet[0] = control;

  packet[1] = FILE_SIZE;

  packet[2] = numBytes;

  for (int i = 0; i < numBytes; i++) {

    packet[i + 3] = ((fileSize >> (numBytes - i - 1) * 8) & 0x0FF);
  }

  int offset = 3 + numBytes;

  packet[offset++] = FILE_NAME;

  int nameLength = strlen(filename);

  packet[offset++] = nameLength;

  memcpy(packet + offset, filename, nameLength);

  offset += nameLength;

  return offset;
}

int buildDataPacket(unsigned char *packet,unsigned char *data, long dataSize) {

    unsigned char L1 = (dataSize & 0x0FF);
    unsigned char L2 = ((dataSize >> 8) & 0x0FF);


    packet[0] = DATA_PACKET;
    packet[1] = L2;
    packet[2] = L1;

    for (int i = 0; i<dataSize; i++){
        packet[i+3] = data[i];
    }


    return 3 + dataSize;

}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {
  LinkLayer llParameters = {
      .baudRate = baudRate,
      .nRetransmissions = nTries,
      .timeout = timeout,
  };

  strcpy(llParameters.serialPort, serialPort);

  unsigned char packet[] = {0x01, 0x7E, 0x02, 0x7D, 0x7E,
                            0x7D, 0x00, 0xFF, 0x7E, 0x7D};

  int packetSize = sizeof(packet);

  if (strcmp(role, "tx") == 0) {
    if (llOpenTx(llParameters) < 0)
      return;

    for (int i = 0; i < 2; i++) {
      if (llSend(packet, packetSize) < 0)
        printf("Error sending packet %d\n", i + 1);
      else
        printf("Packet %d sent successfully!\n", i + 1);
    }

    llCloseTx();
  } else if (strcmp(role, "rx") == 0) {
    if (llOpenRx(llParameters) < 0)
      return;

    for (int i = 0; i < 2; i++) {
      unsigned char received[256];

      int size = llReceive(received);

      if (size < 0) {
        printf("Error receiving packet %d\n", i + 1);
        break;
      }

      printf("Received packet %d (%d bytes):\n", i + 1, size);

      for (int j = 0; j < size; j++)
        printf("%02X ", received[j]);

      printf("\n");
    }

    llCloseRx();
  } else {
    printf("Invalid role: %s. Must be 'tx' or 'rx'.\n", role);
    return;
  }
}
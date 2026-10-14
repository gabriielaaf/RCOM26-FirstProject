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

  while (numBytes > 0 && ((num >> (numBytes * 8)) & 0x0FF) == 0){
    numBytes--;
  }

  return numBytes + 1;
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

  if (strcmp(role, "tx") == 0) {
    
    // Open the file

    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Get file size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(file);
        return;
    }

    long fileSize = ftell(file);

    if (fileSize < 0) {
        perror("ftell");
        fclose(file);
        return;
    }

    rewind(file);

    printf("File size: %ld bytes\n", fileSize);

    // Filename length must fit in one byte
    if (strlen(filename) > 255) {
        printf("Filename too long\n");
        fclose(file);
        return;
    }

    // Build START packet

    unsigned char packet[MAX_PAYLOAD_SIZE];

    int packetSize = buildControlPacket(
        packet,
        PACKET_START,
        filename,
        fileSize
    );

    // Open connection

    if (llOpenTx(llParameters) < 0) {
        fclose(file);
        return;
    }

    // Send START packet

    if (llSend(packet, packetSize) < 0) {
        printf("Error sending START packet\n");
    } else {
        printf("START packet sent successfully!\n");

        // Read first fragment of the file
        unsigned char data[MAX_PAYLOAD_SIZE - 3];

        size_t bytesRead = fread(data, 1, sizeof(data), file);

        if (ferror(file)) {
            perror("Error reading file");
        } else if (bytesRead > 0) {

            printf("Read %zu bytes from file\n", bytesRead);

            // Build DATA packet
            unsigned char dataPacket[MAX_PAYLOAD_SIZE];

            int dataPacketSize = buildDataPacket(
                dataPacket,
                data,
                (long)bytesRead
            );

            // Send DATA packet
            if (llSend(dataPacket, dataPacketSize) < 0) {
                printf("Error sending DATA packet\n");
            } else {
                printf("DATA packet sent successfully!\n");
            }
        }
    }

    fclose(file);

    llCloseTx();

  } else if (strcmp(role, "rx") == 0) {

    if (llOpenRx(llParameters) < 0)
      return;

    unsigned char packet[MAX_PAYLOAD_SIZE];

    int packetSize = llReceive(packet);

    if (packetSize < 0) {
        printf("Error receiving packet\n");
    }
        else if (packetSize > 0 && packet[0] == PACKET_START) {

        printf("START packet received successfully!\n");
        printf("Packet size: %d bytes\n", packetSize);

        // Receive DATA packet
        int dataPacketSize = llReceive(packet);

        if (dataPacketSize < 3 || packet[0] != DATA_PACKET) {
            printf("Error receiving DATA packet\n");
        }
        else {

            // Get data size
            int dataSize = 256 * packet[1] + packet[2];

            if (dataSize != dataPacketSize - 3) {
                printf("Invalid DATA packet size\n");
            }
            else {

                // Open output file
                FILE *file = fopen(filename, "wb");

                if (file == NULL) {
                    perror("Error creating file");
                }
                else {

                    // Write received data
                    size_t written = fwrite(packet + 3, 1, dataSize, file);

                    if (written != (size_t)dataSize) {
                        printf("Error writing file\n");
                    }
                    else {
                        printf("DATA packet received successfully!\n");
                        printf("%d bytes written to file\n", dataSize);
                    }

                    fclose(file);
                }
            }
        }
    }
    else {
        printf("Unexpected packet received\n");
    }

    llCloseRx();

  } else {

    printf("Invalid role: %s. Must be 'tx' or 'rx'.\n", role);
    return;
  }
}
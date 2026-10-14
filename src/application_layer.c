// RCOM 2026/2027
//
// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

#include "packets.h"
#include "packet_values.h"

#include <stdio.h>
#include <string.h>

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

        fclose(file);
        llCloseTx();
        return;
    }

    printf("START packet sent successfully!\n");

    // Buffers for DATA packets
    unsigned char data[MAX_PAYLOAD_SIZE - 3];
    unsigned char dataPacket[MAX_PAYLOAD_SIZE];

    size_t bytesRead;

    // Read and send the entire file
    while ((bytesRead = fread(data, 1, sizeof(data), file)) > 0) {

        // Build DATA packet
        int dataPacketSize = buildDataPacket(
            dataPacket,
            data,
            (long)bytesRead
        );

        // Send DATA packet
        if (llSend(dataPacket, dataPacketSize) < 0) {
            printf("Error sending DATA packet\n");
            break;
        }

        printf("DATA packet sent successfully! (%zu bytes)\n", bytesRead);
    }

    // Check for file reading errors
    if (ferror(file)) {
        perror("Error reading file");
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

        // Validate START packet
        if (packetSize < 4 ||
            packet[1] != FILE_SIZE ||
            packet[2] == 0 ||
            packet[2] > sizeof(unsigned long) ||
            packetSize < 3 + packet[2]) {

            printf("Invalid START packet\n");

        } else {

            // Get file size from START packet
            unsigned long fileSize = 0;

            for (int i = 0; i < packet[2]; i++) {
                fileSize = (fileSize << 8) | packet[3 + i];
            }

            printf("Expected file size: %lu bytes\n", fileSize);

            // Open output file
            FILE *file = fopen(filename, "wb");

            if (file == NULL) {
                perror("Error creating file");
            } else {

                unsigned long totalReceived = 0;
                int error = 0;

                // Receive DATA packets until the file is complete
                while (totalReceived < fileSize) {

                    int dataPacketSize = llReceive(packet);

                    if (dataPacketSize < 3 || packet[0] != DATA_PACKET) {
                        printf("Error receiving DATA packet\n");
                        error = 1;
                        break;
                    }

                    // Get the number of data bytes
                    int dataSize = 256 * packet[1] + packet[2];

                    // Validate packet size
                    if (dataSize <= 0 ||
                        dataSize != dataPacketSize - 3 ||
                        (unsigned long)dataSize > fileSize - totalReceived) {

                        printf("Invalid DATA packet size\n");
                        error = 1;
                        break;
                    }

                    // Write the received fragment
                    size_t written = fwrite(packet + 3, 1, dataSize, file);

                    if (written != (size_t)dataSize) {
                        printf("Error writing file\n");
                        error = 1;
                        break;
                    }

                    totalReceived += dataSize;

                    printf("Received %d bytes (Total: %lu/%lu)\n",
                        dataSize, totalReceived, fileSize);
                }

                if (fclose(file) != 0) {
                    perror("Error closing output file");
                    error = 1;
                }

                if (!error) {
                    printf("File received successfully!\n");
                    printf("Total bytes received: %lu\n", totalReceived);
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
// RCOM 2026/2027
//
// Application layer packet construction

#include "packets.h"
#include "packet_values.h"

#include <string.h>

// Calculate the number of bytes needed to represent a number

int byteSize(unsigned long num) {

    int numBytes = sizeof(unsigned long) - 1;

    while (numBytes > 0 && ((num >> (numBytes * 8)) & 0xFF) == 0) {
        numBytes--;
    }

    return numBytes + 1;
}


// Build START or END packet

int buildControlPacket(unsigned char *packet,
                       unsigned char control,
                       const char *filename,
                       long fileSize) {

    int numBytes = byteSize(fileSize);

    packet[0] = control;

    packet[1] = FILE_SIZE;

    packet[2] = numBytes;

    for (int i = 0; i < numBytes; i++) {
        packet[i + 3] =
            (fileSize >> ((numBytes - i - 1) * 8)) & 0xFF;
    }

    int offset = 3 + numBytes;

    packet[offset++] = FILE_NAME;

    int nameLength = strlen(filename);

    packet[offset++] = nameLength;

    memcpy(packet + offset, filename, nameLength);

    offset += nameLength;

    return offset;
}


// Build DATA packet

int buildDataPacket(unsigned char *packet,
                    unsigned char *data,
                    long dataSize) {

    unsigned char L1 = dataSize & 0xFF;
    unsigned char L2 = (dataSize >> 8) & 0xFF;

    packet[0] = DATA_PACKET;
    packet[1] = L2;
    packet[2] = L1;

    for (int i = 0; i < dataSize; i++) {
        packet[i + 3] = data[i];
    }

    return 3 + dataSize;
}
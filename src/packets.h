#ifndef PACKETS_H
#define PACKETS_H

// Calculate the number of bytes needed to represent a number
int byteSize(unsigned long num);

// Build a START or END control packet
int buildControlPacket(unsigned char *packet,
                       unsigned char control,
                       const char *filename,
                       long fileSize);

// Build a DATA packet
int buildDataPacket(unsigned char *packet,
                    unsigned char *data,
                    long dataSize);

#endif
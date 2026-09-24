// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

#include <stdio.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////

static void createBuffer(unsigned char flag, unsigned char address, unsigned char control, unsigned char (*buf)[5])
{

    (*buf)[0] = flag;
    (*buf)[1] = address;
    (*buf)[2] = control;
    (*buf)[3] = (*buf)[1] ^ (*buf)[2];
    (*buf)[4] = flag;
}

int llOpenTx(LinkLayer llParameters)
{

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);

    unsigned char buf[5];

    createBuffer(0x7E, 0x03, 0x03, &buf);

    // 7E0303007E

    int bytes = writeBytesSerialPort(buf, 5);
    printf("%d bytes written to serial port\n", bytes);

    // Wait until all bytes have been written to the serial port
    sleep(1);

    unsigned char response[5];

    for (int i = 0; i < 5;)
    {

        unsigned char byte;
        int bytes = readByteSerialPort(&byte);
        if (bytes != 1)
            continue;

        response[i] = byte;

        i++;
    }

    if (!((response[0] == 0x7E) && (response[1] == 0x03) && (response[2] == 0x07) && (response[3] == (response[1] ^ response[2])) && (response[4] == 0x7E)))
    {
        return -1;
    }

    // Close serial port
    if (closeSerialPort() < 0)
    {
        perror("closeSerialPort");
        return -1;
    }

    printf("Serial port %s closed\n", llParameters.serialPort);

    return 0;
}

int llOpenRx(LinkLayer llParameters)
{

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);


    unsigned char response[5];

    for (int i = 0; i < 5;)
    {

        unsigned char byte;
        int bytes = readByteSerialPort(&byte);
        if (bytes != 1)
            continue;

        response[i] = byte;

        i++;
    }

    if (!((response[0] == 0x7E) && (response[1] == 0x03) && (response[2] == 0x03) && (response[3] == (response[1] ^ response[2])) && (response[4] == 0x7E)))
    {
        return -1;
    }

    unsigned char buf[5];
    createBuffer(0x7E, 0x03, 0x07, &buf);
    writeBytesSerialPort(buf, 5);


    // Close serial port
    if (closeSerialPort() < 0)
    {
        perror("closeSerialPort");
        return -1;
    }

    printf("Serial port %s closed\n", llParameters.serialPort);

    return 0;
}

////////////////////////////////////////////////
// LLSEND
////////////////////////////////////////////////
int llSend(const unsigned char *buf, int bufSize)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLRECEIVE
////////////////////////////////////////////////
int llReceive(unsigned char *packet)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llCloseTx()
{
    // TODO: Implement this function

    return 0;
}

int llCloseRx()
{
    // TODO: Implement this function

    return 0;
}

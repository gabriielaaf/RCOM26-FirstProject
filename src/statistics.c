#define _POSIX_C_SOURCE 200809L

#include "statistics.h"

#include <stdio.h>
#include <time.h>

static unsigned int txFrames = 0;
static unsigned int txRetransmissions = 0;
static unsigned int txTimeouts = 0;
static unsigned int txRejs = 0;

static int txTransferSuccess = 0;

static unsigned int rxFrames = 0;
static unsigned int rxDuplicates = 0;
static unsigned int rxErrors = 0;
static unsigned int rxRejs = 0;

static int rxTransferSuccess = 0;

static struct timespec txStartTime;
static double txElapsedTime = 0.0;
static long txFileSize = 0;

static unsigned long long txBytes = 0;
static unsigned long long rxAckBytes = 0;

void resetTxStatistics(void) {
    txFrames = 0;
    txRetransmissions = 0;
    txTimeouts = 0;
    txRejs = 0;
    txElapsedTime = 0.0;
    txFileSize = 0;
    txBytes = 0;
    txTransferSuccess = 0;
}

void countTxFrame(void) {
    txFrames++;
}

void countTxRetransmission(void) {
    txRetransmissions++;
}

void countTxTimeout(void) {
    txTimeouts++;
}

void countTxRej(void) {
    txRejs++;
}

void setTxTransferStatus(int success) {
    txTransferSuccess = success;
}

void printTxStatistics(void) {

    printf("\n===== TRANSMISSION STATISTICS =====\n");

    printf("Transfer status: %s\n",
           txTransferSuccess ? "SUCCESS" : "FAILED");

    printf("Information frames sent: %u\n", txFrames);
    printf("Retransmissions: %u\n", txRetransmissions);
    printf("Timeouts: %u\n", txTimeouts);
    printf("REJ received: %u\n", txRejs);

    if (txElapsedTime > 0.0) {
        double throughput = (txFileSize * 8.0) / txElapsedTime;

        printf("Transfer time: %.3f s\n", txElapsedTime);
        printf("Useful throughput: %.2f bit/s\n", throughput);
    }

    printf("Information bytes transmitted: %llu\n", txBytes);

    if (txBytes > 0 && txFileSize > 0) {
        double efficiency = (txFileSize * 100.0) / txBytes;

        printf("Forward-channel efficiency: %.2f%%\n", efficiency);
    }

    printf("===================================\n\n");
}

void resetRxStatistics(void) {
    rxFrames = 0;
    rxDuplicates = 0;
    rxErrors = 0;
    rxRejs = 0;
    rxAckBytes = 0;
    rxTransferSuccess = 0;
}

void countRxFrame(void) {
    rxFrames++;
}

void countRxDuplicate(void) {
    rxDuplicates++;
}

void countRxError(void) {
    rxErrors++;
}

void countRxRej(void) {
    rxRejs++;
}

void setRxTransferStatus(int success) {
    rxTransferSuccess = success;
}

void printRxStatistics(void) {

    printf("\n===== RECEPTION STATISTICS =====\n");

    printf("Transfer status: %s\n",
           rxTransferSuccess ? "SUCCESS" : "FAILED");

    printf("Information frames accepted: %u\n", rxFrames);
    printf("Duplicate frames: %u\n", rxDuplicates);
    printf("BCC2 errors: %u\n", rxErrors);
    printf("REJ frames sent: %u\n", rxRejs);
    printf("Acknowledgement bytes transmitted: %llu\n", rxAckBytes);

    printf("================================\n\n");
}

void startTxTimer(void) {
    clock_gettime(CLOCK_MONOTONIC, &txStartTime);
}

void stopTxTimer(long fileSize) {
    struct timespec endTime;

    clock_gettime(CLOCK_MONOTONIC, &endTime);

    txElapsedTime =
        (endTime.tv_sec - txStartTime.tv_sec) +
        (endTime.tv_nsec - txStartTime.tv_nsec) / 1000000000.0;

    txFileSize = fileSize;
}

void countTxBytes(int bytes) {
    txBytes += bytes;
}

void countRxAckBytes(int bytes) {
    rxAckBytes += bytes;
}
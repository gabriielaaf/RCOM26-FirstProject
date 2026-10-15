#include "statistics.h"

#include <stdio.h>

static unsigned int txFrames = 0;
static unsigned int txRetransmissions = 0;
static unsigned int txTimeouts = 0;
static unsigned int txRejs = 0;

static unsigned int rxFrames = 0;
static unsigned int rxDuplicates = 0;
static unsigned int rxErrors = 0;

void resetTxStatistics(void) {
    txFrames = 0;
    txRetransmissions = 0;
    txTimeouts = 0;
    txRejs = 0;
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

void printTxStatistics(void) {

    printf("\n===== TRANSMISSION STATISTICS =====\n");

    printf("Information frames sent: %u\n", txFrames);
    printf("Retransmissions: %u\n", txRetransmissions);
    printf("Timeouts: %u\n", txTimeouts);
    printf("REJ received: %u\n", txRejs);

    printf("===================================\n\n");
}

void resetRxStatistics(void) {
    rxFrames = 0;
    rxDuplicates = 0;
    rxErrors = 0;
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

void printRxStatistics(void) {
    printf("\n===== RECEPTION STATISTICS =====\n");
    printf("Information frames accepted: %u\n", rxFrames);
    printf("Duplicate frames: %u\n", rxDuplicates);
    printf("BCC2 errors: %u\n", rxErrors);
    printf("================================\n\n");
}
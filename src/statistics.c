#include "statistics.h"

#include <stdio.h>

// Transmission statistics
static unsigned int txFrames = 0;
static unsigned int txRetransmissions = 0;
static unsigned int txTimeouts = 0;
static unsigned int txRejs = 0;

// Initialize statistics
void resetTxStatistics(void) {
    txFrames = 0;
    txRetransmissions = 0;
    txTimeouts = 0;
    txRejs = 0;
}

// Count transmitted information frames
void countTxFrame(void) {
    txFrames++;
}

// Count retransmissions
void countTxRetransmission(void) {
    txRetransmissions++;
}

// Count timeouts
void countTxTimeout(void) {
    txTimeouts++;
}

// Count received REJ frames
void countTxRej(void) {
    txRejs++;
}

// Display statistics
void printTxStatistics(void) {

    printf("\n===== TRANSMISSION STATISTICS =====\n");

    printf("Information frames sent: %u\n", txFrames);
    printf("Retransmissions: %u\n", txRetransmissions);
    printf("Timeouts: %u\n", txTimeouts);
    printf("REJ received: %u\n", txRejs);

    printf("===================================\n\n");
}
#ifndef STATISTICS_H
#define STATISTICS_H

// Initialize transmission statistics
void resetTxStatistics(void);

// Update transmission statistics
void countTxFrame(void);
void countTxRetransmission(void);
void countTxTimeout(void);
void countTxRej(void);

// Display transmission statistics
void printTxStatistics(void);

#endif
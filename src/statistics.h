#ifndef STATISTICS_H
#define STATISTICS_H

// Transmiter statistics

void resetTxStatistics(void);

void countTxFrame(void);
void countTxRetransmission(void);
void countTxTimeout(void);
void countTxRej(void);

void printTxStatistics(void);

// Receiver statistics

void resetRxStatistics(void);

void countRxFrame(void);
void countRxDuplicate(void);
void countRxError(void);

void printRxStatistics(void);

// Transfer time

void startTxTimer(void);
void stopTxTimer(long fileSize);

#endif
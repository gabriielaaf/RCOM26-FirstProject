#ifndef STATISTICS_H
#define STATISTICS_H

// Transmitter statistics

void resetTxStatistics(void);

void countTxFrame(void);
void countTxRetransmission(void);
void countTxTimeout(void);
void countTxRej(void);

void setTxTransferStatus(int success);

void printTxStatistics(void);

// Receiver statistics

void resetRxStatistics(void);

void countRxFrame(void);
void countRxDuplicate(void);
void countRxError(void);
void countRxRej(void);

void setRxTransferStatus(int success);

void printRxStatistics(void);

// Transfer time

void startTxTimer(void);
void stopTxTimer(long fileSize);

// Transmission byte statistics

void countTxBytes(int bytes);
void countRxAckBytes(int bytes);

#endif
#ifndef ALARM_ACTION_H
#define ALARM_ACTION_H

#define FALSE 0
#define TRUE 1

void configureAlarm(void);

void startAlarm(unsigned int seconds);

void stopAlarm(void);

int isAlarmEnabled(void);

#endif
#define _POSIX_C_SOURCE 200809L

#include "alarm_action.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t alarmEnabled = FALSE;
static volatile sig_atomic_t alarmCount = 0;

void startAlarm(unsigned int seconds) {
  alarmEnabled = TRUE;
  alarm(seconds);
}

void stopAlarm(void) {
  alarm(0);
  alarmEnabled = FALSE;
}

static void alarmHandler(int signal) {
  alarmEnabled = FALSE;
  alarmCount++;

  printf("Alarm #%d received\n", alarmCount);
}

void configureAlarm(void) {
  struct sigaction act = {0};

  act.sa_handler = &alarmHandler;

  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("Alarm configured\n");
}

int isAlarmEnabled(void) { return alarmEnabled; }
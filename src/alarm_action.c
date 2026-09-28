#include "alarm_action.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int alarmEnabled = FALSE;
int alarmCount = 0;

static void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d received\n", alarmCount);
}

void configureAlarm(void)
{
    struct sigaction act = {0};

    act.sa_handler = &alarmHandler;

    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("Alarm configured\n");
}


int isAlarmEnabled(void)
{
    return alarmEnabled;
}
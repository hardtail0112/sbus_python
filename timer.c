/*
 *  sample program
 *  interval timer (SIGALRM)
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

void SignalHandler(int);
int _nanosleep(int, int);
int main(void)
{
    struct sigaction action;
    struct itimerval timer;

    printf("sample program(%s) start\n", __FILE__);
    memset(&action, 0, sizeof(action));

    /* set signal handler */
    action.sa_handler = SignalHandler;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    if(sigaction(SIGALRM, &action, NULL) < 0){
        perror("sigaction error");
        exit(1);
    }

    /* set intarval timer (10ms) */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &timer, NULL) < 0){
        perror("setitimer error");
        exit(1);
    }

    /* loop */
    while(1){
        _nanosleep(1, 0);    /* sleep 1 sec */
    }
    return 0;
}
void SignalHandler(int signum)
{
    static unsigned long msec_cnt = 0;

    msec_cnt++;
    if(!(msec_cnt % 100)){
        printf("SignalHandler:%lu sec\n", (msec_cnt / 100));
    }
    return;
}
int _nanosleep(int sec, int nsec)
{
    struct timespec req, rem;

    req.tv_sec = sec;
    req.tv_nsec = nsec;
    rem.tv_sec = 0;
    rem.tv_nsec = 0;
    while(nanosleep(&req, &rem)){
        if(errno == EINTR){
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }else{
            perror("nanosleep error");
            return -1;
        }
    }
    return 0;
}
#ifndef TSIGNAL_H
#define TSIGNAL_H

#include <pthread.h>
#include <signal.h>
int block_signals();
int unblock_signals();
void* sighandler_thread(void* arg);
int setup_watchdog();
void stack_trace(int sig);
void hsighandler(int sig);
#endif

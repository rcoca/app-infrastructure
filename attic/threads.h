#ifndef THREADS_H
#define THREADS_H
#include <pthread.h>
#include "tsignal.h"
#include "queue.h"
void* thread_factory(void *arg);
typedef void * (ThreadFunc)(void *) ;
pthread_t startThreadsFactory();
int stopThreadsFactory(pthread_t tid);
pthread_t startThread(void * (func)(void *) ,void* args);
int stopThread(pthread_t tid,int force);
#endif

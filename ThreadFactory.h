#ifndef _THREAD_FACTORY_H
#define _THREAD_FACTORY_H
#include <pthread.h>
#include "tsignal.h"

typedef enum {
  Create=0,
  Destroy=1,
  Exit=3,
  rCreate=4,
  rDestroy=5,
  rExit=6,
} TFReqRplType;

typedef struct _TFRQM{
  TFReqRplType msgtype;
  union 
  {
    void *thread[2];
    int retcode;
    pthread_t tid;
  }params;
  int force;  
} TFReqRplMessage;

pthread_t startThreadsFactory();
int stopThreadsFactory(pthread_t tid);
pthread_t startThread(void * (func)(void *) ,void* args);
int stopThread(pthread_t tid,int force);
void* Thread_Factory(void *arg);

#define TF_QUEUE_SIZE 10

#endif

#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>

#include "qmessages.pb-c.h"

typedef struct _Qmessage Message;

#define INIT_QUEUE(bList) {				\
      .List = bList,					\
      .capacity = sizeof(bList) / sizeof(bList[0]),	\
      .size = 0,					\
      .start = 0,					\
      .end = 0,						\
      .mutex  = PTHREAD_MUTEX_INITIALIZER,			\
      .full  = PTHREAD_COND_INITIALIZER,			\
      .empty = PTHREAD_COND_INITIALIZER }

typedef struct _Queue
{
  Message *List;
  const int capacity;
  int size;
  int start;
  int end;
  pthread_mutex_t mutex;
  pthread_cond_t full;
  pthread_cond_t empty;
} Queue;

int queue_put(Queue *queue, Message msg);
int queue_get(Queue *queue,Message* pmsg);
int queue_size(Queue *queue);
int queue_close(Queue *queue);
#endif

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h> 

#include "queue.h"
#include "logs.h"
#define QUEUE_TEST 1

int queue_put(Queue *queue, Message msg)
{
        pthread_mutex_lock(&(queue->mutex));
        while (queue->size == queue->capacity)
                pthread_cond_wait(&(queue->empty), &(queue->mutex));
        /* #ifdef QUEUE_TEST */
        //LOG_DEBUG("%d ", msg.req); 
	/* #endif */
        queue->List[queue->start] = msg;
        ++ queue->size;
        ++ queue->start;
        queue->start %= queue->capacity;
        pthread_mutex_unlock(&(queue->mutex));
        pthread_cond_broadcast(&(queue->full));
	return 1;
}

int queue_get(Queue *queue,Message* pmsg)
{
        pthread_mutex_lock(&(queue->mutex));
        while (queue->size == 0)
                pthread_cond_wait(&(queue->full), &(queue->mutex));
        *pmsg = queue->List[queue->end];
        /* #ifdef QUEUE_TEST */
        //LOG_DEBUG("%d ", pmsg->req);
	/* #endif */
        -- queue->size;
        ++ queue->end;
        queue->end %= queue->capacity;
        pthread_mutex_unlock(&(queue->mutex));
        pthread_cond_broadcast(&(queue->empty));
        return 1;
}

int queue_size(Queue *queue)
{
        pthread_mutex_lock(&(queue->mutex));
        int size = queue->size;
        pthread_mutex_unlock(&(queue->mutex));
        return size;
}

int queue_close(Queue *queue)
{
  
  pthread_cond_destroy(&(queue->empty));
  pthread_cond_destroy(&(queue->full));
  pthread_mutex_destroy(&(queue->mutex));
  /*could add code to notify waiters*/
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h> 
#include "queue.h"
#include "ThreadFactory.h"
#include "logs.h"
#ifdef QUEUE_TEST

const int buffer_size = 10;
const int producers = 2;
const int producer_amount = 10;
const int consumers = 2;
int count=1;

void *producer(void *arg)
{
        int i;
        for (i = 0; i < producer_amount; ++ i)
        {
          Message m=QMESSAGE__INIT;
	  m.req=count++;
	  
	  queue_put(arg, m);
	  LOG_DEBUG("%d\n", m.req);
	  sleep(1);
        }
	return NULL;
}

void *consumer(void *arg)
{
        int i;
        for (i = 0; i < producer_amount * producers / consumers; ++ i)
        {
	  Message m=QMESSAGE__INIT;
	  queue_get(arg,&m);
	  LOG_DEBUG("%d\n", m.req);
	  sleep(1);
        }
	return NULL;
}

int main()
{
	Message EngineList[buffer_size];
	Queue EngineQueue= INIT_QUEUE(EngineList);
	
        int i;
	pthread_t ThreadFactory;
        pthread_t producers[producers];
        pthread_t consumers[consumers];

	ThreadFactory=startThreadsFactory();
	block_signals();
        for (i = 0; i < sizeof(producers) / sizeof(producers[0]); ++ i)
        {
	  producers[i]=startThread(producer,&EngineQueue);
	  LOG_DEBUG("Thread producer:%lx created\n",producers[i]);
        }

        for (i = 0; i < sizeof(consumers) / sizeof(consumers[0]); ++ i)
        {
	  consumers[i]=startThread(consumer,&EngineQueue);
	  LOG_DEBUG("Thread consumer:%lx created\n",consumers[i]);
        }
        for (i = 0; i < sizeof(producers) / sizeof(producers[0]); ++ i)
        {
	  
	  stopThread(producers[i],0);
	  LOG_DEBUG("Thread producer:%lx destroyed\n",producers[i]);	  
        }
        for (i = 0; i < sizeof(consumers) / sizeof(consumers[0]); ++ i)
        {
	  stopThread(consumers[i],0);
	  LOG_DEBUG("Thread consumer:%lx destroyed\n",consumers[i]);
        }
	stopThreadsFactory(ThreadFactory);
	return 0;
}
#endif

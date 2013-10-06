
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "logs.h"
#include "ThreadFactory.h"


#define DeclareQueueType(MessageType)			\
  typedef struct _Queue_##MessageType		\
  {						\
  MessageType *List;				\
  const int capacity;				\
  int size;					\
  int start;					\
  int end;					\
  pthread_mutex_t mutex;			\
  pthread_cond_t full;				\
  pthread_cond_t empty;				\
  } Queue_##MessageType;			

#define INIT_QUEUE(bList) {				\
      .List = bList,					\
      .capacity = sizeof(bList) / sizeof(bList[0]),	\
      .size = 0,					\
      .start = 0,					\
      .end = 0,						\
      .mutex  = PTHREAD_MUTEX_INITIALIZER,			\
      .full  = PTHREAD_COND_INITIALIZER,			\
      .empty = PTHREAD_COND_INITIALIZER }


DeclareQueueType(TFReqRplMessage)

static int queue_put(Queue_TFReqRplMessage *queue, TFReqRplMessage msg); 
static int queue_get(Queue_TFReqRplMessage *queue,TFReqRplMessage* pmsg);
static int queue_size(Queue_TFReqRplMessage *queue);    
static int queue_close(Queue_TFReqRplMessage *queue);

static TFReqRplMessage ReqBuffer[TF_QUEUE_SIZE];
static Queue_TFReqRplMessage ReqQueue=INIT_QUEUE(ReqBuffer);
static TFReqRplMessage RplBuffer[TF_QUEUE_SIZE];
static Queue_TFReqRplMessage RplQueue=INIT_QUEUE(RplBuffer);
static  void *qlist[]={&ReqQueue,&RplQueue};

int queue_put(Queue_TFReqRplMessage *queue, TFReqRplMessage msg)
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

int queue_get(Queue_TFReqRplMessage *queue,TFReqRplMessage* pmsg)
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

int queue_size(Queue_TFReqRplMessage *queue)
{
        pthread_mutex_lock(&(queue->mutex));
        int size = queue->size;
        pthread_mutex_unlock(&(queue->mutex));
        return size;
}

int queue_close(Queue_TFReqRplMessage *queue)
{
  
  pthread_cond_destroy(&(queue->empty));
  pthread_cond_destroy(&(queue->full));
  pthread_mutex_destroy(&(queue->mutex));
  /*could add code to notify waiters*/
  return 0;
}

pthread_t startThreadsFactory()
{
  pthread_t thread;
  pthread_attr_t attr;
  int err;  LOG_DEBUG(" ");
  pthread_attr_init(&attr);
  err=pthread_create(&thread,&attr,Thread_Factory,&qlist);
  pthread_attr_destroy(&attr);
  if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
  return thread;
}

int stopThreadsFactory(pthread_t tid)
{
  int err;
  TFReqRplMessage m;
  m.msgtype=Exit;
  queue_put(qlist[0],m);
  err=pthread_join(m.params.tid,NULL);
  if(err)LOG_DEBUG("%d %s",errno,strerror(errno));
  return err;
}



pthread_t startThread(void * (func)(void *) ,void* args)
{
  TFReqRplMessage m,rm;
  m.msgtype=Create;
  m.params.thread[0] = func;
  m.params.thread[1] = args;
  queue_put(qlist[0],m);
  queue_get(qlist[1],&rm);
  return (pthread_t)rm.params.tid;
}
int stopThread(pthread_t tid,int force)
{
  TFReqRplMessage m,rm;
  m.msgtype=Destroy;
  m.force = force;
  m.params.tid = tid;
  queue_put(qlist[0],m);
  queue_get(qlist[1],&rm);
  LOG_DEBUG("Thread:%lx destroyed\n",rm.params.tid);
  return 0;
}


void* Thread_Factory(void *arg)
{
  Queue_TFReqRplMessage *work_q = ((Queue_TFReqRplMessage**)arg)[0];
  Queue_TFReqRplMessage *ret_q  = ((Queue_TFReqRplMessage**)arg)[1];
  TFReqRplMessage msg;
  int err;
  LOG_DEBUG("Thread Factory Started\n");
  pthread_t signal_worker;
  {
      pthread_t thread;
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);  
      err=pthread_create(&signal_worker,&attr,(void * (*)(void *))sighandler_thread,NULL);
      if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
      else   LOG_DEBUG("Signal handler for factory Started\n");
      pthread_attr_destroy(&attr);
  }
  while(queue_get(work_q,&msg))
  {
    LOG_DEBUG("Thread Factory message:%d [%lx %lx]\n",msg.msgtype,(long)msg.params.thread[0],(long)msg.params.thread[1]);
    if( msg.msgtype == Create)
    {
      pthread_t thread;
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);  
      block_signals();
      err=pthread_create(&thread,&attr,(void * (*)(void *))msg.params.thread[0],(void*)msg.params.thread[1]);
      if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
      pthread_attr_destroy(&attr);
      msg.params.tid=thread;
      queue_put(ret_q,msg);
    }
    if ( msg.msgtype == Destroy)
    {
      int err;
	  //signal exit and wait
      if(msg.force)
	{
	  LOG_DEBUG("Thread Factory kill:%lx\n",msg.params.tid);
	  err=pthread_kill(msg.params.tid,SIGKILL);
	  if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
	}
	  
      LOG_DEBUG("Thread Factory join request:%lx, ",msg.params.tid);
      err=pthread_join(msg.params.tid,NULL);
      if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
      queue_put(ret_q,msg);
      LOG_DEBUG("Done [%lx].\n",msg.params.tid);
      
    }
    if ( msg.msgtype == Exit )
    {
      LOG_DEBUG("Thread Factory exit request:%lx, ",pthread_self());
      err=pthread_kill(signal_worker,SIGTERM);
      if(err) LOG_DEBUG("%d %s",err,strerror(err));
      else  LOG_DEBUG("Signal handler for factory killed\n");
      /*err=pthread_join(signal_worker,NULL);
	if(err) LOG_DEBUG("%d %s",err,strerror(err));
	else    LOG_DEBUG("Signal handler for factory joined\n");*/
      break;
    }
    memset(&msg,0,sizeof(msg));
  }
  LOG_DEBUG("Thread Factory Exited[%lx]\n",pthread_self());
  return NULL;
}

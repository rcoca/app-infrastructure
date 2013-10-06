#include "threads.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "logs.h"
#include "queue.h"

#define  buffer_size 10
static Message MsgList[buffer_size];
static Message RepList[buffer_size];
static Queue WorkQ    = INIT_QUEUE(MsgList);
static Queue ResultQ  = INIT_QUEUE(RepList);
static  Queue *qlist[]={NULL,NULL};

pthread_t startThreadsFactory()
{
  pthread_t thread;
  pthread_attr_t attr;
  int err;
  qlist[0]=&WorkQ;
  qlist[1]=&ResultQ;

  LOG_DEBUG(" ");
  pthread_attr_init(&attr);
  err=pthread_create(&thread,&attr,thread_factory,&qlist);
  pthread_attr_destroy(&attr);
  if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
  return thread;
}
int stopThreadsFactory(pthread_t tid)
{
  int err;
  Message m;
  m.req=QMESSAGE__REQUEST_TYPE__EXIT;
  queue_put(qlist[0],m);
  err=pthread_join(tid,NULL);
  if(err)LOG_DEBUG("%d %s",errno,strerror(errno));
  return err;
}

pthread_t startThread(void * (func)(void *) ,void* args)
{
  Message m;
  m.req=QMESSAGE__REQUEST_TYPE__CREATE;
  m.thread_main = (uint64_t)func;
  m.thread_args = (uint64_t)args;
  queue_put(qlist[0],m);
  queue_get(qlist[1],&m);
  return (pthread_t)m.thread_args;
}
int stopThread(pthread_t tid,int force)
{
  Message m;
  m.req=QMESSAGE__REQUEST_TYPE__DESTROY;
  m.thread_main = force;
  m.thread_args = tid;
  queue_put(qlist[0],m);
  queue_get(qlist[1],&m);
  LOG_DEBUG("Thread:%lx destroyed\n",m.thread_args);	  
  return 0;
}

void* thread_factory(void *arg)
{
  Queue *work_q = ((Queue**)arg)[0];
  Queue *ret_q  = ((Queue**)arg)[1];
  Message msg;
  LOG_DEBUG("Thread Factory Started\n");
  while(queue_get(work_q,&msg))
  {
    LOG_DEBUG("Thread Factory message:%d [%lx %lx]\n",msg.req,msg.thread_main,msg.thread_args);
    if( msg.req==QMESSAGE__REQUEST_TYPE__CREATE)
    {
      int err;
      pthread_t thread;
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);  
      block_signals();
      err=pthread_create(&thread,&attr,(void * (*)(void *))msg.thread_main,(void*)msg.thread_args);
      if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
      pthread_attr_destroy(&attr);
      msg.thread_main=0;
      msg.thread_args=(uint64_t)thread;
      queue_put(ret_q,msg);
    }
    if ( msg.req== QMESSAGE__REQUEST_TYPE__DESTROY)
    {
      int err;
	  //signal exit and wait
      if(msg.thread_main)
	{
	  LOG_DEBUG("Thread Factory kill:%lx\n",msg.thread_args);
	  err=pthread_kill(msg.thread_args,SIGKILL);
	  if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
	}
	  
      LOG_DEBUG("Thread Factory join request:%lx, ",msg.thread_args);
      err=pthread_join(msg.thread_args,NULL);
      if(err) LOG_DEBUG("%d %s",errno,strerror(errno));
      msg.thread_main=0;
      queue_put(ret_q,msg);
      LOG_DEBUG("Done [%lx].\n",msg.thread_args);
      
    }
    if ( msg.req== QMESSAGE__REQUEST_TYPE__EXIT)
    {
      LOG_DEBUG("Thread Factory exit request:%lx, ",pthread_self());
      break;
    }
    memset(&msg,0,sizeof(msg));
  }
  LOG_DEBUG("Thread Factory Exited[%lx]\n",pthread_self());
  return NULL;
}

#include "tsignal.h"
#include "logs.h"
#include <execinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static int bsignals[]={SIGINT,SIGUSR1,SIGUSR2,SIGPIPE,SIGKILL,};

int block_signals()
{
  sigset_t blocked_set;
  int i,err;
  sigemptyset(&blocked_set);
  for(i=0;i<sizeof(bsignals)/sizeof(int);i++)
    sigaddset(&blocked_set,bsignals[i]);
  err=pthread_sigmask(SIG_BLOCK, &blocked_set, NULL);
  return err;
}

int unblock_signals()
{
  sigset_t set;
  int err;
  sigemptyset(&set);
  err=pthread_sigmask(SIG_BLOCK, &set, NULL);
  return err;
}

void* sighandler_thread(void* arg)
{
  sigset_t all;
  sigfillset(&all);
  int sig,err;
  while(1)
  {
    err=sigwait(&all,&sig);
    LOG_DEBUG("Sighandler:%d",sig);
    if(err)break;
  }
  return NULL;
}

#define WD_PIPE "/tmp/wdpipe"
int watchdog_fd=-1;
#define tracelen 16
#define linelen 1024

int setup_watchdog()
{
  void * addrbuffer[tracelen];
  pid_t cpid=-1;
  backtrace(addrbuffer,tracelen);
  unlink(WD_PIPE);
  if(mkfifo(WD_PIPE,S_IRWXU)==-1)
  {
    printf("Error creating pipe\n");
  }
  cpid=fork();
  if (cpid == 0) { 
    char buff[linelen];
    watchdog_fd=-1;
    while(watchdog_fd==-1)
    {
      watchdog_fd=open(WD_PIPE,O_RDONLY|O_CLOEXEC);
      if(watchdog_fd==-1)
	printf("error read open:%d %s\n",getpid(),strerror(errno));
      else break;
      usleep(10*1000);
    }
    memset(buff,0,linelen);
    while(read(watchdog_fd,buff,linelen)>0)
    {
      //LOG_DEBUG("%s",buff);
      printf("%s",buff);
      fflush(stdout);
      memset(buff,0,linelen);
    }

  }else{
    watchdog_fd=-1;
    while(watchdog_fd==-1)
    {
      watchdog_fd=open(WD_PIPE,O_WRONLY|O_CLOEXEC);
      if(watchdog_fd==-1)printf("error write open:%d %s\n",getpid(),strerror(errno));
	else break;
      usleep(10*1000);
    }      
 }
  return watchdog_fd;
}

void stack_trace(int sig)
{
  void * addrbuffer[tracelen];
  memset(addrbuffer,0,tracelen*sizeof(void*));
  char *text[2]={"Signal received:\t\n\0","\nStack trace:\n\0"};
  const char* signame=sys_siglist[sig];//strsignal(sig);
  write(watchdog_fd,text[0],strlen(text[0])+1);
  write(watchdog_fd,text[1],strlen(text[1])+1);
  write(watchdog_fd,signame,strlen(signame));
  write(watchdog_fd,"\0",1);
  write(watchdog_fd,"\n\0",2);
  
  const int calls = backtrace(addrbuffer,sizeof(addrbuffer) / sizeof(void *));
  backtrace_symbols_fd(addrbuffer,calls,watchdog_fd);
  printf("StackTrace ended\n");
}

void hsighandler(int sig)
{
  stack_trace(sig);
  exit(1);
}

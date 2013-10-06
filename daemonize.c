#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include "logs.h"


int daemonize()
{
  pid_t pid;
  int fd;
  if(getppid()==1) return 0;
  pid=fork();
  if (pid<0)
  {
    LOG_DEBUG("Error fork:%d %s",errno,strerror(errno));
  }
  if (pid>0) exit(0);
  setsid();
  setpgid(0,0);
  for (fd=getdtablesize();fd>=0;--fd) close(fd);
  fd=open("/dev/null",O_RDWR);
  dup(fd);
  dup(fd);
  umask(027); 
  chdir("/");
  return 0;
}

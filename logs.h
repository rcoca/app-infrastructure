#ifndef LOGS_H
#define LOGS_H
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <string.h>
#define LOG_DEBUG(format,...)\
  fprintf(stderr,"[%s, %s:%d][%d:%ld]",__FUNCTION__,__FILE__,__LINE__,getpid(),syscall(SYS_gettid)), \
    fprintf(stderr,format,##__VA_ARGS__)

#define LOG_SYSERR(format,...)\
  fprintf(stderr,"[%s, %s:%d][%d:%ld][errno:%d %s]",__FUNCTION__,__FILE__,__LINE__,\
	  getpid(),syscall(SYS_gettid),errno,strerror(errno)),		\
    fprintf(stderr,format,##__VA_ARGS__)
  
#endif

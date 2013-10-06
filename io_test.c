#define _GNU_SOURCE
#ifdef IO_TEST

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define min(x,y) (x>y?y:x)
#include <unistd.h>
#include <time.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "logs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io_test.h"
#define FILE_OUT "file.%d.out"

//#define ALIGN_MEM_BLOCK 4096

double Clock( double t0)
{
  struct timeval tp={0,0};
  if(gettimeofday(&tp, (struct timezone *) NULL)==-1) LOG_DEBUG("time error %d %s",errno,strerror(errno));
  return ((double) (tp.tv_sec -t0)) +  (((double) tp.tv_usec) / 1000000.0);
  
}

int DropCaches()
{
    int fd=-1;
    fd=open("/proc/sym/vm/drop_caches",O_WRONLY);
    if(fd==-1)
    {
	printf("%s %d %s\n",__FUNCTION__,errno,strerror(errno));
    }
    write(fd,"3\n",2);
    close(fd);
    return 0;
}
struct _options{
  int    flags;
  size_t size;
  char * FILE_IN;
  int direct;
  int sync;
  int align;
  int flushfs;
  int multifile;
  int repeat;
  blksize_t ALIGN_MEM_BLOCK;
  
}options={
  .flags    =  O_WRONLY|O_CREAT|O_TRUNC,
  .size      = 0,
  .FILE_IN   = "/dev/zero",
  .direct    = 0,
  .sync      = 0,
  .align     = 0,
  .flushfs   = 1,
  .multifile = 1,
  .repeat    = 1,
  .ALIGN_MEM_BLOCK=4096
};

int parse_options(int argc,char **argv)
{
  int i;
  if (argc<2)return -1;
  if(options.size==0 && isdigit(argv[1][0]))options.size=atol(argv[1]);
  {
    int fd,err;
    struct stat bbuf;
    fd=open(FILE_OUT,options.flags,S_IRUSR|S_IWUSR);  
    err=fstat(fd,&bbuf);
    close(fd);
    unlink(FILE_OUT);
    if(!err)options.ALIGN_MEM_BLOCK=bbuf.st_blksize;
  }

  for(i=2;i<argc;i++)
  { 
      if(!strcmp(argv[i],"direct"))   options.flags|=O_DIRECT;
      if(!strcmp(argv[i],"sync"))     options.flags|=O_SYNC;
      if(!strcmp(argv[i],"large"))    options.flags|=O_LARGEFILE;
      if(!strcmp(argv[i],"align"))    options.align=1;
      if(!strcmp(argv[i],"random"))   options.FILE_IN="/dev/urandom";
      if(!strcmp(argv[i],"flushfs"))  options.flushfs=1;
      if(!strcmp(argv[i],"multifile"))
      {
	if(argv[i+1]&&isdigit(argv[i+1][0]))
	  {
	    i++;
	    options.multifile=atol(argv[i]);
	  }
      }
       if(!strcmp(argv[i],"repeat"))
      {
	if(argv[i+1]&&isdigit(argv[i+1][0]))
	  {
	    i++;
	    options.repeat=atol(argv[i]);
	  }
      }
       //if(!strcmp(argv[i],"sim"))sim=1;	 
  }

  return 0;
}

int main(int argc, char *argv[])
{

  ssize_t sread=0;
  uint8_t *buff=NULL,*tbuff=NULL;
  int err=0;
  int fd,rep;
  double tstart,tread,twrite,tunlink,tiowaits;
  cpu_stats timing;

  if(parse_options(argc,argv)==-1)return -1;

  tstart=(double)Clock(0);
  iowaits();
  if(options.align)err=posix_memalign((void**)&buff,options.ALIGN_MEM_BLOCK,options.size);
  if(err==-1)LOG_DEBUG("Error aligned alloc %d %s",err,strerror(err));
  if(!buff)buff=malloc(options.size);
  fd=open(options.FILE_IN,O_RDONLY);
  sread=read(fd,buff,options.size);
  if(sread<0)LOG_DEBUG("%d %s",errno,strerror(errno));
  close(fd);
  tread=(double)Clock(tstart);
  iowaits();

  int written=0;
  int csize=options.size/options.multifile;
  if(options.multifile>1)
  {
    if(options.align)err=posix_memalign((void**)&tbuff,options.ALIGN_MEM_BLOCK,csize);
    else tbuff=malloc(csize);
  }
  else tbuff=buff;
  if(err)LOG_DEBUG("%d %s",errno,strerror(errno));
  for(rep=0;rep<options.repeat;rep++)
  {
    char fname[256];
    sprintf(fname,FILE_OUT,rep);
    fd=open(fname,options.flags,S_IRUSR|S_IWUSR);
    if(fd==-1)
    {
      printf("%d %s\n",errno,strerror(errno));
      exit(1);
    }
    for(written=0;written<options.size;written+=csize)
    {
      if(options.multifile>1) memcpy(tbuff,&buff[written],min(options.size-written,csize));
      else tbuff=&buff[written];
      err=write(fd,tbuff,min(options.size-written,csize));
      if(err==-1)
	{
	  printf("%d %s %d 0x%x %d\n",errno,strerror(errno),fd,
		 (unsigned int)tbuff,(int)min(options.size-written,csize));
	}
      // printf("wrote %d,%d\n",err,min(size-written,csize));
    }
    fsync(fd);
    fdatasync(fd);
    if(options.flushfs){
      syncfs(fd);
    }
    close(fd);
    fd=-1;
    if(options.flushfs) 
    {
	sync();
	if(geteuid()==0)
	    DropCaches();//need root to /proc/sys...
    }
  }
  twrite=(double)Clock(tstart)-tread;
  timing=iowaits();
  tiowaits= timing.iowait;
  if(options.multifile>1)free(tbuff);
  free(buff);
  for(rep=0;rep<options.repeat;rep++)
  {
    char fname[256];
    sprintf(fname,FILE_OUT,rep);
    unlink(fname);
  }
  sync();
  tunlink=(double)Clock(tstart)-twrite;
  printf("nfiles:%d\tsize[M]:%4.5f\tread:%4.5f\twrite:%4.5f\tunlink:%4.5f\tiowait:%4.5f\trusage:%4.5f\tmrusage:%4.5f\twspeed[MB/s]:%4.5f\n",
	 options.multifile,
	 ((double)options.size)/(1024.0*1024.0),tread,(float)twrite/(float)options.repeat,tunlink,
	 (float)tiowaits/(float)options.repeat,
	 (float)timing.cpu_ru/(float)options.repeat,
	 (float)timing.machine_cpu_ru/(float)options.repeat,
	 (float)options.size*(float)options.repeat/(1024.*1024.)/(float)twrite);
  return 0;
}
#endif

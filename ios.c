#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

typedef struct _cpu_info {
	unsigned long long user;
	unsigned long long system;
	unsigned long long machine_user;
	unsigned long long machine_system; 
	unsigned long long idle;
	unsigned long long iowait;
} cpu_info;

#define BUF_SIZE 256
#include <sys/param.h>
#include "io_test.h"


cpu_stats iowaits()
{
  static cpu_info initial_info={0,0,0,0,0,0};
  static long MY_HZ = 0;
  static int cpus=0;
  struct rusage use;
  memset(&use,0,sizeof(use));
  if(MY_HZ==0)MY_HZ=sysconf(_SC_CLK_TCK);
  if(cpus==0)cpus=sysconf (_SC_NPROCESSORS_ONLN);
  char buffer[BUF_SIZE];
  int items;
  unsigned long long nice, irq, softirq;
  cpu_info new_cpu;
  int fd;
  cpu_stats ret;
  fd=open("/proc/stat",O_RDONLY);
  read(fd,buffer,BUF_SIZE);
  close(fd);
  items = sscanf(buffer,
		 "cpu %llu %llu %llu %llu %llu %llu %llu",
		 &new_cpu.machine_user, &nice,
		 &new_cpu.machine_system,
		 &new_cpu.idle,
		 &new_cpu.iowait,
		 &irq, &softirq);


  getrusage(RUSAGE_SELF,&use);
  new_cpu.user   = use.ru_utime.tv_sec*1000.0+use.ru_utime.tv_usec/1000.0;
  new_cpu.system = use.ru_stime.tv_sec*1000.0+use.ru_stime.tv_usec/1000.0; 

  ret.machine_user_ru=(double)(new_cpu.machine_user-initial_info.machine_user)/(double)(HZ);
  ret.machine_system_ru=(double)(new_cpu.machine_system-initial_info.machine_system)/(double)(HZ);
  
  ret.user=(double)(new_cpu.user-initial_info.user)/1000.0;
  ret.system=(double)(new_cpu.system-initial_info.system)/1000.0;
  
  ret.cpu_ru=ret.user+ret.system;
  ret.idle=(double)(new_cpu.idle-initial_info.idle)/(double)(HZ);
  ret.iowait=(double)(new_cpu.iowait-initial_info.iowait)/(double)(HZ);
  ret.machine_cpu_ru=ret.machine_user_ru+ret.machine_system_ru;
  initial_info=new_cpu;
  return ret;
}


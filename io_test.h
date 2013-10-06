#ifndef _IO_TESTS_H
#define _IO_TESTS_H

typedef struct _cpu_stats{
  double user;
  double system;
  double machine_cpu_ru;
  double machine_user_ru;
  double machine_system_ru;
  double cpu_ru;
  double idle;
  double iowait;
} cpu_stats;
cpu_stats iowaits();
#endif

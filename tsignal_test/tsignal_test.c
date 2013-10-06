#include "tsignal.h"

void long_function_name(){
  int *p=NULL;
  *p=3;
}
void b(){long_function_name();}
void a(){b();}

int main(int argc, char *argv[])
{

  setup_watchdog();
  signal(SIGSEGV, hsighandler);
  a();
  sleep(3);
  printf("main exited\n");
  return 0;
}

#include <unistd.h>
#include <stdio.h>

#include "daemonize.h"
#include "logs.h"

#ifdef DAEMON_TEST
int main(int argc, char * argv[])
{
  LOG_DEBUG("");
  daemonize();
  while(1)
  {
    
    sleep(1);
  }
  return 0;
}
#endif

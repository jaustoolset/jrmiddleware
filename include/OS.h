//  Abstract OS calls
#ifndef  __ABSSTRACT_OS_H
#define __ABSSTRACT_OS_H

#include <string>
#ifdef WINDOWS
    #include "Winsock.h"
#else
    #include <sys/socket.h>
    #include <cygwin/in.h>
    #include <unistd.h>
    #include <sys/un.h>
    #include <arpa/inet.h>
#endif

void JrSleep(unsigned long milliseconds);
void JrSpawnProcess(std::string path);
unsigned long JrRandomValue();

#endif



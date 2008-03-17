/*! 
 ***********************************************************************
 * @file      OS.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __ABSSTRACT_OS_H
#define __ABSSTRACT_OS_H

#include <string>
#ifdef WINDOWS
    #include "Winsock.h"
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <sys/un.h>
    #include <arpa/inet.h>
#endif

namespace DeVivo {
namespace Junior {

void JrSleep(unsigned long milliseconds);
void JrSpawnProcess(std::string path, std::string arg);
unsigned long JrGetTimestamp();

}} // namespace DeVivo::Junior
#endif



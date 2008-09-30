/*! 
 ***********************************************************************
 * @file      OS.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 *  Copyright (C) 2008. DeVivo AST, Inc
 *
 *	This program is free software: you can redistribute it and/or modify  it 
 *  under the terms of the Jr Middleware Open Source License which can be 
 *  found at http://www.jrmiddleware.com/osl.html.  This program is 
 *  distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 *  PARTICULAR PURPOSE.  See the Jr Middleware Open Source License for more 
 *  details.
 *	
 *  For more information, please contact DeVivo AST at info@devivoast.com
 *  or by mail at 2225 Drake Ave, Suite 2, Huntsville, AL  35805.
 *
 *  The Jr Middleware Open Source License does not permit incorporating your 
 *  program into proprietary programs. If this is what you want to do, 
 *  use the Jr Middleware Commercial License. More information can be 
 *  found at: http://www.jrmiddleware.com/licensing.html.
 ************************************************************************
 */
#ifndef __ABSSTRACT_OS_H
#define __ABSSTRACT_OS_H

#include <string>
#include <list>
#ifdef WINDOWS
    #include "Winsock.h"
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <sys/un.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <strings.h>
    #include <errno.h>
    #include <termios.h>
    #include <pthread.h>
#ifndef __CYGWIN__
    #include <ifaddrs.h>
#endif
#endif

namespace DeVivo {
namespace Junior {

void JrSleep(unsigned long milliseconds);
void JrSpawnProcess(std::string path, std::string arg);
void JrSpawnThread(void*(*func_ptr)(void*), void* func_arg);
unsigned long JrGetTimestamp();
std::list<unsigned long> JrGetIPAddresses();
bool JrStrCaseCompare(std::string str1, std::string str2);

}} // namespace DeVivo::Junior
#endif



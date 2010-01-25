/*! 
 ***********************************************************************
 * @file      OS.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 *  Copyright (C) 2008. DeVivo AST, Inc
 *
 *  This file is part of Jr Middleware.
 *
 *  Jr Middleware is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Jr Middleware is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Jr Middleware.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    #include <stdlib.h>
    #include <cstdlib>
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
std::list<unsigned int> JrGetIPAddresses();
bool JrStrCaseCompare(std::string str1, std::string str2);

}} // namespace DeVivo::Junior
#endif



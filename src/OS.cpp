/*! 
 ***********************************************************************
 * @file      OS.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "OS.h"
#include "JrLogger.h"

using namespace DeVivo::Junior;


void DeVivo::Junior::JrSleep(unsigned long milliseconds)
{
#ifdef WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}


void DeVivo::Junior::JrSpawnProcess(std::string path, std::string arg)
{
#ifdef WINDOWS
    // Windows gives us the createProcess function.  We just need to
    // make sure it doesn't already exist.
    char cmd[500];
    //memset(cmd, 0, 50);
    sprintf(cmd, "tasklist | findstr %s\0", path.c_str());
    int ret = system(cmd);

    if (ret != 0)
    {
        JrInfo << "Trying to CreateProcess " << path << std::endl;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(STARTUPINFO)); si.cb = sizeof(si);
        memset(&pi, 0, sizeof(PROCESS_INFORMATION));

        // turn off the window
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        // Now create the process that points to the RTE
        sprintf(cmd, "%s %s\0", path.c_str(), arg.c_str());
        BOOL result = CreateProcess(  NULL, LPSTR(cmd), NULL, NULL, FALSE, 
            HIGH_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP, NULL,  NULL,  &si, &pi);
        if(result == 0)  
            JrError << "Could not create process " << path << std::endl;
    }

#else

    // For Unix, we need to fork and manually start the new process
    // First check for existence of the Junior RTE
    char cmd[50];
    sprintf(cmd, "ps -e | grep '%s'\0", path.c_str());
    int ret = system(cmd);
    if (ret != 0)
    {
        // No JuniorRTE found.  Start a new process...
        JrInfo << "Starting Junior Run-Time Engine...\n";
        if (fork()==0)
           execl(path.c_str(), path.c_str(), arg.c_str(), NULL);
    }

#endif
}

// Return the current time (in milliseconds)
unsigned long DeVivo::Junior::JrGetTimestamp()
{
#ifdef WINDOWS
    return (unsigned long)(GetTickCount());
#else
    struct timeval tv; struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec*1000 + (unsigned long)(tv.tv_usec/1000));
#endif
}

// Return a list of IP addresses with all the NIC associate with this host
std::list<unsigned long> DeVivo::Junior::JrGetIPAddresses()
{
    std::list<unsigned long> addresses;

#if defined(WINDOWS) || defined(__CYGWIN__)

    // Windows doesn't support ioctl calls, and the gethostbyname is a 
    // better method anyway....
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == 0)
    {
        struct hostent *phe = gethostbyname(ac);
        if (phe != 0) 
            for (int i = 0; phe->h_addr_list[i] != 0; ++i) 
                addresses.push_back(((in_addr*)phe->h_addr_list[i])->s_addr);
    }

#else

    // On Linux, we can use getifaddrs supported by BSD libraries.
    struct ifaddrs *ifap, *next;
    if (getifaddrs(&ifap) != 0) return addresses;
    if (ifap == NULL) return addresses;

    // Loop through each interface, adding the address to the list
    next = ifap;
    do
    {
        if ( (next->ifa_addr->sa_family == AF_INET)  &&
             ((((sockaddr_in*) next->ifa_addr)->sin_addr.s_addr) !=
                        inet_addr("127.0.0.1")))
            addresses.push_back(((sockaddr_in*) next->ifa_addr)->sin_addr.s_addr);
        next = next->ifa_next;
    } while (next != NULL);

    // We need to free the memory allocated by getifaddrs
    freeifaddrs(ifap);

#endif

   return addresses;
}

bool DeVivo::Junior::JrStrCaseCompare(std::string str1, std::string str2)
{
#ifdef WINDOWS
    return 
        (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
          str1.c_str(), str1.size(), str2.c_str(), str2.size())==2);
#else
    if (str1.size() != str2.size()) return false;
    return ((bool)(!strncasecmp(str1.c_str(), str2.c_str(), str1.size())));
#endif
}

void DeVivo::Junior::JrSpawnThread(void*(*func_ptr)(void*), void* func_arg)
{
#ifdef WINDOWS
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) func_ptr, func_arg, 0, NULL);
#else
    pthread_t thread_info;
    pthread_create(&thread_info, NULL, func_ptr, func_arg);
#endif
}







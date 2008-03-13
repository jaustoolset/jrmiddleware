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
        printf("Trying to CreateProcess(%s)\n", path.c_str());
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(STARTUPINFO)); si.cb = sizeof(si);
        memset(&pi, 0, sizeof(PROCESS_INFORMATION));
        sprintf(cmd, "%s %s\0", path.c_str(), arg.c_str());
        BOOL result = CreateProcess(  NULL, LPSTR(cmd), NULL, NULL, FALSE, 
            NORMAL_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP, NULL,  NULL,  &si, &pi);
        if(result == 0)  printf("Could not create process (%s)\n", path.c_str());
    }

#else

    // For Unix, we need to fork and manually start the new process
    // First check for existence of the Junior RTE
    char cmd[50];
    sprintf(cmd, "ps -a | grep '%s'\0", path.c_str());
    int ret = system(cmd);
    if (ret != 0)
    {
        // No JuniorRTE found.  Start a new process...
        printf("Starting Junior Run-Time Engine...\n");
        if (fork()==0)
           execl(path.c_str(), path.c_str(), arg.c_str(), NULL);
    }

#endif
}

// Return the current time (in seconds)
unsigned long DeVivo::Junior::JrGetTimestamp()
{
#ifdef WINDOWS
    return (unsigned long)(GetTickCount()/1000);
#else
    return (unsigned long)(time(NULL));
#endif
}



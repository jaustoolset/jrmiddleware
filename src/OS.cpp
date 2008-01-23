//  Abstract OS calls
#include "OS.h"

void JrSleep(unsigned long milliseconds)
{
#ifdef WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void JrSpawnProcess(std::string path)
{
#ifdef WINDOWS
    // Windows gives us the createProcess function.  We just need to
    // make sure it doesn't already exist.
    char cmd[50];
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
        BOOL result = CreateProcess(  NULL, LPSTR(path.c_str()), NULL, NULL, FALSE, 
            NORMAL_PRIORITY_CLASS, NULL,  NULL,  &si, &pi);
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
           execl(path.c_str(),0);
    }

#endif
}

unsigned long JrRandomValue()
{
#ifdef WINDOWS
    return GetTickCount();
#else
    return (unsigned long)(time(NULL));
#endif
}




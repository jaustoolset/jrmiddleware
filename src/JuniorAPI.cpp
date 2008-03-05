// This is the principle file for the realization of the Junior API.
// Note that the interface itself is completely function, but the "handle"
// represents an object that manages the connection to the Junior RTE.
#include "JuniorAPI.h"
#include "JuniorMgr.h"
#include "OS.h"
#include <stdio.h>

// Functional interface
JrErrorCode sendto(int handle,
           unsigned long destination, 
           unsigned int bufsize, 
           const char* buffer,
           int priority,
           int flags)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->sendto(destination, bufsize, buffer, priority, flags));
}

JrErrorCode sendto(int handle,
           unsigned long destination, 
           unsigned short msg_id,
           unsigned int bufsize, 
           const char* buffer,
           int priority,
           int flags)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->sendto(destination, bufsize, buffer, priority, flags, msg_id));
}

JrErrorCode broadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->sendto(0xFFFFFFFF, bufsize, buffer, priority, 0));
}

JrErrorCode broadcast(int handle,
              unsigned short msg_id,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->sendto(0xFFFFFFFF, bufsize, buffer, priority, 0, msg_id));
}

JrErrorCode recvfrom(int handle,
             unsigned long* sender,
             unsigned int* bufsize,
             char* buffer,
             int* priority)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->recvfrom(sender, bufsize, buffer, priority));
}

JrErrorCode recvfrom(int handle,
             unsigned long* sender,
             unsigned short* msg_id,
             unsigned int* bufsize,
             char* buffer,
             int* priority)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->recvfrom(sender, bufsize, buffer, priority, msg_id));
}

JrErrorCode connect(unsigned long id, char* config_file, int* handle)
{
    if (handle == NULL)
        return InitFailed;

    // Spawn the RTE.  Note that hte spawn process will 
    // ensure that we don't create a duplicate.
    JrSpawnProcess("JuniorRTE", config_file);
    JrSleep(2000);

    // Create and initialize Junior Manager, to manage this 
    // connection to the RTE.  
    JuniorMgr* mgr = new JuniorMgr();
    JrErrorCode ret = mgr->connect(id, config_file);
    if (ret != Ok)
    {
        delete mgr;
        *handle = -1;
    }
    else
        *handle = (int)mgr;
    return ret;
}

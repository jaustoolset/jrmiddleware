// This is the principle file for the realization of the Junior API.
// Note that the interface itself is completely function, but the "handle"
// represents an object that manages the connection to the Junior RTE.
#include "JuniorAPI.h"
#include "JuniorMgr.h"
#include "OS.h"
#include <stdio.h>

// Functional interface
int sendto(int handle,
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

int broadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->broadcast(bufsize, buffer, priority));
}

int recvfrom(int handle,
             unsigned long* sender,
             unsigned int bufsize,
             char* buffer,
             int* priority)
{
    // Extract the connection data from the handle
    JuniorMgr* mgr = (JuniorMgr*) handle;

    // Call the recv function of the manager.
    return (mgr->recvfrom(sender, bufsize, buffer, priority));
}

int connect(unsigned long id)
{
    // Spawn the RTE.  Note that hte spawn process will 
    // ensure that we don't create a duplicate.
    JrSpawnProcess("JuniorRTE");
    JrSleep(2000);

    // Create and initialize Junior Manager, to manage this 
    // connection to the RTE.  
    JuniorMgr* mgr = new JuniorMgr();
    if (mgr->connect(id) != Transport::Ok)
    {
        printf("Failed to establish connection with the Run-Time Engine...\n");
        delete mgr;
        return 0;
    }
    
    // A pointer to this object is then returned as a handle
    return ((int) mgr);
}

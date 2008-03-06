// This is the principle file for the realization of the Junior API.
// In reality, this "implementation" simply serves as public access
// to the JuniorMgr class, which does the real work.
#include "JuniorAPI.h"
#include "JuniorAPI.hpp"
#include "JuniorMgr.h"

JrErrorCode sendto(int handle,
           unsigned long destination, 
           unsigned short msg_id,
           unsigned int bufsize, 
           const char* buffer,
           int priority,
           int flags)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->sendto(destination, bufsize, buffer, priority, flags, msg_id));
}

// Functional interface
JrErrorCode sendto(int handle,
           unsigned long destination, 
           unsigned int bufsize, 
           const char* buffer,
           int priority,
           int flags)
{
    return sendto(handle, destination, 0, bufsize, buffer, priority, flags);
}

JrErrorCode broadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    return sendto(handle, 0xFFFFFFFF, 0, bufsize, buffer, priority, 0);
}

JrErrorCode broadcast(int handle,
              unsigned short msg_id,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    return sendto(handle, 0xFFFFFFFF, msg_id, bufsize, buffer, priority, 0);
}

JrErrorCode recvfrom(int handle,
             unsigned long* sender,
             unsigned short* msg_id,
             unsigned int* bufsize,
             char* buffer,
             int* priority)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->recvfrom(sender, bufsize, buffer, priority, msg_id));
}

JrErrorCode recvfrom(int handle,
             unsigned long* sender,
             unsigned int* bufsize,
             char* buffer,
             int* priority)
{
    return recvfrom(handle, sender, NULL, bufsize, buffer, priority);
}

JrErrorCode connect(unsigned long id, char* config_file, int* handle)
{
    if (handle == NULL) return InitFailed;

    // Create and initialize Junior Manager, to manage this 
    // connection to the RTE.  
    JuniorMgr* mgr = new JuniorMgr();
    JrErrorCode ret = mgr->connect(id, config_file);
    if (ret != Ok)
    {
        delete mgr;
        *handle = 0;
    }
    else
        *handle = (int)mgr;
    return ret;
}

// Define the class implementation, too, so we can have both
// a functional and object oriented interface.
JuniorAPI::JuniorAPI():
    handle(0)
{
}
JuniorAPI::~JuniorAPI()
{
    // Delete the manager instance, if we've got one.
    if (handle != 0)
    {
        JuniorMgr* mgr = (JuniorMgr*) handle;
        delete mgr;
    }
}
JrErrorCode JuniorAPI::sendto( unsigned long destination, 
                               unsigned int size, 
                               const char* buffer,
                               int priority,
                               int flags,
                               unsigned short msg_id)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->sendto(destination, size, buffer, priority, flags, msg_id));
}
 
JrErrorCode JuniorAPI::recvfrom( unsigned long* source,
                                 unsigned int* bufsize,
                                 char* buffer,
                                 int* priority,
                                 unsigned short* msg_id)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->recvfrom(source, bufsize, buffer, priority, msg_id));
}

JrErrorCode JuniorAPI::broadcast( unsigned int bufsize,
                                  const char* buffer,
                                  int priority,
                                  unsigned short msg_id)
{
    return sendto(0xFFFFFFFF, bufsize, buffer, priority, 0, msg_id);
}

JrErrorCode JuniorAPI::connect( unsigned long id, 
                                char* config_file )
{
    // Create and initialize Junior Manager, to manage this 
    // connection to the RTE.  
    JuniorMgr* mgr = new JuniorMgr();
    JrErrorCode ret = mgr->connect(id, config_file);
    if (ret != Ok)
    {
        delete mgr;
        handle = 0;
    }
    else
        handle = (int)mgr;
    return ret;
}

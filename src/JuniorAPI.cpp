/*! 
 ***********************************************************************
 * @file      JuniorAPI.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "JuniorAPI_v1.h"
#include "JuniorAPI_v2.h"
#include "JuniorAPI.hpp"
#include "JuniorMgr.h"

using namespace DeVivo::Junior;

JrErrorCode JrSend(int handle,
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
JrErrorCode JrSend(int handle,
           unsigned long destination, 
           unsigned int bufsize, 
           const char* buffer,
           int priority,
           int flags)
{
    return JrSend(handle, destination, 0, bufsize, buffer, priority, flags);
}

JrErrorCode JrBroadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    return JrSend(handle, 0xFFFFFFFF, 0, bufsize, buffer, priority, 0);
}

JrErrorCode JrBroadcast(int handle,
              unsigned short msg_id,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    return JrSend(handle, 0xFFFFFFFF, msg_id, bufsize, buffer, priority, 0);
}

JrErrorCode JrReceive(int handle,
             unsigned long* sender,
             unsigned short* msg_id,
             unsigned int* bufsize,
             char* buffer,
             int* priority,
             int* flags )
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->recvfrom(sender, bufsize, buffer, priority, flags, msg_id));
}

JrErrorCode JrReceive(int handle,
             unsigned long* sender,
             unsigned int* bufsize,
             char* buffer,
             int* priority,
             int* flags )
{
    return JrReceive(handle, sender, NULL, bufsize, buffer, priority, flags);
}

JrErrorCode JrConnect(unsigned long id, const char* config_file, int* handle)
{
    if (handle == NULL) return InitFailed;

    // Create and initialize Junior Manager, to manage this 
    // connection to the RTE.  
    JuniorMgr* mgr = new JuniorMgr();
    JrErrorCode ret;
    if (config_file) ret = mgr->connect(id, config_file);
    else ret = mgr->connect(id, "");
    if (ret != Ok)
    {
        delete mgr;
        *handle = 0;
    }
    else
        *handle = (int)mgr;
    return ret;
}

JrErrorCode JrDisconnect(int handle)
{    
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    delete(mgr);
    return Ok;
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
JrErrorCode JuniorAPI::JrSend( unsigned long destination, 
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
 
JrErrorCode JuniorAPI::JrReceive( unsigned long* source,
                                 unsigned int* bufsize,
                                 char* buffer,
                                 int* priority,
                                 int* flags,
                                 unsigned short* msg_id)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->recvfrom(source, bufsize, buffer, priority, flags, msg_id));
}

JrErrorCode JuniorAPI::JrBroadcast( unsigned int bufsize,
                                  const char* buffer,
                                  int priority,
                                  unsigned short msg_id)
{
    return JrSend(0xFFFFFFFF, bufsize, buffer, priority, 0, msg_id);
}

JrErrorCode JuniorAPI::JrConnect( unsigned long id, 
                                  const char* config_file )
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

JrErrorCode JuniorAPI::JrDisconnect( )
{
    // Delete the manager instance, if we've got one.
    if (handle != 0)
    {
        JuniorMgr* mgr = (JuniorMgr*) handle;
        delete mgr;
    }
    return Ok;
}

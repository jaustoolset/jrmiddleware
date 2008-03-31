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
#include "JuniorAPI.h"
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

JrErrorCode JrConnect(unsigned long id, const char* config_file, int* handle)
{
    if (handle == NULL) return InitFailed;

    // Create and initialize Junior Manager, to manage this 
    // connection to the RTE.  
    JuniorMgr* mgr = new JuniorMgr();
    JrErrorCode ret;
    if ((config_file == NULL) || strlen(config_file) == 0) 
        ret = mgr->connect(id, "");
    else 
        ret = mgr->connect(id, config_file);
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
/*! 
 ***********************************************************************
 * @file      JuniorAPI.cpp
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
#include "JuniorAPI.h"
#include "JuniorRA.h"
#include "JuniorMgr.h"

using namespace DeVivo::Junior;
static std::vector<int> handles;


// This function checks all known handles for pending
// messages.  Any handle with 1 or more messages
// waiting is returned in the list.  This function does 
// not allocate any memory; therefore, the list must be
// allocated by the calling application, with a maximum
// size passed in 'size_of_list'.  This value will be modified
// to equal the total number of handles with messages waiting.
//
JrErrorCode _stdcall JrCheckAllHandles(int* list, int& size_of_list)
{
    JrErrorCode ret = Ok;
    int count = 0;
    if (list == NULL) return InvalidParams;

    // Check each known handle for outstanding messages.
    for (int i=0; i < handles.size(); i++)
    {
        if (handles[i] == 0) continue;
        if (((JuniorMgr*) handles[i])->pending())
        {
            if (count < size_of_list) list[count] = handles[i];
            count++;
        }
    }

    // If we actually have more handles with messages than 
    // the list allows us to return, mark as Overflow.
    if (count > size_of_list) ret = Overflow;
    size_of_list = count;
    return ret;
}


JrErrorCode _stdcall JrSend(int handle,
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

JrErrorCode _stdcall RaSend(int handle,
                   unsigned int bufsize,
                   const char* buffer)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->sendto(bufsize, buffer));
}

JrErrorCode _stdcall RaReceive(int handle,
                      unsigned int* bufsize,
                      char* buffer)
{
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    return (mgr->recvfrom(bufsize, buffer));
}


JrErrorCode _stdcall JrBroadcast(int handle,
              unsigned short msg_id,
              unsigned int bufsize,
              const char* buffer,
              int priority)
{
    return JrSend(handle, 0xFFFFFFFF, msg_id, bufsize, buffer, priority, 0);
}

JrErrorCode _stdcall JrReceive(int handle,
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

JrErrorCode _stdcall JrConnect(unsigned long id, const char* config_file, int* handle)
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
    {
        *handle = (int)mgr;
        handles.push_back((int) mgr);
    }
    return ret;
}

JrErrorCode _stdcall JrDisconnect(int handle)
{    
    if (handle == 0) return NotInitialized;
    JuniorMgr* mgr = (JuniorMgr*) handle;
    delete(mgr);

    // find it in the static list
    for (int i=0; i < handles.size(); i++)
    {
        if (handles[i] == handle)
        {
            handles[i] = 0;
            break;
        }
    }
    return Ok;
}

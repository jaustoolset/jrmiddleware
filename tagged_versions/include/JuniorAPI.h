/*! 
 ***********************************************************************
 * @file      JuniorAPI.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JUNIOR_API_H
#define __JUNIOR_API_H

// Extern the definitions to avoid name mangling
extern "C" {

// Define an enumerated list of error codes used by the Junior API.
typedef enum {Ok, NoMessages, InvalidID, Overflow, InitFailed, 
              InvalidParams, Timeout, UnknownError, NotInitialized} JrErrorCode;

// Define the list of valid flags.  These can be logically AND'ed into
// the "flags" field to allow for more than one flag per message.
const unsigned char GuaranteeDelivery = 0x01;
const unsigned char ServiceConnection = 0x02;
const unsigned char ExperimentalFlag  = 0x04;

// Functional interface.  
JrErrorCode _stdcall JrSend(int handle,
           unsigned long destination, 
           unsigned short msg_id,
           unsigned int size, 
           const char* buffer,
           int priority,
           int flags);

JrErrorCode _stdcall JrReceive(int handle,
             unsigned long* source,
             unsigned short* msg_id,
             unsigned int* bufsize,
             char* buffer,
             int* priority,
             int* flags );

JrErrorCode _stdcall JrBroadcast(int handle,
              unsigned short msg_id,
              unsigned int size,
              const char* buffer,
              int priority);

JrErrorCode _stdcall JrCheckAllHandles(int* list, int& size_of_list);

JrErrorCode _stdcall JrConnect(unsigned long id, 
                                const char* config_file, 
                                int* handle);

JrErrorCode _stdcall JrDisconnect(int handle);

} // end extern "C"
#endif

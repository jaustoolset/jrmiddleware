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
#ifndef __JUNIOR_API_V1_H
#define __JUNIOR_API_V1_H

// Manage the defines for building (or using) the
// Junior Toolset as a DLL.
#if defined(__BUILD_DLL__)
#define DLL_MACRO __declspec(dllexport)
#elif defined(__USE_DLL__)
#define DLL_MACRO __declspec(dllimport)
#else
#define DLL_MACRO 
#endif

#ifndef __JUNIOR_TYPEDEFS
#define __JUNIOR_TYPEDEFS

// Define an enumerated list of error codes used by the Junior API.
typedef enum {Ok, NoMessages, InvalidID, Overflow, InitFailed, 
              AddrUnknown, Timeout, UnknownError, NotInitialized} JrErrorCode;

// Define the list of valid flags.  These can be logically AND'ed into
// the "flags" field to allow for more than one flag per message.
const unsigned char GuarenteeDelivery = 0x01;
const unsigned char ServiceConnection = 0x02;
const unsigned char ExperimentalFlag  = 0x04;

#endif

// Functional interface.  
JrErrorCode DLL_MACRO JrSend(int handle,
           unsigned long destination, 
           unsigned short msg_id,
           unsigned int size, 
           const char* buffer,
           int priority,
           int flags);

JrErrorCode DLL_MACRO JrReceive(int handle,
             unsigned long* source,
             unsigned short* msg_id,
             unsigned int* bufsize,
             char* buffer,
             int* priority,
             int* flags );

JrErrorCode DLL_MACRO JrBroadcast(int handle,
              unsigned short msg_id,
              unsigned int size,
              const char* buffer,
              int priority);

JrErrorCode DLL_MACRO JrConnect(unsigned long id, 
                                const char* config_file, 
                                int* handle);

JrErrorCode DLL_MACRO JrDisconnect(int handle);

#endif

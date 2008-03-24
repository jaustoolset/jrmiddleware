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
#ifndef __JUNIOR_API_V2_H
#define __JUNIOR_API_V2_H

// Manage the defines for building (or using) the
// Junior Toolset as a DLL.
#if defined(__BUILD_DLL__)
#define DLL_MACRO __declspec(dllexport)
#elif defined(__USE_DLL__)
#define DLL_MACRO __declspec(dllimport)
#else
#define DLL_MACRO 
#endif

// Convenient typedefs, enumerations and constants
#ifndef __JUNIOR_TYPEDEFS
#define __JUNIOR_TYPEDEFS
typedef enum {Ok, NoMessages, InvalidID, Overflow, InitFailed, 
              AddrUnknown, Timeout, UnknownError, NotInitialized} JrErrorCode;
const unsigned char GuarenteeDelivery = 0x01;
const int JrMaxPriority = 15;
#endif

// Functional interface.  
//
// Version 2 of the interface does not support tranmission
// of the code.
JrErrorCode DLL_MACRO JrSend(int handle,
           unsigned long destination, 
           unsigned int size, 
           const char* buffer,
           int priority,
           int flags);

JrErrorCode DLL_MACRO JrReceive(int handle,
             unsigned long* source,
             unsigned int* bufsize,
             char* buffer,
             int* priority);

JrErrorCode DLL_MACRO JrBroadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority);

JrErrorCode DLL_MACRO JrConnect(unsigned long id, 
                              char* config_file, 
                              int* handle);

JrErrorCode DLL_MACRO JrDisconnect(int handle);

#endif



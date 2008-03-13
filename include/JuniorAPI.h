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

// Convenient typedefs, enumerations and constants
typedef enum {Ok, NoMessages, InvalidID, Overflow, InitFailed, 
              AddrUnknown, Timeout, UnknownError, NotInitialized} JrErrorCode;
const unsigned char GuarenteeDelivery = 0x01;
const int JrMaxPriority = 15;

// Functional interface.  Note that two forms are currently supported.
// If the compiler definition JR_SEND_MESSAGE_ID is defined,
// JR will also route a two byte message identifier along with the message.
// This allows the receiving end to identify the message type.  Otherwise,
// the application must handle any message identification mechanism.

#ifndef JR_SEND_MESSAGE_ID

JrErrorCode sendto(int handle,
           unsigned long destination, 
           unsigned int size, 
           const char* buffer,
           int priority,
           int flags);

JrErrorCode recvfrom(int handle,
             unsigned long* source,
             unsigned int* bufsize,
             char* buffer,
             int* priority);

JrErrorCode broadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority);

JrErrorCode connect(unsigned long id, char* config_file, int* handle);
JrErrorCode disconnect(int handle);

#else

JrErrorCode sendto(int handle,
           unsigned long destination, 
           unsigned short msg_id,
           unsigned int size, 
           const char* buffer,
           int priority,
           int flags);

JrErrorCode recvfrom(int handle,
             unsigned long* source,
             unsigned short* msg_id,
             unsigned int* bufsize,
             char* buffer,
             int* priority);

JrErrorCode broadcast(int handle,
              unsigned short msg_id,
              unsigned int size,
              const char* buffer,
              int priority);

JrErrorCode connect(unsigned long id, char* config_file, int* handle);
JrErrorCode disconnect(int handle);

#endif

#endif



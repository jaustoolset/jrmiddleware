/*! 
 ***********************************************************************
 * @file      JuniorAPI.h
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
#ifndef __JUNIOR_API_H
#define __JUNIOR_API_H

// Extern the definitions to avoid name mangling
extern "C" {

// Define an enumerated list of error codes used by the Junior API.
typedef enum {Ok, NoMessages, InvalidID, Overflow, InitFailed, 
              InvalidParams, Timeout, UnknownError, NotInitialized, NoMemory} JrErrorCode;

// Define the list of valid flags.  These can be logically AND'ed into
// the "flags" field to allow for more than one flag per message.
const unsigned char GuaranteeDelivery = 0x01;
const unsigned char ServiceConnection = 0x02;
const unsigned char ExperimentalFlag  = 0x04;

#if !(defined WINDOWS) && !(defined WIN32) && !(defined __CYGWIN__)
#define _stdcall
#endif

// Functional interface.  
JrErrorCode _stdcall JrSend(int handle,
           unsigned long destination, 
           unsigned int size, 
           const char* buffer,
           int priority = 6,
           int flags = 0,
		   unsigned short msg_id = 0);

JrErrorCode _stdcall JrReceive(int handle,
             unsigned long* source,
             unsigned int* bufsize,
             char* buffer,
             int* priority = 0,
             int* flags = 0,
			 unsigned short* msg_id = 0);

JrErrorCode _stdcall JrBroadcast(int handle,
              unsigned int size,
              const char* buffer,
              int priority = 6,
			  unsigned short msg_id = 0);

JrErrorCode _stdcall JrCheckAllHandles(int* list, int* size_of_list);

JrErrorCode _stdcall JrConnect(unsigned long id, 
                                const char* config_file, 
                                int* handle);

JrErrorCode _stdcall JrDisconnect(int handle);

} // end extern "C"
#endif

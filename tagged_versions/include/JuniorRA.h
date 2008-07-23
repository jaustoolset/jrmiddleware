/*! 
 ***********************************************************************
 * @file      JuniorRA.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JUNIOR_RA_H
#define __JUNIOR_RA_H

// This is a public header that exports "pass through" functionality
// for JAUS messages that include the 16-byte header as part of the 
// byte stream.  This interface is provided for test and debug only!
// It is the responsibility of the user to ensure that the header is
// properly formatted and compliant.

// Include the standard Junior API
#include "JuniorAPI.h"

// Extern the definitions to avoid name mangling
extern "C" {

JrErrorCode _stdcall RaSend( int handle,
             unsigned int size,
             const char* buffer);

JrErrorCode _stdcall RaReceive( int handle,
                unsigned int* bufsize,
                char* buffer );

} // end extern "C"
#endif

/*! 
 ***********************************************************************
 * @file      JuniorRA.h
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

/*! 
 ***********************************************************************
 * @file      JuniorAPI.hpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JUNIOR_API_HPP
#define __JUNIOR_API_HPP

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

// This header defines a C++ style object interface for Junior.
//
// Optional arguments allow JR to route a two byte message identifier 
// along with the message.  The receiving end to identify the message type.  
// Otherwise, the application must handle any message identification mechanism.
class DLL_MACRO JuniorAPI
{
public:

    JuniorAPI();
    ~JuniorAPI();

    JrErrorCode JrSend( unsigned long destination, 
                        unsigned int size, 
                        const char* buffer,
                        int priority,
                        int flags,
                        unsigned short msg_id = 0);

    JrErrorCode JrReceive( unsigned long* source,
                          unsigned int* bufsize,
                          char* buffer,
                          int* priority,
                          int* flags,
                          unsigned short* msg_id = 0);

    JrErrorCode JrBroadcast( unsigned int bufsize,
                           const char* buffer,
                           int priority,
                           unsigned short msg_id = 0);

    JrErrorCode JrConnect( unsigned long id, 
                           const char* config_file );

    JrErrorCode JrDisconnect( );

private:
    int handle;
};

#endif



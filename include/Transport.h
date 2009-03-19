/*! 
 ***********************************************************************
 * @file      Transport.h
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

#ifndef  __TRANSPORT_H
#define __TRANSPORT_H

#include <string>
#include "JrMessage.h"
#include "ConfigData.h"

namespace DeVivo {
namespace Junior {

class Transport
{
public:
    Transport(){};
   ~Transport(){};

    //
    // Define the error codes
    //
    enum TransportError 
    {
        Ok, NoMessages, InvalidConfigFile, InitFailed, AddrUnknown, Failed, ConnectionClosed 
    };

    // All functions are abstract
    virtual TransportError sendMsg(Message& msg) = 0;
    virtual TransportError recvMsg(MessageList& msglist) = 0;
    virtual TransportError broadcastMsg(Message& msg) = 0;
    virtual TransportError initialize(ConfigData& config) = 0;

    // Debugging
    std::string enumToString( TransportError code );
   
protected:

};

inline std::string Transport::enumToString( TransportError code )
{
    switch (code)
    {
        case Transport::Ok:
            return std::string("Success");
        case Transport::NoMessages:
            return std::string("No messages in queue");
        case Transport::InvalidConfigFile:
            return std::string("Invalid config file");
        case Transport::InitFailed:
            return std::string("Initialization failed");
        case Transport::AddrUnknown:
            return std::string("Unknown destination address");
        case Transport::Failed:
            return std::string("Unknown failure");
        default:
            return std::string("Unkown error code");
    }
}
}} // namespace DeVivo::Junior    

#endif



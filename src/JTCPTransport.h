/*! 
 ***********************************************************************
 * @file      JTCPTransport.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/08/05
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
#ifndef __JAUS_TCP_TRANSPORT_H
#define __JAUS_TCP_TRANSPORT_H

#include "Transport.h"
#include "ConnectionList.h"
#include "TCPConnection.h"
#include "Types.h"

namespace DeVivo {
namespace Junior {


class JTCPTransport : public Transport
{
public:
    JTCPTransport();
   ~JTCPTransport();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError broadcastMsg(Message& msg);
    TransportError recvMsg(MessageList& msglist);
    TransportError initialize(ConfigData& config);

    // These functions are specific to TCP implementation
    TransportError acceptConnections();

protected:

    IpAddressBook       _address_map;      // address book
    JTCPConnectionList  _connectionsList;  // active connections
    int                 _listen_socket;
    bool                _exit_flag;
};
}} // namespace DeVivo::Junior
#endif



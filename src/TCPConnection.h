/*! 
 ***********************************************************************
 * @file      TCPConnection.h
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
#ifndef __JAUS_TCP_CONNECTION_H
#define __JAUS_TCP_CONNECTION_H

#include "Transport.h"
#include "JTCPArchive.h"
#include "ConnectionList.h"
#include <map>

namespace DeVivo {
namespace Junior {


//
// A TCPConnection object manages a single TCP connection.
// Communication is by-directional (sending and receiving).
// We inherit from, but extend, the Connection class.  The basic
// Connection only supports data types (id, address, version)
// while a TCPConnection provides additional functionality
// for sending/receiving on a dedicated socket.
//
class JTCPConnection : public Connection<IP_ADDRESS>
{
public:
    JTCPConnection(int socket);
   ~JTCPConnection();

    // Public interface functions
    Transport::TransportError sendMsg(Message& msg);
    Transport::TransportError recvMsg(MessageList& msglist);

    // Data accessors
    int getSocket(){return _socket;}

protected:
    int               _socket;
    JTCPArchive       _incoming_stream;
    bool              _isStreamActive;
};

//
// Helper class to manage a list of connections
//
class JTCPConnectionList
{
public:
    JTCPConnectionList():_connections(){};
    ~JTCPConnectionList(){};

    // List management functions
    JTCPConnection* addConnection(int socket);
    void closeConnection(int socket);
    JTCPConnection* getConnection(int socket);
    JTCPConnection* getConnection(JAUS_ID id);

    // Public interface functions
    Transport::TransportError sendMsgToAll(Message& msg);
    Transport::TransportError recvMsgs(MessageList& msglist);

protected:

    std::map<int, JTCPConnection*> _connections;
};


}} // namespace DeVivo::Junior
#endif



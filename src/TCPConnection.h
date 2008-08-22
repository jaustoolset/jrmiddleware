/*! 
 ***********************************************************************
 * @file      TCPConnection.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/08/05
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JAUS_TCP_CONNECTION_H
#define __JAUS_TCP_CONNECTION_H

#include "TransportArchive.h"
#include "Transport.h"
#include "Types.h"
#include <map>

namespace DeVivo {
namespace Junior {


//
// A TCPConnection object manages a single TCP connection.
// Communication is by-directional (sending and receiving)
//
class JTCPConnection
{
public:
    JTCPConnection(int socket);
   ~JTCPConnection();

    // Public interface functions
    Transport::TransportError sendMsg(Message& msg);
    Transport::TransportError recvMsg(MessageList& msglist);
    Transport::TransportError close();

    // Data accessors
    int getSocket(){return _socket;}
    JAUS_ID getJausId(){return _id;}
    void setJausId(JAUS_ID id){_id = id;}

protected:

    int               _socket;
    JTCPArchive       _incoming_stream;
    JAUS_ID           _id;
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



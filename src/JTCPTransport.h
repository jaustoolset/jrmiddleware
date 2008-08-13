/*! 
 ***********************************************************************
 * @file      JTCPTransport.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/08/05
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JAUS_TCP_TRANSPORT_H
#define __JAUS_TCP_TRANSPORT_H

#include "Transport.h"
#include "TransportArchive.h"
#include "IpAddressBook.h"
#include "Types.h"
#include <map>

namespace DeVivo {
namespace Junior {

typedef std::list<int> SocketList;
typedef std::list<int>::iterator SocketListIter;
typedef std::map<int, JTCPArchive*> SocketDataMap;



class JTCPTransport : public Transport
{
public:
    JTCPTransport();
   ~JTCPTransport();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError broadcastMsg(Message& msg);
    TransportError recvMsg(MessageList& msglist);
    TransportError initialize(std::string config);

    // These functions are specific to TCP implementation
    TransportError sendMsg(Message& msg, int socket);
    TransportError acceptConnections();
    TransportError closeConnection(int socket);

protected:

    IpAddressBook     _address_map;
    AddressMap<int>   _socket_map;
    SocketList        _socket_list;
    SocketDataMap     _socket_data;
    int               _listen_socket;
    bool              _exit_flag;

};
}} // namespace DeVivo::Junior
#endif



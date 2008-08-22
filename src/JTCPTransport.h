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
#include "IpAddressBook.h"
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
    TransportError initialize(std::string config);

    // These functions are specific to TCP implementation
    TransportError acceptConnections();

protected:

    IpAddressBook       _address_map;
    JTCPConnectionList  _connectionsList;
    int                 _listen_socket;
    bool                _exit_flag;

};
}} // namespace DeVivo::Junior
#endif



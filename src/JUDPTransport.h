/*! 
 ***********************************************************************
 * @file      JUDPTransport.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JAUS_UDP_TRANSPORT_H
#define __JAUS_UDP_TRANSPORT_H

#include "Transport.h"
#include "TransportArchive.h"
#include "HeaderCompression.h"
#include "IpAddressBook.h"
#include <sstream>

namespace DeVivo {
namespace Junior {


class JUDPTransport : public Transport
{
public:
    JUDPTransport();
   ~JUDPTransport();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError broadcastMsg(Message& msg);
    TransportError recvMsg(MessageList& msglist);
    TransportError initialize(std::string config);

protected:

    IpAddressBook            _map;
    int                      _socket;
    HeaderCompressionTable   _inTable, _outTable;
    IP_ADDRESS               _multicastAddr;
    std::list<unsigned long> _interfaces;
    char                     _use_opc;

    // Internal functions to help with compression
    bool uncompressHeader( TransportArchive*, JAUS_ID, struct sockaddr_in& );
    bool compressHeader  ( TransportArchive*, JAUS_ID );

};
}} // namespace DeVivo::Junior
#endif



/*! 
 ***********************************************************************
 * @file      JSerial.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JAUS_SERIAL_TRANSPORT_H
#define __JAUS_SERIAL_TRANSPORT_H

#include "Transport.h"
#include "TransportArchive.h"
#include "AddressMap.h"
#include "ConfigData.h"

namespace DeVivo {
namespace Junior {

#ifndef WINDOWS
typedef int HANDLE;
#endif

class JSerial : public Transport
{
public:
    JSerial();
   ~JSerial();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError broadcastMsg(Message& msg);
    TransportError recvMsg(MessageList& msglist);
    TransportError initialize(std::string config);

protected:
    HANDLE              hComm;
    ConfigData          _config;
    char                _use_opc;
    JSerialArchive      unusedBytes;
    AddressMap<HANDLE>  _map;
    bool                previousByteWasDLE;

    // protected functions
    TransportError sendMsg(Message& msg, HANDLE handle);
    TransportError extractMsgsFromPacket(MessageList& msglist);
    TransportError configureLink();
};
}} // namespace DeVivo::Junior
#endif



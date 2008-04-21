/*! 
 ***********************************************************************
 * @file      JrSockets.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __JR_SOCKETS_H
#define __JR_SOCKETS_H

#include "Transport.h"
#include "AddressMap.h"
#include "OS.h"

namespace DeVivo {
namespace Junior {

// Since Windows and UNIX use different values for socket
// identifiers (strings versus handles), create an environment
// specific typedef.
#ifdef WINDOWS
typedef HANDLE SocketId;
#else
typedef std::string SocketId;
#endif

class JrSocket : public Transport
{
public:
    JrSocket(std::string name);
   ~JrSocket();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError recvMsg(MessageList& msglist);
    TransportError broadcastMsg(Message& msg);
    TransportError initialize(std::string source);
    TransportError setDestination(std::string destination);
    TransportError removeDestination(JAUS_ID id);
    unsigned char  messagesInQueue();
   
protected:

    // Helper function to get around duplicating code in
    // sendMsg and broadcastMst
    TransportError sendMsg(Message& msg, SocketId dest);

    // Helper function to open a return channel
    void openResponseChannel(Message* msg);

    // Internal variables
    bool                 is_connected;
    AddressMap<SocketId> _map;
    SocketId             connected_dest;
    std::string          _socket_name;
    
    // Unfortunately, implementations are different between UNIX
    // and Windows, since Windows does not support named sockets.
    // Instead we use Mailslots, managed by handles.
#ifdef WINDOWS
    SocketId OpenMailslot(std::string name);
    HANDLE sock;
#else
    int sock;
#endif
};
}} // namespace DeVivo::Junior
#endif



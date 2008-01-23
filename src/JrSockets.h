//  JAUS Router Socket implementation
#ifndef  __JR_SOCKETS_H
#define __JR_SOCKETS_H

#include "Transport.h"
#include "AddressMap.h"
#include "OS.h"

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
    JrSocket();
   ~JrSocket();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError recvMsg(Message& msg);
    TransportError broadcastMsg(Message& msg);
    TransportError initialize(std::string source);
    TransportError setDestination(std::string destination);
   
protected:

    // Helper function to get around duplicating code in
    // sendMsg and broadcastMst
    TransportError sendMsg(Message& msg, SocketId dest);

    // Helper function to open a return channel
    void openResponseChannel(Message& msg);

    // Internal variables
    bool                 is_connected;
    AddressMap<SocketId> _map;
    SocketId             connected_dest;
    
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

#endif



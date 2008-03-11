/*! 
 ***********************************************************************
 * @file      JrSockets.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "JrSockets.h"
#include "ConfigData.h"
#include <fcntl.h>
#include <errno.h>
#include <sstream>

using namespace DeVivo::Junior;

#ifdef WINDOWS
#define SOCK_PATH "\\\\.\\mailslot\\"
#else
#define SOCK_PATH "."
#endif

JrSocket::JrSocket(std::string name):
     sock(),
     is_connected(false),
     _map(),
     _socket_name(name)
{
}


JrSocket::~JrSocket()
{
}

#ifdef WINDOWS
SocketId JrSocket::OpenMailslot(std::string name)
{
    std::stringstream s; s << SOCK_PATH; s << name;
    return CreateFile(s.str().c_str(), 
         GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
         NULL, OPEN_EXISTING, 0, NULL);
}
#endif

void JrSocket::openResponseChannel(Message* msg)
{
#ifdef WINDOWS
    // For Windows, we need to open a mailslot back to the sender, if we
    // don't already have it.
    SocketId sockname;
    if (_map.getAddrFromId(msg->getSourceId(), sockname) == false)
    {
        std::stringstream s; s << msg->getSourceId().val;
        HANDLE source = OpenMailslot(s.str());
        if (source != INVALID_HANDLE_VALUE) 
            _map.addAddress(msg->getSourceId(), source);
    }
#else
    // For Unix, we just use the ID as the name of the socket.
    // The AddressMap class will prevent duplicates.
    std::stringstream s; s << SOCK_PATH; s << msg->getSourceId().val;
    _map.addAddress(msg->getSourceId(), s.str());
#endif
}

Transport::TransportError JrSocket::sendMsg(Message& msg, SocketId sockname)
{
    // Serialize the message before sending it.
    Archive archive;
    msg.pack(archive);

    // Send to the given socket
#ifdef WINDOWS
    DWORD cbWritten;
    bool fSuccess = WriteFile( sockname, archive.getArchive(), 
        archive.getArchiveLength(), &cbWritten, NULL);
    if (!fSuccess || (cbWritten != archive.getArchiveLength())) 
    {
        printf("Unable to write to local mailslot.  Message dropped"); 
        return Failed;
    }

#else
    struct sockaddr_un addr;
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, sockname.c_str(), sockname.length());
    sendto(sock, archive.getArchive(), archive.getArchiveLength(), 0,
       (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
#endif

    return Ok;
}

Transport::TransportError JrSocket::sendMsg(Message& msg)
{
    Transport::TransportError result = AddrUnknown;

    // If the socket is connected, the endpoint is pre-specified.
    // We can send the archive without much fuss.
    if (is_connected)
    {
        // Send it to the connected socket
        result = sendMsg(msg, connected_dest);
    }
    else
    {
        // Otherwise, send to the destination id specified.  Note that the destination
        // specified in the message may contain wildcard characters.  We need to loop
        // through all known destinations, sending to any that match (except the source). 
        JAUS_ID dest = msg.getDestinationId();
        for (int i = 0; i < _map.getList().size(); i++)
        {
            if ((msg.getDestinationId() == _map.getList()[i].first) )
                //&&(msg.getSourceId() != _map.getList()[i].first))
            {
                msg.setDestinationId(_map.getList()[i].first);
                result = sendMsg(msg, _map.getList()[i].second);
            }
        }

        // Restore the initial destination identifier before we return.
        msg.setDestinationId( dest );
    }
    return result;
}

Transport::TransportError JrSocket::recvMsg(MessageList& msglist)
{
    // Assume we don't have any messages to return...
    Transport::TransportError ret = NoMessages;

    // Recv the message into a finite sized buffer
    char buffer[4096];
    int bytes = 0;
    //int counter = 0;

    // Check the recv port in a loop, exiting only when we have
    // no messages in the buffer or we've received 10 messages.
    for (int counter = 0; counter < 10; counter++)
    {

#ifdef WINDOWS
 
        // Read the mailslot as if it's a file descriptor
        bool fSuccess; DWORD bytesread;
        fSuccess = ReadFile( sock, buffer, 4096, &bytesread, NULL);  
        if (!fSuccess) break;
        bytes = bytesread;

#else

        struct sockaddr_un addr;
        memset(addr.sun_path, 0, sizeof(addr.sun_path));
        addr.sun_family = AF_UNIX;
        int addr_len = sizeof(struct sockaddr_un);
        bytes = recvfrom(sock, buffer, 4096, 0, 
                         (struct sockaddr*)&addr, 
                         (socklen_t*) &addr_len);

#endif

        // If we didn't receive anything, break from the read loop.
        if (bytes <= 0) break;
 
        // Now that we have a datagram in our buffer, unpack it.
        Archive archive;
        archive.setData(buffer, bytes);

        // And unpack it...
        Message* msg = new Message();
        msg->unpack(archive);

        // If we're not a connected socket, open a response
        // channel to the sender so we can talk to it later.
        if (!is_connected) openResponseChannel(msg);

        // Add the message to the MessageList and change the return value
        msglist.push_back(msg);
        ret = Ok;
    }

    return ret;
}

Transport::TransportError JrSocket::broadcastMsg(Message& msg)
{
    // Connected sockets send to a single destination only.
    if (is_connected)
    {
        sendMsg(msg);
    }
    else
    {
        // Loop through all known destinations, sending the message to
        // each socket that matches the destination (including wildcards).
        for (int i = 0; i < _map.getList().size(); i++)
        {
            if (msg.getDestinationId() == _map.getList()[i].first)
                sendMsg(msg, _map.getList()[i].second);
        }
    }
    return Ok;
}

Transport::TransportError JrSocket::initialize(std::string config_file)
{
    // Set-up is considerably different for UNIX sockets and
    // Windows named pipes.
#ifdef WINDOWS
    std::stringstream s; s << SOCK_PATH; s << _socket_name;
    sock = CreateMailslot(s.str().c_str(), 0, 0, NULL); 
    if (sock == INVALID_HANDLE_VALUE)
    {
        printf("Internal error.  Cannot initialize mailslot for IPC comms\n");
        return Failed;
    }
#else

    // Before creating a sock, make sure that UNIX sockets
    // support enough data grams to parse a large message
    // into 4096 byte chunks.
    int qlen; int qlen_size = sizeof(int);
    sysctl("net.unix.max_dgram_qlen", &qlen, &qlen_size, NULL, qlen_size);
    printf("Got dgram_qlen = %ld\n", qlen);

    // Create the socket
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock==-1) return InitFailed;

    // Bind to the given filename
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    std::stringstream s; s << SOCK_PATH; s << _socket_name;
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    memcpy(addr.sun_path, s.str().c_str(), s.str().length());
    unlink(addr.sun_path);
    int len = s.str().length() + sizeof(addr.sun_family);
    if (bind(sock, (struct sockaddr *)&addr, len) != 0)
    {

        printf("Bind failed for local socket(%s).  err=%d\n", s.str().c_str(), errno);
        return InitFailed;
    }

    // Make it nonblocking
    int flags = fcntl(sock, F_GETFL);
    fcntl( sock, F_SETFL, flags | O_NONBLOCK );

    // Read the configuration file for buffer size info
    ConfigData config;
    config.parseFile(config_file);
    socklen_t buffer_size = 10000;
    config.getValue("MaxBufferSize", buffer_size);

    // Increase the size of the send/receive buffers
    int length = sizeof(buffer_size);
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&buffer_size, length);
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&buffer_size, length);

#endif

    return Ok;
}

Transport::TransportError JrSocket::setDestination(std::string destination)
{
    // Connect to the given endpoint
    
#ifdef WINDOWS
    connected_dest = OpenMailslot(destination);
    if (connected_dest == INVALID_HANDLE_VALUE) return Failed;
#else
    std::stringstream name; name << SOCK_PATH; name << destination;
    connected_dest = name.str();
#endif

    is_connected = true;
    return Transport::Ok;
}



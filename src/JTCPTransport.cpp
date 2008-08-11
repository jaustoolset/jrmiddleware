/*! 
 ***********************************************************************
 * @file      JTCPTransport.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/08/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */

#include "JTCPTransport.h"
#include "Message.h"
#include "ConfigData.h"
#include "OS.h"
#include "JrLogger.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

using namespace DeVivo::Junior;

#ifdef WINDOWS
#define getSocketError WSAGetLastError()
#else
#define getSocketError errno
#define closesocket close
#endif


// Helper function.  This needs to be static so we can
// call it from pthread_create.
static void* JrAcceptConnections(void* arg)
{
    ((JTCPTransport*)arg)->acceptConnections();
    return NULL;
}

// TCP Class implementation
JTCPTransport::JTCPTransport():
    _address_map(),
    _socket_map(),
    _socket_list(),
    _socket_data(),
    _listen_socket(0),
    _exit_flag(false)
{
}

JTCPTransport::~JTCPTransport()
{
    if (_listen_socket > 0) closesocket(_listen_socket);
    //_exit_flag = true;
}

Transport::TransportError JTCPTransport::initialize( std::string filename )
{
    // Open the configuration file
    ConfigData config;
    config.parseFile(filename);

    // Read the configuration file, and set-up defaults for anything
    // that isn't specified.
    unsigned short port = 3794;
    config.getValue("TCP_Port", port);
    int buffer_size = 10000;
    config.getValue("MaxBufferSize", buffer_size);
    std::string address_book;
    config.getValue("TCP_AddressBook", address_book);

#ifdef WINDOWS
    // Must initialize the windows socket library before using
    WSADATA temp;
    WSAStartup(0x22, &temp);
#endif

    // Create the socket
    _listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_socket < 0)
    {
        JrError << "Unable to create socket for TCP communication.  Error: " 
            << getSocketError << std::endl;
        return InitFailed;
    }

    // Bind the socket to the specified port
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(port);
    if (bind(_listen_socket,(struct sockaddr*)&sockAddr,sizeof(sockAddr))<0)
    {
        JrError << "Unable to bind to TCP port " << port << std::endl;
        closesocket(_listen_socket);
        _listen_socket = 0;
        return InitFailed;
    }

    // Set the socket as a listening socket since it's TCP based
    if (listen(_listen_socket, 10) < 0)
    {
        JrError << "Unable to listen on TCP port " << port << std::endl;
        closesocket(_listen_socket);
        _listen_socket = 0;
        return InitFailed;
    }

    // Increase the size of the send/receive buffers
    int length = sizeof(buffer_size);
    setsockopt(_listen_socket, SOL_SOCKET, SO_RCVBUF, (char*)&buffer_size, length);
    setsockopt(_listen_socket, SOL_SOCKET, SO_SNDBUF, (char*)&buffer_size, length);

    // Initialize the address book
    _address_map.loadFromFile(address_book);

    // Spawn the thread that will accept incoming connections
    JrSpawnThread(JrAcceptConnections, this);
    return Ok;
}

// Helper function to send to a given socket
Transport::TransportError JTCPTransport::sendMsg(Message& msg, int destSocket)
{
    Transport::TransportError result = AddrUnknown;

    // Assume we have a valid destination socket.
    // Serialize the message to send.
    JTCPArchive payload;
    Archive msg_archive;
    msg.pack(msg_archive);
    payload.setJausMsgData( msg_archive );

    // Send the data
    int val = send(destSocket, payload.getArchive(), payload.getArchiveLength(), 0);
    if (val < 0) 
    {
        JrError << "Unable to send TCP packet.  Error: " << getSocketError << std::endl;
        result = Failed;
    }
    else if (val != payload.getArchiveLength())
    {
        JrError << "Unable to full TCP packet.  Sent  " << val << " of " <<
            payload.getArchiveLength() << " bytes.\n";
        result = Failed;
    }
    else
    {
        JrDebug << "Sent " << payload.getArchiveLength() << "bytes on TCP connection\n";
        result = Ok;
    }

    return result;
}

Transport::TransportError JTCPTransport::sendMsg(Message& msg)
{
    // Get the destination id from the message.  
    JAUS_ID destId = msg.getDestinationId();

    // First check to see if we have an open socket.
    int destSocket;
    if (!_socket_map.getAddrFromId(destId, destSocket))
    {
        // Didn't find a match in the list of open sockets.
        // Do we know the ip address for the given destination?
        IP_ADDRESS destAddr;
        if (!_address_map.getAddrFromId(destId, destAddr))
        {
            // Didn't find a match of known addresses.  Since
            // TCP only supports point-to-point connections,
            // we have no way of sending this message.
            JrError << "Unknown address for ID: " << destId.val << std::endl;
            return AddrUnknown;
        }

        // Try to open a socket to the specified IP address
        destSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (destSocket < 0)
        {
            JrError << "Unable to create socket for TCP communication.  Error: " 
                << getSocketError << std::endl;
            return Failed;
        }

        // Connect to the given IP address and port
        struct sockaddr_in sockAddr;
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = destAddr.addr;
        sockAddr.sin_port = destAddr.port;
        if (connect(destSocket,(struct sockaddr*)&sockAddr,sizeof(sockAddr))<0)
        {
            JrError << "Unable to connect to TCP port at " << destAddr.toString() << std::endl;
            closesocket(destSocket);
            return Failed;
        }

        // We also need to create a storage buffer to keep incoming data
        JTCPArchive* socket_data = new JTCPArchive();
        if (socket_data == NULL)
        {
            JrError << "Unable to allocate data buffer for incoming TCP messages\n";
            closesocket(destSocket);
            return Failed;
        }

        // debug
        JrDebug << "Opening TCP connection to " << destAddr.toString() << std::endl;

        // With a successful connection, add it to the map and the socket list
        _socket_map.addAddress(destId, destSocket);
        _socket_list.push_back(destSocket);
        _socket_data[destSocket] = socket_data;
    }

    // Getting to this point means we have a valid destination socket.
    // Send on this socket
    return sendMsg(msg, destSocket);
}


Transport::TransportError JTCPTransport::recvMsg(MessageList& msglist)
{
    char buffer[5000];
    Transport::TransportError ret = NoMessages;

    // We need to check each open socket for waiting data.  If we
    // process data on a particular socket, we can 
    // subsequently check the archive to see if it contains a full 
    // message.  Any messages get added to the list returned
    // to the caller.
    SocketListIter iter;
    for (iter = _socket_list.begin(); iter != _socket_list.end(); iter++)    
    {
        // Check the socket for data.  
        struct timeval timeout;
        timeout.tv_sec=0; timeout.tv_usec=0;
        fd_set set; FD_ZERO(&set); FD_SET(*iter, &set);
        if (select(*iter+1, &set, NULL, NULL, &timeout) == 0) continue;

        // getting here means we have data.  pull it.
        int len = recv(*iter, buffer, 5000, 0);
        if (len <= 0) continue;
        JrDebug << "Read " << len << " bytes on TCP port\n";

        // Since TCP data represents a stream, we can't assume the data
        // read represents a complete message.  Add it to the data previously
        // received.
        if (_socket_data.count(*iter) < 1) continue;
        _socket_data[*iter]->setData(buffer, len);

        // If we've accrued a valid packet, shape it into a message
        while (_socket_data[*iter]->isArchiveValid())
        {
            Archive archive;
            unsigned short jausMsgLength;
            _socket_data[*iter]->getMsgLength(jausMsgLength);
            archive.setData( _socket_data[*iter]->getJausMsgPtr(), jausMsgLength);
            Message* msg = new Message();
            msg->unpack(archive);

            // Add the message to the list and change the return value
            JrDebug << "Found valid TCP message (size " << jausMsgLength << ")\n";
            msglist.push_back(msg);
            ret = Ok;

            // Remove this message from the archive.
            _socket_data[*iter]->removeHeadMsg();
        }
    }

    return ret;
}


Transport::TransportError JTCPTransport::broadcastMsg(Message& msg)
{
    // TCP doesn't support true broadcast.  Best we can do is send 
    // the message on all known sockets.
    SocketListIter iter;
    for (iter = _socket_list.begin(); iter != _socket_list.end(); iter++)    
        sendMsg(msg, *iter);
    return Ok;
}

Transport::TransportError JTCPTransport::acceptConnections()
{
    JrDebug << "Started thread to manage TCP connection requests\n";

    // keep looping until requested to exit
    while (!_exit_flag)
    {
        // we need to seed the address size before calling accept()
        int newSock;
        struct sockaddr_in addr;
        int addr_length = sizeof(addr);

        // wait for new connection requests
        if ((newSock = accept(_listen_socket, (struct sockaddr*) &addr, &addr_length)) > 0)
        {
            // throw a little debug
            IP_ADDRESS clientAddr( addr );
            JrDebug << "Received connection request from " << clientAddr.toString() << std::endl;

            // We also need to create a storage buffer to keep incoming data
            JTCPArchive* socket_data = new JTCPArchive();
            if (socket_data != NULL)
            {
                // Add this connection to our incoming sockets list
                _socket_list.push_back(newSock);
                _socket_data[newSock] = socket_data;
            }
            else
                JrError << "Unable to allocate data buffer for incoming TCP messages\n";
        }
    }

    JrDebug << "Closing thread that manages TCP connection requests\n";
    return Ok;
}

/*! 
 ***********************************************************************
 * @file      JTCPTransport.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/08/03
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
#define closesocket close
#define getSocketError errno
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
    _connectionsList(),
    _listen_socket(0),
    _exit_flag(false)
{
}

JTCPTransport::~JTCPTransport()
{
    if (_listen_socket > 0) closesocket(_listen_socket);
	_connectionsList.closeAllConnections();
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

	// Set the socket option to permit immediate re-use after close
	char reuse = 1;
	setsockopt(_listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(char));

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


Transport::TransportError JTCPTransport::sendMsg(Message& msg)
{
    // Get the destination id from the message.  
    JAUS_ID destId = msg.getDestinationId();

	// Also note which header version we're using
	MsgVersion version = (msg.getMessageCode() == 0) ? AS5669A : AS5669;

    // First check to see if we have an open socket.
    JTCPConnection* pDest = _connectionsList.getConnection(destId, version);
    if (!pDest)
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
        int destSocket = socket(AF_INET, SOCK_STREAM, 0);
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

        // Add this to the connection list.  The list manager will create
        // the connection object.
        if ((pDest = _connectionsList.addConnection(destSocket)) == NULL)
        {
            JrError << "Unable to create connection object for TCP messages\n";
            closesocket(destSocket);
            return Failed;
        }

        // debug
        JrDebug << "Opening TCP connection to " << destAddr.toString() << std::endl;

        // Also set the JAUS_ID & version for this destination.
        pDest->setId(destId);
		pDest->setAddress(destAddr);
		pDest->setVersion(version);
    }

    // Getting to this point means we have a valid connection object for
    // the specified destination.  Try to send the message.  If it fails,
    // close the connection.
    if (pDest->sendMsg(msg) != Transport::Ok)
    {
        _connectionsList.closeConnection(pDest->getSocket());
        return Failed;
    }

    return Ok;
}


Transport::TransportError JTCPTransport::recvMsg(MessageList& msglist)
{
    _connectionsList.recvMsgs(msglist);
    return Transport::Ok;
}

Transport::TransportError JTCPTransport::broadcastMsg(Message& msg)
{
    // TCP doesn't support true broadcast.  Best we can do is send 
    // the message on all known sockets.
    _connectionsList.sendMsgToAll(msg);
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
        socklen_t addr_length = sizeof(addr);

        // wait for new connection requests
        if ((newSock = accept(_listen_socket, (struct sockaddr*) &addr, &addr_length)) > 0)
        {
            // throw a little debug
            IP_ADDRESS clientAddr( addr );
            JrDebug << "Received connection request from " << clientAddr.toString() << std::endl;

            // Add this connection to the list
            _connectionsList.addConnection(newSock);
        }
    }

    JrDebug << "Closing thread that manages TCP connection requests\n";
    return Ok;
}

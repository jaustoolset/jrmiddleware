/*! 
 ***********************************************************************
 * @file      JUDPTransport.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
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
#include "JUDPTransport.h"
#include "Message.h"
#include "JUDPArchive.h"
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


JUDPTransport::JUDPTransport():
    _map(),
    _socket(0),
    _multicastAddr(),
    _interfaces(),
    _compatibilityMode(0)
{
}

JUDPTransport::~JUDPTransport()
{
    if (_socket > 0) closesocket(_socket);
}

Transport::TransportError JUDPTransport::initialize( std::string filename )
{
    // Open the configuration file
    ConfigData config;
    config.parseFile(filename);

    // Read the configuration file, and set-up defaults for anything
    // that isn't specified.
    unsigned short port = 3794;
    config.getValue("UDP_Port", port);
    unsigned char multicast_TTL = 16;
    config.getValue("MulticastTTL", multicast_TTL);
    std::string multicast_addr = "239.255.0.1";
    config.getValue("MulticastAddr", multicast_addr);
    int buffer_size = 10000;
    config.getValue("MaxBufferSize", buffer_size);
    config.getValue("CompatibilityMode", _compatibilityMode);
    std::string address_book;
    config.getValue("UDP_AddressBook", address_book);

    // Set-up the multicast address based on config data
    _multicastAddr.port = htons(port);
    _multicastAddr.addr = inet_addr(multicast_addr.c_str());

#ifdef WINDOWS
    // Must initialize the windows socket library before using
    WSADATA temp;
    WSAStartup(0x22, &temp);
#endif

    // Create the socket
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket < 0)
    {
        JrError << "Unable to create socket for UDP communication.  Error: " 
            << getSocketError << std::endl;
        return InitFailed;
    }

    // Bind the socket to the public JAUS port
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(port);
    if (bind(_socket,(struct sockaddr*)&sockAddr,sizeof(sockAddr))<0)
    {
        JrError << "Unable to bind to UDP port " << port << std::endl;
        closesocket(_socket);
        _socket = 0;
        return InitFailed;
    }

    // Increase the size of the send/receive buffers
    int length = sizeof(buffer_size);
    setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&buffer_size, length);
    setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&buffer_size, length);

    // 
    // Set-up for multicast:
    //  1) No loopback
    //  2) TTL value set by configuration file
    /// 3) Send out our socket.
    //  4) Join the multicast group set by configuration file
    //
    char loop = 0; struct ip_mreq mreq;
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &multicast_TTL, sizeof(multicast_TTL));
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &sockAddr, sizeof(sockAddr));
    mreq.imr_multiaddr.s_addr = _multicastAddr.addr; 

    // Using INADDR_ANY causes us to join the multicast group, but only
    // on the default NIC.  When multiple NICs are present, we need to join
    // each manually.  Loop through all available addresses...
    // Get a list of IP addresses associated with this host.
    _interfaces = JrGetIPAddresses();
    std::list<unsigned long>::iterator addy;
    for (addy = _interfaces.begin(); addy != _interfaces.end(); ++addy)
    {
        mreq.imr_interface.s_addr = *addy;
        if (setsockopt (_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
            (const char*) &mreq, sizeof(mreq)) != 0)
			JrError << "Error joining multicast group : " << getSocketError << std::endl;

		JrInfo << "Found network interface: " << 
			inet_ntoa(*(in_addr*) &mreq.imr_interface.s_addr);
    }

    // UDP sockets support run-time discovery.  It's also possible, however,
    // to initialize the map statically through a config file.
    _map.loadFromFile(address_book);

    return Ok;
}

Transport::TransportError JUDPTransport::sendMsg(Message& msg)
{
    // Assume the worst...
    Transport::TransportError result = AddrUnknown;

    //
    // Get the destination id from the message.  
    //
    JAUS_ID destId = msg.getDestinationId();

    //
    // Creating a byte stream (payload) for the message
    //
    JUDPArchive archive;
    
    //
    // Loop through all known destination, sending to each match.
    //
    for (int i = 0; i < _map.getList().size(); i++)
    {
        // Store a local variable for convenience
        JAUS_ID id = _map.getList()[i]->getId();

        // Check this ID against the message's destination
        if ((id == destId) && (msg.getSourceId() != id))
        {
            //
            // Change the destination to the specific JAUS_ID.  In most cases,
            // this does nothing.  In some cases, it will remove wildcard characters
            // to prevent messages from being repeatedly forwarded.  This
            // must be done before the message is packed.
            //
            // NOTE!!! We should optimize this later so we're not always
            // packing the message for each destination.
            //
            msg.setDestinationId(id);

            //
            // For each destination, we use information from the last received
			// message to determine the message version to send.
			// If no entry exists in the map, the
            // default selection from _compatibilityMode is taken.
			MsgVersion version = UnknownVersion;
		    if (!_map.getMsgVersion(id, version) || (version == UnknownVersion))
			{
				// this is a problem case.  we really should never be here.
				version = (_compatibilityMode == 1) ? AS5669 : AS5669A;
				JrWarn << "Unable to determine header version for " << id.val
					<< ".  Using: " << VersionEnumToString(version) << std::endl;
			}

            //
            // Now pack the message into the transport archive
            //
			archive.pack(msg, version);

            // Create the destination address structure
            struct sockaddr_in dest;
            dest.sin_family = AF_INET;
            dest.sin_addr.s_addr = _map.getList()[i]->getAddress().addr;
            dest.sin_port = _map.getList()[i]->getAddress().port;
            
            // Lastly, send the message.  
            int val = sendto(_socket, archive.getArchive(), archive.getArchiveLength(),
                       0, (struct sockaddr*) &dest, sizeof(dest));
            if (val < 0) 
            {
                JrError << "Unable to send UDP packet.  Error: " << getSocketError << std::endl;
                result = Failed;
            }
            else 
            {
                JrDebug << "Sent " << archive.getArchiveLength() << " bytes on UDP port\n";
                result = Ok;
            }
        }
    }

    // Note that we may have changed the destination of the message,
    // so we need to restore it before returning.  In most cases,
    // this will do nothing.
    msg.setDestinationId( destId );
    return result;
}


Transport::TransportError JUDPTransport::recvMsg(MessageList& msglist)
{
    char buffer[5000];
    Transport::TransportError ret = NoMessages;
    unsigned short jausMsgLength;
    unsigned char version;
 
    // Check the recv port in a loop, exiting only when we have
    // no messages in the buffer (or we received 10 packets).
    for (int i=0; i<10; i++)
    {
        // See if we have anything waiting before we call recvfrom
        struct timeval timeout;
        timeout.tv_sec=0; timeout.tv_usec=0;
        fd_set set; FD_ZERO(&set); FD_SET(_socket, &set);
        if (select(_socket+1, &set, NULL, NULL, &timeout) == 0) break;

        // Check the socket for a message
        struct sockaddr_in source;
        int source_length = sizeof(source);
        int result = recvfrom(_socket, buffer, 5000, 0,
                              (struct sockaddr*) &source, 
                              (socklen_t*) &source_length);
        if (result <= 0) break;
        JrDebug << "Read " << result << " bytes on UDP port\n";

        // 
        // Pull off the source IP info for convenience.  
        //
        IP_ADDRESS sourceAddr( source );

        //
        // Getting to this point means we have a message.  
        // Check what type, so we use the appropriate archive.  
        // 
        JUDPArchive raw_msg;

        // Otherwise, set the data from the receive buffer.
        raw_msg.setData( buffer, result );

        // A single packet may have multiple JAUS messages on it, each
        // with there own header compression flags.  We need to parse through
        // the entire packet, remove each message one at a time and
        // adding it to the return list.
        while (raw_msg.isArchiveValid())
        {
            // Extract the payload into a message
			Message* msg = new Message();
			raw_msg.unpack(*msg);

            //
            // Add the source to the transport discovery map.
            // We also need to remember the format that was used.
            //
            _map.addElement( msg->getSourceId(), sourceAddr, raw_msg.getVersion() );

            // Add the message to the list and change the return value
            JrDebug << "Found valid UDP message (size " << msg->getDataLength() << 
                ", seq " << msg->getSequenceNumber() << ")\n";
            msglist.push_back(msg);
            ret = Ok;

            // Remove this message from the archive, so
            // we can process the next message in the packet.
            raw_msg.removeHeadMsg( );
        }
    }

    // If we didn't find any message, return NoMessage.  Otherwise, Ok.
    return ret;
}

Transport::TransportError JUDPTransport::broadcastMsg(Message& msg)
{
    TransportError ret = Ok;

    //
    // Serialize the message.  Start by
    // creating a byte stream (payload) that contains the appropriate
    // header.  Note that Jr will only broadcast with the AS5669A header,
	// unless the CompatibilityMode config parameter is changed.  If
	// maximum compatibility is selected, the message will
	// be broadcasted multiple times, each with different header
	// versions.
    //
	if (_compatibilityMode != 0) ret = broadcastMsg(msg, AS5669);
	if (_compatibilityMode != 1) ret = broadcastMsg(msg, AS5669A);
	return ret;
}

Transport::TransportError JUDPTransport::broadcastMsg(Message& msg,
													  MsgVersion version)
{
	TransportError ret = Ok;

    //
    // Now pack the message for network transport and append
    // it on the JUDP archive.
    //
	JUDPArchive archive;
    archive.pack( msg, version );

    // Create the destination address structure
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = _multicastAddr.addr;
    dest.sin_port = _multicastAddr.port;

    // Send message on all available interfaces
    std::list<unsigned long>::iterator iter;
    for (iter = _interfaces.begin(); iter != _interfaces.end(); ++iter)
    {
        struct in_addr sockAddr;
        sockAddr.s_addr = *iter;
        setsockopt (_socket, IPPROTO_IP, IP_MULTICAST_IF, 
            (const char*) &sockAddr, sizeof(sockAddr));

        // Lastly, send the message.
        if (sendto(_socket, archive.getArchive(), archive.getArchiveLength(),
                   0, (struct sockaddr*) &dest, sizeof(dest)) < 0)
        {
            JrError << "Failed to broadcast UDP message on interface " <<
                inet_ntoa( *(struct in_addr*) &sockAddr.s_addr ) <<
                ".  Error: " << getSocketError << std::endl;
            ret = Failed;
        }
        else
        {
            JrDebug << "Broadcasted " << archive.getArchiveLength() <<
                " bytes on interface " << inet_ntoa( *(struct in_addr*) &sockAddr.s_addr ) 
                << std::endl;
        }
    }

	return ret;
}

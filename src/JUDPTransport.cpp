/*! 
 ***********************************************************************
 * @file      JUDPTransport.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */

#include "JUDPTransport.h"
#include "Message.h"
#include "ConfigData.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

using namespace DeVivo::Junior;

#ifdef WINDOWS
#define getSocketError WSAGetLastError()
#else
#define getSocketError errno
#endif


JUDPTransport::JUDPTransport():
    _map(),
    _socket(0),
    _inTable(),
    _outTable(),
    _multicastAddr()
{
}

static std::map< unsigned long, Archive> Messages;


JUDPTransport::~JUDPTransport()
{
}

Transport::TransportError JUDPTransport::initialize( std::string filename )
{
#ifdef WINDOWS
    // Must initialize the windows socket library before using
    WSADATA temp;
    WSAStartup(0x22, &temp);
#endif

    // Read the configuration file, and set-up defaults for anything
    // that isn't specified.
    ConfigData config;
    config.parseFile(filename);
    unsigned short port = 3794;
    config.getValue("UDP_Port", port);
    unsigned short multicast_TTL = 1;
    config.getValue("MulticastTTL", multicast_TTL);
    std::string multicast_addr = "224.1.0.1";
    config.getValue("MulticastAddr", multicast_addr);
    int buffer_size = 10000;
    config.getValue("MaxBufferSize", buffer_size);

    // Set-up the multicast address based on config data
    _multicastAddr.port = port;
    _multicastAddr.addr = inet_addr(multicast_addr.c_str());
    
    // Create the socket
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket < 0)
    {
        printf("Unable to create socket for UDP communication.\n");
        return InitFailed;
    }

    // Bind the socket to the public JAUS port
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(port);
    if (bind(_socket,(struct sockaddr*)&sockAddr,sizeof(sockAddr))<0)
    {
        printf("Unable to bind to port %ld.  Returning failed.\n", port);
        return InitFailed;
    }

    //
    // Make it non-blocking
    //
#if WINDOWS
    unsigned long flags = 1;
    ioctlsocket(_socket, FIONBIO, &flags);
#else
    int flags = fcntl(_socket, F_GETFL);
    fcntl( _socket, F_SETFL, flags | O_NONBLOCK );
#endif

    // 
    // Set-up for multicast:
    //  1) No loopback
    //  2) TTL value set by configuration file
    /// 3) Send out our socket.
    //  4) Join the multicast group set by configuration file
    //
    char loop = 0;
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &multicast_TTL, sizeof(multicast_TTL));
    setsockopt (_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &sockAddr, sizeof(sockAddr));
    struct ip_mreq mreq;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = _multicastAddr.addr; 
    setsockopt (_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &mreq, sizeof(mreq));

    // Increase the size of the send/receive buffers
    int length = sizeof(buffer_size);
    setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&buffer_size, length);
    setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&buffer_size, length);

    getsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&buffer_size, &length);
    printf("Configured UDP socket for size: %ld\n", buffer_size);

    return Ok;
}


void JUDPTransport::packHdr(JUDPArchive& packed_msg)
{
    // JUDP header is packed as BigEndian.
    packed_msg.setPackMode( Archive::BigEndian );

    // For JUDP header, the first byte includes only the 
    // transport version.
    packed_msg << (char) 1;

    // Next two bytes are the hc flags (default is no compression)
    // THese values may be adjusted before transmission.
    packed_msg << (unsigned short) 0;

    // Next comes message length.  This value needs to be adjusted 
    // before transmission.
    packed_msg << (unsigned short) 0;
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
    // Loop through all known destination, sending to each match.
    //
    for (int i = 0; i < _map.getList().size(); i++)
    {
        if ((_map.getList()[i].first == destId) &&
            (msg.getSourceId() != _map.getList()[i].first))
        {
            //
            // Serialize the message.  Start by
            // creating a byte stream (payload) that contains the JUDP
            // header.
            //
            JUDPArchive payload;
            packHdr( payload );
            payload.setMsgLength( msg.getMsgLength() );

            //
            // Change the destination to the specific JAUS_ID.  In most cases,
            // this does nothing.  In some cases, it will remove wildcard characters
            // to prevent messages from being repeatedly forwarded.
            //
            msg.setDestinationId(_map.getList()[i].first);

            //
            // Now pack the message for network transport and append
            // it on the JUDP archive.
            //
            Archive msg_archive;
            msg.pack(msg_archive);
            payload.append( msg_archive );

            //
            // Apply header compression
            //
            compressHeader( payload, _map.getList()[i].first );

            // Create the destination address structure
            struct sockaddr_in dest;
            dest.sin_family = AF_INET;
            dest.sin_addr.s_addr = _map.getList()[i].second.addr;
            dest.sin_port = _map.getList()[i].second.port;
            
            // Lastly, send the message.         
            int val =sendto(_socket, payload.getArchive(), payload.getArchiveLength(),
                       0, (struct sockaddr*) &dest, sizeof(dest));// < 0 )
            if (val < 0)
            {
                printf("Unable to send message to %s:%d (return=%ld)\n",val,
                       inet_ntoa( *(struct in_addr*) &dest.sin_addr.s_addr ),
                       htons(dest.sin_port));
                printf("Sendto failed with last error=%d\n", getSocketError);
                result = Failed;
            }
            else
            {
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
        // Check the socket for a message
        struct sockaddr_in source;
        int source_length = sizeof(source);
        int result = recvfrom(_socket, buffer, 5000, 0,
                              (struct sockaddr*) &source, 
                              (socklen_t*) &source_length);
         
        // If there are no new messages, stop checking.
        if (result <= 0) break;

        //
        // Getting to this point means we have a message.  
        // Stuff it into a JUDP Archive, so we can access pieces of the message
        // for analysis.
        // 
        JUDPArchive raw_msg;
        raw_msg.setData( buffer, result );

        // 
        // Pull off the source IP info for convenience.  Look-up the corresponding
        // JAUS_ID.
        //
        IP_ADDRESS sourceAddr( source );
        JAUS_ID sourceId;
        _map.getIdFromAddr( sourceId, sourceAddr );
        
        // 
        // Check JUDP version (should be "1")
        //
        raw_msg.getVersion(version);
        if ( version != 1)
        {
            // This message contains an invalid version.  Drop it.
            continue;
        }

        // A single JUDP packet may have multiple JAUS messages on it, each
        // with there own header compression flags.  We need to parse through
        // the entire packet, remove each message one at a time and
        // adding it to the return list.
        while (raw_msg.getArchiveLength() > 1)
        {
            // Handle header compression.  If uncompressing the message fails,
            // silently discard the message (but not the remainder of the packet)
            if (!uncompressHeader( raw_msg, sourceId, source ))
            {
                // Remove this message from the JUDP archive, so
                // we can process the next message in the packet.
                raw_msg.getMsgLength( jausMsgLength );
                raw_msg.removeAt(1, 4+jausMsgLength);
                continue;
            }

            // If the message length is zero, this message was only a transport
            // message (probably a Header Compression message).  Nothing more
            // to do.
            raw_msg.getMsgLength( jausMsgLength );
            if ( jausMsgLength != 0 )
            {
                // Extract the payload into a message
                // UGH!! Two copies here.  Need to eliminate this.
                Archive archive;
                archive.setData( raw_msg.getJausMsgPtr(), jausMsgLength );
                Message* msg = new Message();
                msg->unpack(archive);

                //
                // Add the source to the transport discovery map.
                //
                _map.addAddress( msg->getSourceId(), sourceAddr );

                // Add the message to the list and change the return value
                msglist.push_back(msg);
                ret = Ok;
            }

            // Remove this message from the JUDP archive, so
            // we can process the next message in the packet.
            raw_msg.removeAt(1, 4+jausMsgLength);
        }
    }

    // If we didn't find any message, return NoMessage.  Otherwise, Ok.
    return ret;
}


bool JUDPTransport::uncompressHeader( JUDPArchive& packed_msg,
                                      JAUS_ID  source,
                                      struct sockaddr_in& sourceAddr )
{
    // Behavior changes based on the HC values.  Extract  the flags.
    unsigned char flags;
    packed_msg.getHCFlags( flags );

    if (flags == 0)
    {
        // No compression requested.  Nothing to do.
        return true;
    }
    else if ( flags == 1)
    {
        // HEADER COMPRESSION STILL NEEDS WORK.  NOT YET FUNCTIONAL.
        return true;

        // source has requested compression.  Add an entry to our table
        _inTable.update( source, packed_msg );

        // Create the acceptance message, using the JUDP header from the 
        // incoming message for convenience.  We need to change the HC flags
        // and clear the message length.
        JUDPArchive response;
        response.setData( packed_msg.getArchive(), 5 );
        response.setMsgLength( 0 );
        response.setHCFlags( 2 );

        //printf("Send header compression response to %s:%d\n",
        //       inet_ntoa( *(struct in_addr*) &sourceAddr.sin_addr.s_addr ),
        //       htons(sourceAddr.sin_port));

        // Lastly, send the message back to the source
        sendto(_socket, response.getArchive(), 
                   response.getArchiveLength(), 0,
                    (struct sockaddr*) &sourceAddr, sizeof(sourceAddr));
        return true;
    }
    else if (flags == 2)
    {
        // acceptance message for header compression.  Update the entry.
        _outTable.update( source, packed_msg );
        return true;
    }
    else if (flags == 3)
    {
        // CURRENTLY AN ERROR CASE!  WE SHOULD NEVER RECEIVE A COMPRESSED MSG.
        return false;

        // Compressed message received.  Uncompress it.
        _inTable.uncompress( source, packed_msg );
    }
    
    return false;
}


bool JUDPTransport::compressHeader( JUDPArchive& packed_msg,
                                    JAUS_ID  destId )
{
    // HEADER COMPRESSION STILL NEEDS WORK.  NOT YET FUNCTIONAL.
    return false;

    // Check for a special case.  If we're broadcasting
    // the messsage, don't try to compress anything 
    if (destId.containsWildcards()) return false;

    // Header compression is handled within the table.
    return (_outTable.compress( destId, packed_msg ));
}

Transport::TransportError JUDPTransport::broadcastMsg(Message& msg)
{
    // Serialize the message.  Start by
    // creating a byte stream (payload) that contains the JUDP
    // header.
    //
    JUDPArchive payload;
    packHdr( payload );
    payload.setMsgLength( msg.getMsgLength() );

    //
    // Now pack the message for network transport and append
    // it on the JUDP archive.
    //
    Archive msg_archive;
    msg.pack(msg_archive);
    payload.append( msg_archive );

    // Create the destination address structure
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = _multicastAddr.addr;
    dest.sin_port = htons(_multicastAddr.port);
    
    // Lastly, send the message.
    if (sendto(_socket, payload.getArchive(), payload.getArchiveLength(),
               0, (struct sockaddr*) &dest, sizeof(dest)) < 0 )
    {
        printf("Sendto (multicast) failed with getlasterror=%d\n", getSocketError);
        return Failed;
    }
    
    // debug
    //std::cout << "Dumping archive..." << std::endl;
    //payload.printArchive();
    return Transport::Ok;
}

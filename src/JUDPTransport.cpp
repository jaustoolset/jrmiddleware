//  JAUS UDP Transport implementation

#include "JUDPTransport.h"
#include "Message.h"
//#include <netdb.h>
//#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
//#include <sys/socket.h>


JUDPTransport::JUDPTransport():
    _map(),
    _socket(0),
    _inTable(),
    _outTable()
{
}



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
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket < 0)
    {
        printf("Unable to create socket for port 3794.\n");
        return InitFailed;
    }

    // Bind the socket to the public JAUS port
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(3794);
    if (bind(_socket,(struct sockaddr*)&sockAddr,sizeof(sockAddr))<0)
    {
        printf("Unable to bind to port 3794\n");
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
    //  2) TTL value of 1
    /// 3) Send out our socket.
    //  4) Join the multicast group
    //
    char loop = 0;
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    char ttl = 1;
    setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    setsockopt (_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &sockAddr, sizeof(sockAddr));
    struct ip_mreq mreq;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = inet_addr("224.1.0.1"); 
    setsockopt (_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &mreq, sizeof(mreq));
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
            payload << msg_archive.getArchiveLength();
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
            if (sendto(_socket, payload.getArchive(), payload.getArchiveLength(),
                       0, (struct sockaddr*) &dest, sizeof(dest)) < 0 )
            {
                printf("Unable to send message to %s:%d\n",
                       inet_ntoa( *(struct in_addr*) &dest.sin_addr.s_addr ),
                       htons(dest.sin_port));
                printf("Sendto failed with errno=%d\n", errno);
                result = Failed;
            }
            else
                result = Ok;
    
        }
    }

    // Note that we may have changed the destination of the message,
    // so we need to restore it before returning.  In most cases,
    // this will do nothing.
    msg.setDestinationId( destId );
    return result;
}


Transport::TransportError JUDPTransport::recvMsg(Message& msg)
{
    char buffer[5000];

    // Check the socket for a message
    struct sockaddr_in source;
    int source_length = sizeof(source);
    int result = recvfrom(_socket, buffer, 5000, 0,
                          (struct sockaddr*) &source, &source_length);
    if (result <= 0)
    {
        // No new messages.
        return NoMessages;
    }

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
    unsigned char version;
    raw_msg.getVersion(version);
    if ( version != 1)
    {
        printf("Invalid message received from %s:%d.  Bad version.\n",
                  inet_ntoa(*(struct in_addr*)&sourceAddr.addr), 
                  ntohs(sourceAddr.port));
        return NoMessages;
    }

    //
    // Handle header compression
    //
    uncompressHeader( raw_msg, sourceId, source );

    //
    // If the message length is zero, this message was only a transport
    // message (probably a Header Compression message).  Nothing more
    // to do.
    unsigned short jausMsgLength;
    raw_msg.getMsgLength( jausMsgLength );
    if ( jausMsgLength == 0)
        return NoMessages;

    // Extract the payload into a message
    Archive archive;
    archive.setData( raw_msg.getJausMsgPtr(), jausMsgLength );
    msg.unpack(archive);

    //
    // Add the source to the transport discovery map.
    //
    _map.addAddress( msg.getSourceId(), sourceAddr );

    return Ok;
}


void JUDPTransport::uncompressHeader( JUDPArchive& packed_msg,
                                      JAUS_ID  source,
                                      struct sockaddr_in& sourceAddr )
{
    // Behavior changes based on the HC values.  Extract  the flags.
    unsigned char flags;
    packed_msg.getHCFlags( flags );

    if (flags == 0)
    {
        // No compression requested.  Nothing to do.
        return;
    }
    else if ( flags == 1)
    {

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
        if (sendto(_socket, response.getArchive(), 
                   response.getArchiveLength(), 0,
                    (struct sockaddr*) &sourceAddr, sizeof(sourceAddr)) < 0 )
        {
            printf("Unable to send header compression response to %s:%d\n",
               inet_ntoa( *(struct in_addr*) &sourceAddr.sin_addr.s_addr ),
               htons(sourceAddr.sin_port));
            printf("Sendto failed with errno=%d\n", errno);
            return;
        }
    }
    else if (flags == 2)
    {
        // acceptance message for header compression.  Update the entry.
        _outTable.update( source, packed_msg );
    }
    else if (flags == 3)
    {
        // Compressed message received.  Uncompress it.
        _inTable.uncompress( source, packed_msg );
    }
    else
        printf("Unknown flag: %ld\n", flags);
}


void JUDPTransport::compressHeader( JUDPArchive& packed_msg,
                                    JAUS_ID  destId )
{
    // Check for a special case.  If we're broadcasting
    // the messsage, don't try to compress anything 
    if (destId == 0xFFFFFFFF) return ;

    // Header compression is handled within the table.
    _outTable.compress( destId, packed_msg );
}

Transport::TransportError JUDPTransport::broadcastMsg(Message& msg)
{
    // Serialize the message.  Start by
    // creating a byte stream (payload) that contains the JUDP
    // header.
    //
    JUDPArchive payload;
    packHdr( payload );

    //
    // Now pack the message for network transport and append
    // it on the JUDP archive.
    //
    Archive msg_archive;
    msg.pack(msg_archive);
    payload << msg_archive.getArchiveLength();
    payload.append( msg_archive );

    // Create the destination address structure
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("224.1.0.1");
    dest.sin_port = htons(3794);
    
    // Lastly, send the message.
    if (sendto(_socket, payload.getArchive(), payload.getArchiveLength(),
               0, (struct sockaddr*) &dest, sizeof(dest)) < 0 )
    {
        printf("Sendto (multicast) failed with errno=%d\n", errno);
        return Failed;
    }
    
    // debug
    //std::cout << "Dumping archive..." << std::endl;
    //payload.printArchive();
    return Transport::Ok;
}

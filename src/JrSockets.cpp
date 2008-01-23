//  JAUS Router Socket implementation
#include "JrSockets.h"
#include <fcntl.h>
#include <errno.h>
#include <sstream>

#ifdef WINDOWS
#define SOCK_PATH "\\\\.\\mailslot\\"
#else
#define SOCK_PATH "/"
#endif

JrSocket::JrSocket():
     sock(),
     is_connected(false),
     _map()
{
}


JrSocket::~JrSocket()
{
}

#ifdef WINDOWS
SocketId JrSocket::OpenMailslot(std::string name)
{
    std::stringstream s; s << SOCK_PATH; s << name;
    printf("Opening mailslot: %s\n", s.str().c_str());
    return CreateFile(s.str().c_str(), 
         GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
         NULL, OPEN_EXISTING, 0, NULL);
}
#endif

void JrSocket::openResponseChannel(Message& msg)
{
#ifdef WINDOWS
    // For Windows, we need to open a mailslot back to the sender, if we
    // don't already have it.
    SocketId sockname;
    if (_map.getAddrFromId(msg.getSourceId(), sockname) == false)
    {
        std::stringstream s; s << msg.getSourceId().val;
        HANDLE source = OpenMailslot(s.str());
        if (source != INVALID_HANDLE_VALUE) 
            _map.addAddress(msg.getSourceId(), source);
    }
#else
    // For Unix, we just use the ID as the name of the socket.
    // The AddressMap class will prevent duplicates.
    std::stringstream s; s << SOCK_PATH; s << msg.getSourceId().val;
    _map.addAddress(msg.getSourceId(), s.str());
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
    //printf("SOCKET: Writing archive size=%ld\n", archive.getArchiveLength());
    bool fSuccess = WriteFile( sockname, archive.getArchive(), 
        archive.getArchiveLength(), &cbWritten, NULL);
    if (!fSuccess || (cbWritten != archive.getArchiveLength())) 
    {
        printf("WriteFile failed"); 
        return Failed;
    }

#else
    struct sockaddr_un addr;
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, sockname.c_str(), sockname.length());
    sendto(sock, archive.getArchive(), archive.getArchiveLength(), 0,
       (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    //printf("Back from sending %d bytes on socket\n", archive.getArchiveLength());
#endif

    return Ok;
}

Transport::TransportError JrSocket::sendMsg(Message& msg)
{
    Transport::TransportError result = Ok;

    // If the socket is connected, the endpoint is pre-specified.
    // We can send the archive without much fuss.
    if (is_connected)
    {
        // Send it to the connected socket
        sendMsg(msg, connected_dest);
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
                sendMsg(msg, _map.getList()[i].second);
            }
        }

        // Restore the initial destination identifier before we return.
        msg.setDestinationId( dest );
    }

    return Ok;
}

Transport::TransportError JrSocket::recvMsg(Message& msg)
{
    // Recv the message into a finite sized buffer
    char buffer[4096];
    int bytes;

#ifdef WINDOWS
    bool fSuccess;

    // Read the mailslot as if it's a file descriptor
    DWORD bytesread;
    fSuccess = ReadFile( sock, buffer, 4096, &bytesread, NULL);  
    if (!fSuccess || (bytesread == 0)) return Transport::NoMessages;
    bytes = bytesread;

#else

    struct sockaddr_un addr;
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    addr.sun_family = AF_UNIX;
    int addr_len = sizeof(struct sockaddr_un);
    bytes = recvfrom(sock, buffer, 4096, 0, (struct sockaddr*)&addr, &addr_len);
    //printf("Back from recvfrom %ld\n", bytes);
    if (bytes == -1) return Transport::NoMessages;

#endif
 
    // Now that we have a datagram in our buffer, unpack it.
    if (bytes > 0)
    {
        //printf("SOCKET: recv'ed %d bytes\n", bytes);
        // Found a message.  Unpack it.
        Archive archive;
        archive.setData(buffer, bytes);
        msg.unpack(archive);

        // If we're not a connected socket, open a response
        // channel to the sender so we can talk to it later.
        if (!is_connected) openResponseChannel(msg);
    }
    return Transport::Ok;
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
        // Loop through all known destinations, sending the message to each.
        for (int i = 0; i < _map.getList().size(); i++)
            sendMsg(msg, _map.getList()[i].second);
    }
    return Ok;
}

Transport::TransportError JrSocket::initialize(std::string source)
{
    // Set-up is considerably different for UNIX sockets and
    // Windows named pipes.
#ifdef WINDOWS
    std::stringstream s; s << SOCK_PATH; s << source;
    printf("Trying to initialize: %s\n", s.str().c_str());
    sock = CreateMailslot(s.str().c_str(), 0, 0, NULL); 
    if (sock == INVALID_HANDLE_VALUE)
    {
        printf("Cannot initialize mailslot for IPC comms\n");
        return Failed;
    }
#else

    // Create the socket
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock==-1) return InitFailed;

    // Bind to the given filename
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    std::stringstream s; s << SOCK_PATH; s << source;
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    memcpy(addr.sun_path, s.str().c_str(), s.str().length());
    unlink(addr.sun_path);
    int len = s.str().length() + sizeof(addr.sun_family);
    printf("Bind: %s (len=%d)\n", addr.sun_path, len);
    if (bind(sock, (struct sockaddr *)&addr, len) != 0)
    {
        printf("Bind failed.  err=%d\n", errno);
        return InitFailed;
    }

    // Make it nonblocking
    int flags = fcntl(sock, F_GETFL);
    fcntl( sock, F_SETFL, flags | O_NONBLOCK );

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



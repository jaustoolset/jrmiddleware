// This is the principle file for the realization of the Junior RTE.

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "JrSockets.h"
#include "JUDPTransport.h"
#include "Transport.h"
#include "Types.h"
#include "OS.h"


// Convenient typedefs
typedef std::list<unsigned long>     ConnectionList;
typedef std::list<JAUS_ID>::iterator ConnectionListIter;


// Main loop
int main(int argc, char* argv[])
{
    printf("Hello, and welcome to the JuniorRTE\n");

    // Fork off a child and kill the parent so that we get adopted by the O/S
#if 0
    int i=fork();
    if (i != 0) exit(0);
#endif

    // Break for terminal signals.
    //setsid();

    // Create the public socket that allows APIs to find us.
    JrSocket publicSocket;
    if (publicSocket.initialize(std::string("JuniorRTE")) != Transport::Ok)
    {
        printf("Unable to initialize internal socket.  Exiting with error...\n");
        exit(1);
    }

    // Maintain a list of connected clients.  Note that we store the
    // raw unsigned long, rather than the JAUS_ID, so that operator== means 
    // "strictly equal to".  This allows us to detected when a message is for a local
    // client, and a local client only (it contains no wildcard characters).
    ConnectionList _clients;

    // Create a list of all supported transports.
    std::list<Transport*> _transports;
    std::list<Transport*>::iterator _iter;
    _transports.push_back(&publicSocket);

    // Add UDP
    JUDPTransport udp;
    if (udp.initialize("") != Transport::Ok)
    {
        printf("Unable to initialize UDP communications.\n");
    }
    else
        _transports.push_back(&udp);

    // Predefine a list of messages we receive from the transports.
    MessageList msglist;
    Message* msg;

    // Process messages.
    while(1)
    {
        // Wait 1 millisecond so we don't hog the CPU
        JrSleep(1);

        // Check the public socket for outgoing requests
        publicSocket.recvMsg(msglist);
        while (!msglist.empty())
        {
            // Get the first message from the list
            msg = msglist.front();
            msglist.pop_front();

            // Process the message request
            if (msg->getDestinationId().val == 0)
            {
                // This message was intended for the RTE, and therefore must
                // be a connection request.  Response appropriately.
                printf("Received connection request for %ld\n", msg->getSourceId().val);
                Message response;
                response.setSourceId(0);
                response.setDestinationId(msg->getSourceId());
                publicSocket.sendMsg(response);
                _clients.push_back(msg->getSourceId().val);
            }
            else 
            {
                // If the destination contains no wildcards, try to send it
                // as a point-to-point message.
                bool matchFound = false;
                if (!msg->getDestinationId().containsWildcards())
                {
                    // Send this message to the recipients on any transport.
                    for (_iter = _transports.begin(); _iter != _transports.end(); ++_iter)
                    {
                        Transport::TransportError result = (*_iter)->sendMsg(*msg);
                        if (result != Transport::AddrUnknown) matchFound = true;
                    }

                    // If we don't have an entry in our address book, broadcast this message.
                    // First set the ack/nak bit, though, so that if we do find the endpoint
                    // we can learn its address.
                    if (!matchFound) msg->setAckNakFlag(1);
                }

                // If the destination contains wildcards, or we didn't find
                // a match, broadcast it.
                if (msg->getDestinationId().containsWildcards() || !matchFound)
                {
                    // Send this message to all recipients on all transports.
                    for (_iter = _transports.begin(); _iter != _transports.end(); ++_iter)
                    {
                        (*_iter)->broadcastMsg(*msg);
                    }
                }
            }

            // Done with this message.
            delete msg;
        }

        // Now check receive messages on all other transports
        for (_iter = _transports.begin(); _iter != _transports.end(); ++_iter)
        {
            // Don't check the socket in this loop, since we already did it.
            if (_iter == _transports.begin()) continue;
            (*_iter)->recvMsg(msglist);
            while (!msglist.empty())
            {
                // Get the first message from the list
                msg = msglist.front();
                msglist.pop_front();

                // If this message is intended for a local client (and a 
                // local client only), send it only on the socket interface.
                if (std::find(_clients.begin(), _clients.end(), msg->getSourceId().val) !=
                    _clients.end())
                {
                    // Match found.  Send to the socket interface.
                    publicSocket.sendMsg(*msg);
                }
                else
                {
                    // This message is either not intended for us, or contains
                    // wildcard characters.  Send to all available interfaces.
                        
                    //printf("Received message from %ld to %ld\n", msg.getSourceId().val, msg.getDestinationId().val);
                    for ( std::list<Transport*>::iterator tport = _transports.begin(); 
                        tport != _transports.end(); ++tport)
                          (*tport)->sendMsg(*msg);
                }

                // Done processing this message
                delete msg;
            }
        }
    }
}

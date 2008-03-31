/*! 
 ***********************************************************************
 * @file      JuniorRTE.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <signal.h>
#include "ConfigData.h"
#include "JrSockets.h"
#include "JUDPTransport.h"
#include "Transport.h"
#include "Types.h"
#include "OS.h"

using namespace DeVivo::Junior;

// Convenient typedefs
typedef std::list<unsigned long>     ConnectionList;
typedef std::list<JAUS_ID>::iterator ConnectionListIter;

// Define a signal handler, so we can clean-up properly
static int exit_flag = 0;
static void handle_exit_signal( int signum )
{
    exit_flag = 1;
}

// Main loop
int main(int argc, char* argv[])
{
    printf("Hello, and welcome to the JuniorRTE\n");

    // For linux, we need to break from the parent's signals
#ifndef WINDOWS
    setsid();
#endif

    // Pull the config file from the command line arguments
    std::string config_file = "";
    if (argc >= 2)
    {
        printf("Using config file: %s\n", argv[1]);
        config_file = std::string(argv[1]);
    }

    // Parse the config file, looking for our settings
    ConfigData config;
    config.parseFile(config_file);
    unsigned char _allowRelay = 1;
    config.getValue("AllowRelay", _allowRelay);

    // Create the public socket that allows APIs to find us.
    JrSocket publicSocket(std::string("JuniorRTE"));
    if (publicSocket.initialize(config_file) != Transport::Ok)
    {
        printf("Unable to initialize internal socket.  Exiting with error...\n");
        exit(1);
    }

    // Catch the termination signals
    signal( SIGINT, handle_exit_signal );
    signal( SIGTERM, handle_exit_signal );
    signal( SIGABRT, handle_exit_signal );

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
    if (udp.initialize(config_file) != Transport::Ok)
    {
        printf("Unable to initialize UDP communications.\n");
    }
    else
        _transports.push_back(&udp);

    // Predefine a list of messages we receive from the transports.
    MessageList msglist;
    Message* msg;

    // Process messages.
    while(!exit_flag)
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
            if ((msg->getDestinationId().val == 0) && 
                (msg->getMessageCode() == Connect))
            {
                // Connection request from client.
                Message response;
                response.setSourceId(0);
                response.setDestinationId(msg->getSourceId());
                response.setMessageCode(Accept);
                publicSocket.sendMsg(response);
                _clients.push_back(msg->getSourceId().val);
            }
            else if ((msg->getDestinationId().val == 0) && 
                     (msg->getMessageCode() == Cancel))
            {
                // Disconnect client.
                _clients.remove(msg->getSourceId().val);
                publicSocket.removeDestination(msg->getSourceId());
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

                // If relay is off, or this message is intended for a local client (and a 
                // local client only), send it only on the socket interface.
                if (!_allowRelay || 
                    (std::find(_clients.begin(), _clients.end(), msg->getDestinationId().val) !=
                    _clients.end()))
                {
                    // Match found.  Send to the socket interface.
                    publicSocket.sendMsg(*msg);
                }
                else
                {
                    // This message is either not intended for us, or contains
                    // wildcard characters.  Send to all available interfaces.
                    for ( std::list<Transport*>::iterator tport = _transports.begin(); 
                        tport != _transports.end(); ++tport)
                          (*tport)->sendMsg(*msg);
                }

                // Done processing this message
                delete msg;
            }
        }
    }

    // Received termination signal
    printf("Shutting down Junior RTE...\n");
}

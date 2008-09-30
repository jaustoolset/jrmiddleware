/*! 
 ***********************************************************************
 * @file      JuniorRTE.cpp
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
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <signal.h>
#include "ConfigData.h"
#include "JrSockets.h"
#include "JUDPTransport.h"
#include "JTCPTransport.h"
#include "JSerial.h"
#include "Transport.h"
#include "Types.h"
#include "OS.h"
#include "JrLogger.h"

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
    // Pull the config file from the command line arguments
    std::string config_file = "";
    if (argc >= 2) config_file = std::string(argv[1]);

    // This is the main entry point for hte Junior Run-Time engine.
    // First thing we need to do is initialize the log, but we can't
    // do that until we read in the log file name from the configuration
    // file.  So we start with opening and parsing the config file...
    ConfigData config;
    config.parseFile(config_file);
    std::string logfile;
    config.getValue("LogFileName", logfile);
    unsigned char debug_level = 0;
    config.getValue("LogMsgLevel", debug_level);
    unsigned char allowRelay = 1;
    config.getValue("AllowRelay", allowRelay);
    unsigned char delay = 1;
    config.getValue("RTE_CycleTime", delay);
    char use_udp = 1;
    config.getValue("EnableUDPInterface", use_udp);
    char use_tcp = 0;
    config.getValue("EnableTCPInterface", use_tcp);
    char use_serial = 0;
    config.getValue("EnableSerialInterface", use_serial);
    char repeater_mode = 0;
    config.getValue("EnableRepeaterMode", repeater_mode);

    // Now set-up the data logger
    if (debug_level > (int) Logger::full) debug_level = (int) Logger::full;
    Logger::get()->setMsgLevel((enum Logger::LogMsgType) debug_level);
    if (!logfile.empty()) Logger::get()->openOutputFile(logfile);

    // We can finally output some proof-of-life info
    JrInfo << "Hello, and welcome to the JuniorRTE" << std::endl;
    JrInfo << "Using config file: " << config_file << std::endl;

    // For linux, we need to break from the parent's signals
#ifndef WINDOWS
    setsid();
#endif

    // Create the public socket that allows APIs to find us.
    JrSocket publicSocket(std::string("JuniorRTE"));
    if (publicSocket.initialize(config_file) != Transport::Ok)
    {
        JrError << "Unable to initialize internal socket.  Exiting ...\n";
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

    // Create the transports, but don't initialize them unless requested.
    JUDPTransport udp;
    JTCPTransport tcp;
    JSerial serial;

    // Add UDP, if selected
    if (use_udp)
    {
        if (udp.initialize(config_file) != Transport::Ok)
        {
            JrInfo << "Unable to initialize UDP communications.\n";
        }
        else
            _transports.push_back(&udp);
    }
    else
    {
        JrInfo << "UDP communication deactivated in configuration file\n";
    }

    // Add TCP, if selected
    if (use_tcp)
    {

        if (tcp.initialize(config_file) != Transport::Ok)
        {
            JrInfo << "Unable to initialize TCP communications.\n";
        }
        else
            _transports.push_back(&tcp);
    }
    else
    {
        JrInfo << "TCP communication deactivated in configuration file\n";
    }

    // Add Serial, if selected
    if (use_serial)
    {

        if (serial.initialize(config_file) != Transport::Ok)
        {
            JrInfo << "Unable to initialize serial communications.\n";
        }
        else
            _transports.push_back(&serial);
    }
    else
    {
        JrInfo << "Serial communication deactivated in configuration file\n";
    }


    // Predefine a list of messages we receive from the transports.
    MessageList msglist;
    Message* msg;

    // Process messages.
    while(!exit_flag)
    {
        // Wait a bit so we don't hog the CPU
        JrSleep(delay);

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
                    if (!matchFound) 
                    {
                        JrInfo << "Destination id (" << msg->getDestinationId().val <<
                            ") unknown.  Switching to broadcast.\n";
                        msg->setAckNakFlag(1);
                    }
                }

                // If the destination contains wildcards, or we didn't find
                // a match, broadcast it.
                if (msg->getDestinationId().containsWildcards() || !matchFound)
                {
                    // Send this message to all recipients on all transports.
                    for (_iter = _transports.begin(); _iter != _transports.end(); ++_iter)
                        (*_iter)->broadcastMsg(*msg);
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

                // In repeater mode, the Junior RTE will broadcast any incoming message
                // on all interfaces.  THIS MODE SHOULD BE USED WITH CAUTION!!!
                // If multiple junior instances are set to repeater mode, network traffic
                // will continuous bounce between them until the end of time.
                if (repeater_mode)
                {
                    JrDebug << "Repeating message from " << msg->getSourceId().val <<
                        " to " << msg->getDestinationId().val << " (seq " <<
                        msg->getSequenceNumber() << ")\n";

                    for ( std::list<Transport*>::iterator tport = _transports.begin(); 
                        tport != _transports.end(); ++tport)
                          (*tport)->broadcastMsg(*msg);
                }
                // If relay is off, or this message is intended for a local client (and a 
                // local client only), send it only on the socket interface.
                else if (!allowRelay || 
                    (std::find(_clients.begin(), _clients.end(), msg->getDestinationId().val) !=
                    _clients.end()))
                {
                    // Match found.  Send to the socket interface.
                    publicSocket.sendMsg(*msg);
                }
                // Otherwise, forward this message on all channels (unless it originated locally)
                else if (std::find(_clients.begin(), _clients.end(), msg->getSourceId().val) ==
                          _clients.end())
                {
                    JrDebug << "Trying to forward message from " << msg->getSourceId().val <<
                        " to " << msg->getDestinationId().val << " (seq " <<
                        msg->getSequenceNumber() << ")\n";
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
    JrInfo << "Shutting down Junior RTE...\n";
}

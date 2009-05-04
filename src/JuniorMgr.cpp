/*! 
 ***********************************************************************
 * @file      JuniorMgr.cpp
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
#include "JuniorMgr.h"
#include "ConfigData.h"
#include <sstream>
#include <algorithm>

using namespace DeVivo::Junior;

#define minint(a,b) ((a<b)?a:b)

JuniorMgr::JuniorMgr():
    _socket_ptr(NULL)
{
    // Initialize config data
    _message_counter = 1;
    _maxMsgHistory = 100;   // as a message count
    _oldMsgTimeout = 10;    // in seconds
    _detectDuplicates =1; 
    _max_retries = 3;
    _ack_timeout = 100; // in milliseconds
    _msg_count = 0;
    _max_msg_size = 4079;
}

JuniorMgr::~JuniorMgr()
{
    if (_socket_ptr)
    {
        Message cancel;
        cancel.setMessageCode(Cancel);
        cancel.setSourceId(_id);
        cancel.setDestinationId(0);
        _socket_ptr->sendMsg(cancel);
        delete(_socket_ptr);
    }
}

unsigned int JuniorMgr::umin(unsigned int x, unsigned int y)
{
    return (( x < y) ? x : y);
}

void JuniorMgr::sendAckMsg( Message* incoming ) 
{
    Message response;
    response.setMessageCode(incoming->getMessageCode());
    response.setSourceId(_id);
    response.setDestinationId(incoming->getSourceId());
    response.setSequenceNumber(incoming->getSequenceNumber());
    response.setAckNakFlag(3);
    response.setPriority(incoming->getPriority());
    _socket_ptr->sendMsg(response);
}

// Returns the number of messages waiting, either in a local buffer or
// in the queue
unsigned char JuniorMgr::pending()
{
    unsigned char count = _msg_count + _socket_ptr->messagesInQueue();
    return (count);
}


// Helper function to detect duplicate messages
bool JuniorMgr::isDuplicateMsg(Message* msg)
{
    // This checking can be configured off.
    if (!_detectDuplicates) return false;

    MsgIdListIter iter = _recentMsgs.begin();
    while (iter != _recentMsgs.end())
    {
        // Make sure this entry isn't old (in time)
        if ((unsigned long)((JrGetTimestamp() - iter->first)) > (_oldMsgTimeout*1000))
        {
            iter = _recentMsgs.erase(iter);
            continue;
        }

        // Check if this element is a match for the incoming message
        if (iter->second.first == msg->getSourceId() &&
            iter->second.second == msg->getSequenceNumber())
        {
            return true;
        }

        // Increment the iterator to check the next element
        iter++;
    }

    // Push this message into our recently received list, so we
    // can detect future duplicates.  Make sure our list doesn't get too big.
    _recentMsgs.push_back(std::make_pair(JrGetTimestamp(), std::make_pair(
        msg->getSourceId(), msg->getSequenceNumber())));
    if (_recentMsgs.size() > _maxMsgHistory) _recentMsgs.pop_front();
    return false;
}


// This code works, but is absolutely atrocious.
// I need to rework this for improved readability.
void JuniorMgr::checkLargeMsgBuffer()
{
    // This function checks the large message buffer
    // (a temporary holding cell while we wait for all the pieces
    // of a divided message) for completed transmissions.
    TimeStampedMsgListIter msgIter = _largeMsgBuffer.begin();
    while (msgIter != _largeMsgBuffer.end())
    {
        // If the message in the history buffer is too old, discard it.
        // This prevents us from filling the buffer with partial messages
        // that never found a mate.
        if ((unsigned long)(JrGetTimestamp() - msgIter->first) > (_oldMsgTimeout*1000))
        {
            // Discard the message and remove from the list
            delete (msgIter->second);
            msgIter = _largeMsgBuffer.erase(msgIter);
            continue;
        }

        // If this is the first message of a sequence, try to find the rest of 'em.
        if (msgIter->second->getDataControlFlag() == Message::FirstMsg)
        {
            // Search the message list for the next one in the sequence.
            unsigned short msgnum = msgIter->second->getSequenceNumber();
            int msgcount = 1;
            TimeStampedMsgListIter nextMsg;

            while (1)
            {
                nextMsg = searchMsgList(_largeMsgBuffer,
                                msgIter->second->getSourceId(), ++msgnum);
                msgcount++;

                // If we didn't find the next message in the sequence,
                // break from this interior "while" loop, returning to 
                // the "for" loop.
                if (nextMsg == _largeMsgBuffer.end())
                {
                    msgIter++;
                    break;
                }

                // If this message is not the last message in the sequence,
                // continue the interior "while" loop until we find a missing
                // message or the true end.
                if (nextMsg->second->getDataControlFlag() != Message::LastMsg) 
					continue;

                // Getting to this point means we know that all the messages
                // in a sequence are available.  Reconstruct the original message.
                for (int i=1; i < msgcount; i++)
                {
                    nextMsg = searchMsgList(_largeMsgBuffer,
                                msgIter->second->getSourceId(), 
                                msgIter->second->getSequenceNumber()+i);
                    msgIter->second->getPayload().append( nextMsg->second->getPayload() );
                    delete (nextMsg->second);
                    _largeMsgBuffer.erase(nextMsg);
                }

                // Now that we have a complete message, add it to the delivery buffer
                // and remove it from the unfinished message buffer.
				_buffers[minint(msgIter->second->getPriority(), JrMaxPriority)].
					push_back(msgIter->second);
                _msg_count++;

                // Last, we need to erase the first message in the set from 
                // the large message buffer.  Note that we don't delete
                // the actual message, since it still needs to be delivered to the app.
                msgIter = _largeMsgBuffer.erase(msgIter);
                break;
            }
        }
        else
        {
            // Try the next message in the buffer
            msgIter++;
        }
    }
}


bool JuniorMgr::addMsgToBuffer(Message* msg)
{
    // If the source of this message is the RTE, throw it away.
    // If this is an ack/nak response or a duplicate message, disregard it.
    if ((msg->getAckNakFlag() == 2) || (msg->getAckNakFlag() == 3) ||
        isDuplicateMsg(msg) || (msg->getSourceId().val == 0))
    {
        delete msg;
        return false;
    }

    // If this message is part of a larger set, add it to the
    // "waiting for a complete" message queue.
    if (msg->getDataControlFlag() != Message::None)
    {
        _largeMsgBuffer.push_back(std::make_pair(JrGetTimestamp(),msg));
        checkLargeMsgBuffer();

        // We need to manage the large message buffer, so we don't have messages
        // that never vanish.
        if (_largeMsgBuffer.size() > _maxMsgHistory)
        {
            TimeStampedMsg oldest = _largeMsgBuffer.front();
            delete oldest.second;
            _largeMsgBuffer.pop_front();
        }
    }
    // Otherwise, put the message in a priority-based buffer, being
    // careful to make sure that the priority is in range.
	else
	{
		_buffers[minint(msg->getPriority(), JrMaxPriority)].push_back(msg);
        _msg_count++;
    }

    return true;
}

JrErrorCode JuniorMgr::sendto( unsigned int destination, 
                       unsigned int size, 
                       const char* buffer,
                       int priority,
                       int flags,
                       MessageCode code)
{
    // Check for degenerate case
    if (destination == 0) return InvalidID;

    // Modify the priority to not exceed the 4 bit space
    if (priority > JrMaxPriority) priority = JrMaxPriority;

    // As per the Standard, ServiceConnection and ACK/NAK cannot
    // be set at the same time
    if ((flags & 0x03) == 0x03) return InvalidParams;

    // If the destination identifier contains wildcard characters,
    // we need to route the message as a broadcast instead of a unicast.
    JAUS_ID destId(destination);
    if (destId.containsWildcards())
    {
        // This is a broadcast, so make sure we turn off ack/nak, 
        // and we meet the size limit (broadcasts cannot be parsed into
        // multiple packets.
        flags &= 0xFFFFFFFE;
        if (size > _max_msg_size)
        {
            JrError << "Broadcast of buffers larger than MTU_Size is not supported\n";
            return Overflow;
        }
    }

    // If we're not trying to detect duplicate messages, we follow JAUS 5669, v1.
    // As a result, the sequence number must start at zero for large data sets.
    if ((size > _max_msg_size) && (!_detectDuplicates))
        _message_counter = 0;

    // We can never send more than 4079 bytes in a single
    // message, so break up large data sets.
    unsigned int bytes_sent = 0;
    do
    {
        // Create the message
        Message msg;
        msg.setDestinationId(destination);
        msg.setSourceId(_id);
        msg.setPriority(priority);
        msg.setMessageCode(code);
        if (flags & GuaranteeDelivery) msg.setAckNakFlag(1);
        if (flags & ServiceConnection) msg.setServiceConnection(1);
        if (flags & ExperimentalFlag) msg.setExperimental(1);
        msg.setSequenceNumber(_message_counter);
        _message_counter++;

        // Set the payload, being careful not to exceed
        // 4079 bytes on any individual message.
        unsigned int payload_size = umin(_max_msg_size, size - bytes_sent);
        msg.setPayload(payload_size, &buffer[bytes_sent]);

        // Fill in the data control flags, so the receiver
        // can piece together the original message if it was
        // broken up.
        if (payload_size < size)
        {
            if (bytes_sent == 0) 
				msg.setDataControlFlag(Message::FirstMsg);
            else if ((bytes_sent + payload_size) == size) 
				msg.setDataControlFlag(Message::LastMsg);
            else 
				msg.setDataControlFlag(Message::MiddleMsg);
        }

        // Send the message to the RTE for distribution
        _socket_ptr->sendMsg(msg);
        bytes_sent += payload_size;


        // TO DO : Pend here for ACK-NAK
        if (flags & GuaranteeDelivery)
        {
            // We need to wait for an acknowledgement.  We wait a configurable
            // period of time, resend a configurable number of times.
            MessageList msglist;
            unsigned long last_msg_time = JrGetTimestamp();
            int send_count = 0; bool acked = false;
            while (!acked)
            {
                // sleep a bit to free up the CPU
                JrSleep(1);

                // While waiting, we need to process other messages. 
                Transport::TransportError ret = _socket_ptr->recvMsg(msglist);
                while (!msglist.empty())
                {
                    Message* incoming = msglist.front();
                    msglist.pop_front();

                    // Found a message.  Form a response if ACK/NAK selected.
                    if (incoming->getAckNakFlag() == 1) sendAckMsg( incoming );

                    // Check if this is the acknowledgement we've been waiting for.
                    if ((incoming->getAckNakFlag() == 3) && 
                        (incoming->getSequenceNumber() == msg.getSequenceNumber()) )
                    {
                        delete incoming;
                        acked = true;
                    }
                    else
                    {
                        // Put the incoming message in the buffer
                        addMsgToBuffer(incoming);
                    }
                }

                // If we have to resend a message that is part of a large data
                // stream, the data control flags need to be updated.  This 
                // seems wonky to have to do, but it's part of JAUS.
				if (msg.getDataControlFlag() == Message::MiddleMsg) 
					msg.setDataControlFlag(Message::MiddleResentMsg);

                // See if it's time to resend the message (or timeout)
                if ((unsigned long)(JrGetTimestamp() - last_msg_time) > _ack_timeout)
                {
                    if (++send_count > _max_retries) return Timeout;
                    _socket_ptr->sendMsg(msg);
                    last_msg_time = JrGetTimestamp();
                }
            }
        }
    } while(bytes_sent < size);  // continue to loop until we've sent
                                 // the entire buffer.
    return Ok;
}


JrErrorCode JuniorMgr::recvfrom(unsigned int* sender,
                        unsigned int* bufsize,
                        char* buffer,
                        int* priority,
                        int* flags,
                        MessageCode* code)
{
        // Put the entire message in an archive, so we can 
    // Check the socket for incoming messages.  
    MessageList msglist;
    Transport::TransportError ret = _socket_ptr->recvMsg(msglist);

    // Process each message in the received list
    while (!msglist.empty())
    {
        // Extract the message from the list
        Message* msg = msglist.front();
        msglist.pop_front();

        // Found a message.  Form a response if ACK/NAK selected.
        if (msg->getAckNakFlag() == 1) sendAckMsg( msg );

        // Add this message to a priority buffer
        addMsgToBuffer(msg);
    }

    // Check each priority based buffer (highest first) looking for a message
    // to deliver.
    for (int j = JrMaxPriority; j >= 0; j--)
    {
        if (!_buffers[j].empty())
        {
            // Found a non-empty buffer.  Pop the message out and return the data.
            Message* value = _buffers[j].front();
            _buffers[j].pop_front();
            _msg_count--;

            // Extract the data fields
            if (sender != NULL) *sender = value->getSourceId().val;
            if (priority != NULL) *priority = value->getPriority();
            if (code != NULL) *code = value->getMessageCode();
            if (flags != NULL)
            {
                if (value->getServiceConnection()) *flags |= ServiceConnection;
                if (value->getExperimental()) *flags |= ExperimentalFlag;
            }
            unsigned int data_size; char* data_ptr;
            value->getPayload(data_size, data_ptr);

            // Make sure our buffer is big enough to hold the entire message
            JrErrorCode ret = Ok;
            if (*bufsize < data_size)
            {
                JrError << "Receive buffer too small (buffer size: " << *bufsize <<
                    ", data size: " << data_size << std::endl; 
                ret = Overflow;
            }
            else
                *bufsize = data_size;

            // Copy over the data to the user supplied buffer, 
            memcpy( buffer, data_ptr, *bufsize);
            delete value;
            return ret;
        }
    }

    // Getting to this point means we have no messages to return.
    return NoMessages;
}

JrErrorCode JuniorMgr::connect(unsigned int id,  std::string config_file)
{
    // Parse the config file & read logger settings
    ConfigData config;
    config.parseFile(config_file);
    std::string logfile;
    config.getValue(logfile, "LogFileName", "Log_Configuration");
    int debug_level = 3;
    config.getValue(debug_level, "LogMsgLevel", "Log_Configuration");
	int connection_timeout = 100;
	config.getValue(connection_timeout, "ConnectionTimeout", "API_Configuration");
	if (connection_timeout < 50) connection_timeout = 50;

    // Now set-up the data logger
    if (debug_level > (int) Logger::full) debug_level = (int) Logger::full;
    Logger::get()->setMsgLevel((enum Logger::LogMsgType) debug_level);
    if (!logfile.empty()) Logger::get()->openOutputFile(logfile);

    // Check for degenerate value
    if (id == 0) 
    {
        JrError << "Cannot connect clients with id = 0\n";
        return InvalidID;
    }

    // Make sure the ID doesn't contain any wildcards.
    JAUS_ID jausId(id);
    if (jausId.containsWildcards())
    {
        JrError << "Client ID may not contain wildcards (0xFF).  Returning error...\n";
        return InvalidID;
    }

    // Spawn the RTE.  Note that hte spawn process will 
    // ensure that we don't create a duplicate.
    JrSpawnProcess("JuniorRTE", config_file);
    JrSleep(2000);

    // The name of our local socket is the string form of our ID.
    std::stringstream name; name << id;

    // First open a socket with the given name.
    JrSocket* mySocket = new JrSocket(name.str());
    if (mySocket->initialize(config) != Transport::Ok)
    {
        JrError << "Failed to open a local socket.  Returning error...\n";
        delete mySocket;
        return InitFailed;
    }

    // We only send to the RTE, so we can explicitly connect
    mySocket->setDestination("JuniorRTE");

    // Send a connection request.  This will cause the RTE
    // to look for private traffic on a socket with the Identifier name.
    Message msg;
    msg.setSourceId(id);
    msg.setDestinationId(0);
    msg.setMessageCode(Connect);
    mySocket->sendMsg(msg);
    JrInfo << "Sending client connection request to Junior Run-Time Engine...\n";

    // Wait for a connection accept message
    MessageList msglist;
    bool connected = false;
    int counter = 0;
    while (!connected)
    {
        if (counter++ > connection_timeout)
        {
            // timeout.
            delete mySocket;
            JrError << "Timeout waiting for response from Run-Time Engine (Timeout=" <<
				connection_timeout << ")\n";
            return Timeout;
        }

        // Resend the connection request msg
        if ((counter % 25) == 0) mySocket->sendMsg(msg);

        // Check for incoming messages
        mySocket->recvMsg(msglist);
        while (!msglist.empty())
        {
            Message* response = msglist.front();
            msglist.pop_front();
            if (response->getSourceId().val == 0)
            {
                connected = true;
                JrInfo << "Client connection to RTE accepted and open...\n";
            }
            delete response;
        }
        JrSleep(1);
    }

    // Success.  Store values
    _socket_ptr = mySocket;
    _id.val     = id;

    // Initialize config data from file
    config.getValue(_maxMsgHistory, "MaxMsgHistory", "API_Configuration");
    config.getValue(_oldMsgTimeout, "OldMsgTimeout", "API_Configuration");
    config.getValue(_detectDuplicates, "DropDuplicateMsgs", "API_Configuration");
    config.getValue(_max_retries, "MaxAckNakRetries", "API_Configuration");
    config.getValue(_ack_timeout, "AckTimeout", "API_Configuration");
    config.getValue(_max_msg_size, "MTU_Size", "API_Configuration");
	if (_max_msg_size > 4079)
	{
		JrWarn << "MTU_Size cannot exceed 4079 bytes.  Setting MTU_Size=4079\n";
		_max_msg_size = 4079;
	}
    return Ok;
}

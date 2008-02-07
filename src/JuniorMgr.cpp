// This is the principle file for the realization of the Junior API.
// Note that the interface itself is completely function, but the "handle"
// represents an object that manages the connection to the Junior RTE.
#include "JuniorMgr.h"
#include <sstream>
#include <algorithm>

JuniorMgr::JuniorMgr()
{
}

JuniorMgr::~JuniorMgr()
{
}

unsigned int JuniorMgr::umin(unsigned int x, unsigned int y)
{
    return (( x < y) ? x : y);
}

void JuniorMgr::sendAckMsg( Message* incoming ) 
{
    Message response(incoming->getMessageCode());
    response.setSourceId(_id);
    response.setDestinationId(incoming->getSourceId());
    response.setSequenceNumber(incoming->getSequenceNumber());
    response.setAckNakFlag(3);
    response.setPriority(incoming->getPriority());
    _socket_ptr->sendMsg(response);
}


// This code works, but is absolutely atrocious.
// I need to rework this for improved readability.
void JuniorMgr::checkLargeMsgBuffer()
{
    // This function checks the large message buffer
    // (a temporary holding cell while we wait for all the pieces
    // of a divided message) for completed transmissions.
    MessageListIter msgIter = _largeMsgBuffer.begin();
    while (msgIter != _largeMsgBuffer.end())
    {
        // If this is the first message of a sequence, try to find the rest of 'em.
        if ((*msgIter)->getDataControlFlag() == 1)
        {
            // Search the message list for the next one in the sequence.
            unsigned short msgnum = (*msgIter)->getSequenceNumber();
            int msgcount = 1;
            MessageListIter nextMsg;

            while (1)
            {
                nextMsg = searchMsgList(_largeMsgBuffer,
                                (*msgIter)->getSourceId(), ++msgnum);
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
                if ((*nextMsg)->getDataControlFlag() != 8) continue;
//printf("Reconstructing message.  Original seq num = %ld (size=%ld, contol=%ld)\n", 
//       (*msgIter)->getSequenceNumber(), (*msgIter)->getPayload().getArchiveLength(),
//       (*msgIter)->getDataControlFlag());

                // Getting to this point means we know that all the messages
                // in a sequence are available.  Reconstruct the original message.
                for (int i=1; i < msgcount; i++)
                {
                    nextMsg = searchMsgList(_largeMsgBuffer,
                                (*msgIter)->getSourceId(), (*msgIter)->getSequenceNumber()+i);
                    (*msgIter)->getPayload().append( (*nextMsg)->getPayload() );
//printf("Adding message.  Next seq num = %ld (size=%ld, flags=%ld)\n", 
//       (*nextMsg)->getSequenceNumber(), (*nextMsg)->getPayload().getArchiveLength(), (*nextMsg)->getDataControlFlag());

                    delete (*nextMsg);
                    _largeMsgBuffer.erase(nextMsg);
                }
//                printf("Total message size: %ld\n", (*msgIter)->getPayload().getArchiveLength());

                // Now that we have a complete message, add it to the delivery buffer
                // and remove it from the unfinished message buffer.
                if ((*msgIter)->getPriority() > JrMaxPriority)
                {
                    _buffers[JrMaxPriority].push_back((*msgIter));
                }
                else
                {
                    _buffers[(*msgIter)->getPriority()].push_back((*msgIter));
                }

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
    // Before adding to the buffer, make sure it isn't a duplicate
    // of one we just processed.
    if (std::find(_recentMsgs.begin(), _recentMsgs.end(),
        std::make_pair(msg->getSourceId(), msg->getSequenceNumber())) !=
        _recentMsgs.end())
    {
        // Looks like this is a duplicate of one we've already received.
        // Free the message and return.
        delete msg;
        return false;
    }

    // Push this message into our recently received list, so we
    // can detect duplicates.  Make sure our list doesn't get too big.
    _recentMsgs.push_back(std::make_pair(
        msg->getSourceId(), msg->getSequenceNumber()));
    if (_recentMsgs.size() > MsgHistory) _recentMsgs.pop_front();

    // If this message is part of a larger set, add it to the
    // "waiting for a complete" message queue.
    if (msg->getDataControlFlag() != 0)
    {
        _largeMsgBuffer.push_back(msg);
        checkLargeMsgBuffer();
    }
    // Otherwise, put the message in a priority-based buffer, being
    // careful to make sure that the priority is in range.
    else if (msg->getPriority() > JrMaxPriority)
    {
        _buffers[JrMaxPriority].push_back(msg);
    }
    else
    {
        _buffers[msg->getPriority()].push_back(msg);
    }

    return true;
}

int JuniorMgr::sendto( unsigned long destination, 
                       unsigned int size, 
                       const char* buffer,
                       int priority,
                       int flags)
{
    // We can never send more than 4079 bytes in a single
    // message, so break up large data sets.
    unsigned int bytes_sent = 0;
    do
    {
        // Create the message
        Message msg(P2P_Message);
        msg.setDestinationId(destination);
        msg.setSourceId(_id);
        msg.setPriority(priority);
        if (flags & 0x01) msg.setAckNakFlag(1);
        msg.setSequenceNumber(++_message_counter);

        // Set the payload, being careful not to exceed
        // 4079 bytes on any individual message.
        unsigned int payload_size = umin(4079, size - bytes_sent);
        msg.setPayload(payload_size, &buffer[bytes_sent]);

        // Fill in the data control flags, so the receiver
        // can piece together the original message if it was
        // broken up.
        if (payload_size < size)
        {
            if (bytes_sent == 0) msg.setDataControlFlag(1);
            else if ((bytes_sent + payload_size) == size) msg.setDataControlFlag(8);
            else msg.setDataControlFlag(2);
        }

        // Send the message to the RTE for distribution
        _socket_ptr->sendMsg(msg);
        bytes_sent += payload_size;


        // TO DO : Pend here for ACK-NAK
        if (flags & 0x01)
        {
            // We need to wait for an acknowledgement.  Note that we wait a maximum of
            // 150 milliseconds, and retransmit the original message every 50 milliseconds.
            // While waiting, we need to process other messages.
            Message* incoming = new Message(0);
            int counter;
            for (counter = 1; counter < 400; counter++)
            {
                JrSleep(1);
                Transport::TransportError ret = _socket_ptr->recvMsg(*incoming);
                if (ret == Transport::Ok)
                {
                    // Found a message.  Form a response if ACK/NAK selected.
                    if (incoming->getAckNakFlag() == 1) sendAckMsg( incoming );

                    // Check if this is an acknowledgement
                    if ((incoming->getAckNakFlag() == 3) && 
                        (incoming->getSequenceNumber() == msg.getSequenceNumber()))
                    {
                        delete incoming;
                        break;
                    }

                    // Put the incoming message in the buffer
                    addMsgToBuffer(incoming);
     
                    // Since we had a message, allocate buffer for a new one
                    incoming = new Message(0);
                }

                // If we have to resend a message that is part of a large data
                // stream, the data control flags need to be updated.  This 
                // seems wonky to have to do, but it's part of JAUS.
                if (msg.getDataControlFlag() == 2) msg.setDataControlFlag(4);

                // Every 100 milliseconds, resend the message.  At the 200 millisecond
                // mark, use a broadcast on the hopes that we can find the destination.
                if ((counter % 200) == 0) msg.setMessageCode(BroadcastMsg);
                if ((counter % 100) == 0) _socket_ptr->sendMsg(msg);
            }
            
            // If we didn't successfully receive a acknowledgement, we
            // return an error.
            if (counter == 400) return Transport::Failed;
        }
        //printf("Looping with bytes_sent = %ld, size = %ld\n", bytes_sent, size);
    } while(bytes_sent < size);  // continue to loop until we've sent
                                 // the entire buffer.
    return Transport::Ok;
}

int JuniorMgr::broadcast( unsigned int bufsize,
                          const char* buffer,
                          int priority )
{
    // Make sure we're within size limits.  Broadcasting of large datasets is
    // not permitted.
    if (bufsize > 4079)
    {
        printf("Broadcast of buffers larger than 4079 bytes is not supported\n");
        return Transport::Failed;
    }

    // Create the message
    Message msg(BroadcastMsg);
    msg.setSourceId(_id);
    msg.setDestinationId(0xFFFFFFFF);
    msg.setPriority(priority);
    msg.setAckNakFlag(0);
    msg.setSequenceNumber(++_message_counter);
    msg.setPayload(bufsize, buffer);

    // Send the message to the RTE for distribution
    _socket_ptr->broadcastMsg(msg);
    return 0;
}

int JuniorMgr::recvfrom(unsigned long* sender,
                        unsigned int bufsize,
                        char* buffer,
                        int* priority)
{
    // Check the socket for incoming messages.  Process up to 5, so
    // we don't spin too long in this function.
    for (int i=0; i < 10; i++)
    {
        Message* msg = new Message(0);
        Transport::TransportError ret = _socket_ptr->recvMsg(*msg);
        if (ret != Transport::Ok)
        {
            // No messages.  Break the loop early.  We also
            // have to free the buffer we pre-allocated.
            delete msg;
            break;
        }

        // Found a message.  Form a response if ACK/NAK selected.
        if (msg->getAckNakFlag() == 1) sendAckMsg( msg );

        // If this is an extra acknowledgement, disregard it.  Otherwise,
        // put it in the priority based buffer.
        if ((msg->getAckNakFlag() != 2) && msg->getAckNakFlag() != 3)
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

            // Extract the data fields
            if (sender != NULL) *sender = value->getSourceId().val;
            if (priority != NULL) *priority = value->getPriority();
            unsigned int data_size; char* data_ptr;
            value->getPayload(data_size, data_ptr);

            // Copy over the data to the user supplied buffer, 
            // making sure not to copy more than the given size.
            if (bufsize < data_size) printf("RECV: Buffer too small (buf=%ld, data=%ld)\n", bufsize, data_size);
            unsigned int copy_size = umin(bufsize, data_size);
            memcpy( buffer, data_ptr, copy_size);
            delete value;
            return copy_size;
        }
    }

    // Getting to this point means we have no messages to return.
    return 0;
}

int JuniorMgr::connect(unsigned long id)
{
    // The name of our local socket is the string form of our ID.
    std::stringstream name; name << id;

    // First open a socket with the given name.
    JrSocket* mySocket = new JrSocket;
    if (mySocket->initialize(name.str()) != Transport::Ok)
    {
        printf("Failed to open a local socket.  Return error...\n");
        delete mySocket;
        return -1;
    }

    // We only send to the RTE, so we can explicitly connect
    mySocket->setDestination("JuniorRTE");

    // Send a connection request.  This will cause the RTE
    // to look for private traffic on a socket with the Identifier name.
    Message msg(RequestConnection);
    msg.setSourceId(id);
    mySocket->sendMsg(msg);
    printf("API: Sending connection request to RTE...\n");

    // Wait for a connection accept message
    Message response(0);
    while (1)
    {
        if (mySocket->recvMsg(response) == Transport::Ok) 
        {
            if (response.getMessageCode() == AcceptConnection)
                printf("API: Connection to RTE accepted and open...\n");
            break;
        }
        JrSleep(1);
    }

    // Success
    _socket_ptr = mySocket;
    _id.val     = id;
    return Ok;
}

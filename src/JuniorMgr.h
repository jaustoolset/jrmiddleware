/*! 
 ***********************************************************************
 * @file      JuniorMgr.h
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
#include "JrSockets.h"
#include "JuniorAPI.h"
#include "OS.h"
#include <stdio.h>
#include <stdlib.h>

namespace DeVivo {
namespace Junior {

typedef std::pair<JAUS_ID, unsigned short> MsgId;
typedef std::pair<unsigned long, MsgId> TimeStampedMsgId;
typedef std::list<TimeStampedMsgId> MsgIdList;
typedef std::list<TimeStampedMsgId>::iterator MsgIdListIter;
const int JrMaxPriority = 15;

class JuniorMgr
{
public:

    JuniorMgr();
    ~JuniorMgr();

    // The public functions mirror the API equivalents.
    JrErrorCode sendto( unsigned long destination, unsigned int size, 
                const char* buffer, int priority, int flags, MessageCode code = 0);

    JrErrorCode recvfrom( unsigned long* sender, unsigned int* bufsize,
                  char* buffer, int* priority, int* flags, MessageCode* code = NULL);
    
    JrErrorCode connect(unsigned long id, std::string config_file);

    unsigned char pending( );

private:
 
    // Define a couple of private helper functions
    unsigned int umin(unsigned int x, unsigned int y);
    void sendAckMsg(Message* source);
    bool addMsgToBuffer(Message* msg);
    void checkLargeMsgBuffer();
    bool isDuplicateMsg(Message* msg);
    TimeStampedMsgListIter searchMsgList(TimeStampedMsgList& list, 
                                         JAUS_ID sender, 
                                         unsigned short seqnum);

    // Private data.
    MessageList        _buffers[JrMaxPriority+1];
    TimeStampedMsgList _largeMsgBuffer;
    JAUS_ID            _id;
    JrSocket*          _socket_ptr;
    unsigned short     _message_counter;
    MsgIdList          _recentMsgs;
    unsigned int       _max_retries;
    unsigned int       _ack_timeout;
    unsigned int       _msg_count;
    unsigned int       _max_msg_size;

    // Configuration data
    unsigned short _maxMsgHistory;      // as a message count
    unsigned short _oldMsgTimeout;      // in seconds
    unsigned char  _detectDuplicates; 
};

inline TimeStampedMsgListIter JuniorMgr::searchMsgList(
                                         TimeStampedMsgList& list, 
                                         JAUS_ID sender, 
                                         unsigned short seqnum)
{
    for (TimeStampedMsgListIter iter = list.begin();
         iter != list.end(); iter++)
    {
        if ((iter->second->getSourceId() == sender) &&
            (iter->second->getSequenceNumber() == seqnum))
            return iter;
    }
    return list.end();
}

}} // namespace DeVivo::Junior

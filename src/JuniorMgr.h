/*! 
 ***********************************************************************
 * @file      JuniorMgr.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "JrSockets.h"
#include "JuniorAPI.h"
#include "Message.h"
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

    JrErrorCode sendto( unsigned int size, const char* buffer);

    JrErrorCode recvfrom( unsigned long* sender, unsigned int* bufsize,
                  char* buffer, int* priority, int* flags, MessageCode* code = NULL);
    
    JrErrorCode recvfrom( unsigned int* bufsize, char* buffer );

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
    unsigned char      _max_retries;
    unsigned char      _ack_timeout;
    unsigned int       _msg_count;

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

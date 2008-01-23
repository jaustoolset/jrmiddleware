// This is the principle file for the realization of the Junior API.
// Note that the interface itself is completely function, but the "handle"
// represents an object that manages the connection to the Junior RTE.
#include "JrSockets.h"
#include "JuniorAPI.h"
#include "Message.h"
#include "OS.h"
#include <stdio.h>
#include <stdlib.h>

typedef std::pair<JAUS_ID, unsigned short> MsgId;
typedef std::list<Message*> MessageList;
typedef std::list<Message*>::iterator MessageListIter;
const unsigned short MsgHistory = 50;

class JuniorMgr
{
public:

    JuniorMgr();
    ~JuniorMgr();

    // The public functions mirror the API equivalents.
    int sendto( unsigned long destination, unsigned int size, 
                const char* buffer, int priority, int flags);

    int broadcast( unsigned int bufsize, const char* buffer, int priority);

    int recvfrom( unsigned long* sender, unsigned int bufsize,
                  char* buffer, int* priority);

    int connect(unsigned long id);

private:

    // Define a couple of private helper functions
    unsigned int umin(unsigned int x, unsigned int y);
    void sendAckMsg(Message* source);
    bool addMsgToBuffer(Message* msg);
    void checkLargeMsgBuffer();
    MessageListIter searchMsgList(MessageList& list, JAUS_ID sender, unsigned short seqnum);

    // Private data.
    MessageList       _buffers[JrMaxPriority+1];
    MessageList       _largeMsgBuffer;
    JAUS_ID           _id;
    JrSocket*         _socket_ptr;
    unsigned short    _message_counter;
    std::list<MsgId>  _recentMsgs;
};

inline MessageListIter JuniorMgr::searchMsgList(MessageList& list, 
                                         JAUS_ID sender, 
                                         unsigned short seqnum)
{
    for (MessageListIter iter = list.begin();
         iter != list.end(); iter++)
    {
        if (((*iter)->getSourceId() == sender) &&
            ((*iter)->getSequenceNumber() == seqnum))
            return iter;
    }
    return list.end();
}

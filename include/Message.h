//  Message class - an abstract class on which all messages are based
#ifndef  __MESSAGE_H
#define  __MESSAGE_H


#include "Archive.h"
#include "Types.h"

// Define a Message Code
typedef unsigned short MessageCode;

class Message;
typedef std::list<Message*> MessageList;
typedef std::list<Message*>::iterator MessageListIter;
typedef std::pair<unsigned long, Message*> TimeStampedMsg;
typedef std::list<TimeStampedMsg> TimeStampedMsgList;
typedef std::list<TimeStampedMsg>::iterator TimeStampedMsgListIter;


class Message
{
public:
    Message():
            _code(0),_source(0),_destination(0), _priority(6), 
            _acknak(0), _control(0), _payload(){}
   ~Message(){}

    //
    // Functions to get/set the message code
    //
    MessageCode getMessageCode(){return _code;}
    void setMessageCode(MessageCode code){_code = code;}

    //
    // Functions to set the source and destination numeric id for the message
    //
    void setSourceId(JAUS_ID source){_source=source;}
    JAUS_ID getSourceId(){return _source;}

    void setDestinationId(JAUS_ID destination){_destination=destination;}
    JAUS_ID getDestinationId(){return _destination;}

    //
    // Functions for priority handling
    //
    void setPriority(unsigned char prio){_priority=prio;}
    unsigned char getPriority(){return _priority;}

    //
    // Functions for ack/nak
    //
    void setAckNakFlag(char flag){_acknak=flag;}
    char getAckNakFlag(){return _acknak;}

    //
    // Functions for data control (large message handling)
    //
    void setDataControlFlag(char flag){_control=flag;}
    char getDataControlFlag(){return _control;}

    //
    // Functions for sequence number
    //
    void setSequenceNumber(unsigned short seq){_sequence = seq;}
    unsigned short getSequenceNumber(){return _sequence;}

    //
    // Functions for handling the payload
    //
    void setPayload(unsigned int size, const char* data);
    void getPayload(unsigned int& size, char*& data);
    Archive& getPayload(){return _payload;}

    //
    // Functions for encoding/decoding the message into an archive.
    //
    virtual void pack(Archive& packed_msg);
    virtual void unpack(Archive& packed_msg);

    //
    // Returns the packed length of a message (including headers)
    // or packed length of data (without headers)
    //
    unsigned short getMsgLength();
    unsigned short getDataLength();

protected:

    MessageCode    _code;
    JAUS_ID        _source;
    JAUS_ID        _destination;
    unsigned char  _priority;
    char           _acknak;
    char           _control;
    unsigned short _sequence;
    Archive        _payload;

    //
    // Internal function to (un-)pack a JAUS header with standard parameters
    //
    virtual void packHdr(Archive& packed_msg);
    virtual void unpackHdr(Archive& packed_msg);
};

//
// Inline function defintions.
// 
inline void Message::pack(Archive& packed_msg)
{
    // Default implementation packs the header, then the data.
    packHdr( packed_msg );
    packed_msg.append( _payload );
}

inline void Message::unpack(Archive& packed_msg)
{
    // Default implementation unpacks the header, then the data.
    unpackHdr( packed_msg );
    packed_msg>>_payload;
}

inline unsigned short Message::getDataLength()
{
    // Return the length of the payload
    return _payload.getArchiveLength();
}

inline unsigned short Message::getMsgLength()
{
    // The length of the a packed message is simply the
    // length of packed data plus 16 byte header.
    return (getDataLength() + 16);
}

inline void Message::packHdr( Archive& packed_msg )
{
    // Pack header as little endian
    packed_msg.setPackMode( Archive::LittleEndian );

    // Priority and acknowledgement
    packed_msg << (char) (_priority | (_acknak << 4));

    // Default version is 3.2 (value 2)
    packed_msg << (char) 2;

    // Next 2 bytes are the command code
    packed_msg << (unsigned short) _code;

    // Next come the destination and source addresses.
    packed_msg << _destination.val;
    packed_msg << _source.val;

    // The data size and data flags represent the next 2 bytes.
    // Data control flags are the highest 4 bits.
    packed_msg << ((unsigned short)(getDataLength() | (_control<<12)));

    // Sequence number is zero for single messages
    packed_msg << _sequence;
}

inline void Message::unpackHdr( Archive& packed_msg )
{
    // Unpack header as little endian
    packed_msg.setPackMode( Archive::LittleEndian );

    // Rewind to the start of the archive
    packed_msg.rewind();

    // Pull the priority and ack/nak bits
    char delivery_options;
    packed_msg >> delivery_options; // message priority
    _priority = delivery_options & 0x0F;
    _acknak   = ((delivery_options & 0x30) >> 4);

    // Read (and discard) fields we don't care about...
    char dummy;
    packed_msg >> dummy; // version

    // Next 2 bytes are the command code
    packed_msg >> _code;

    // Next come the destination and source addresses.
    packed_msg >> _destination.val;
    packed_msg >> _source.val;

    // The data size and data flags represent the next 2 bytes.
    unsigned short temp;
    packed_msg >> temp; // 
    _control = ((temp >> 12) & 0xF);

    // Sequence number is also two bytes
    packed_msg >> _sequence;
}

inline void Message::setPayload(unsigned int size, const char* data)
{
    // Copy the given data to the internal Archive
    _payload.setData( data, size );
}

inline void Message::getPayload(unsigned int& size, char*& data)
{
    // Return a pointer to the data buffer, and its size
    size = _payload.getArchiveLength();
    data = _payload.getArchive();
}


#endif



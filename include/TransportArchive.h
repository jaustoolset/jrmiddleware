/*! 
 ***********************************************************************
 * @file      TransportArchive.h
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

#ifndef  __TRANSPORT_ARCHIVE_H
#define  __TRANSPORT_ARCHIVE_H

#include "Archive.h"
#include "JrLogger.h"

namespace DeVivo {
namespace Junior {

//
// A Transport Archive is a specialty archive that includes
// on-the-wire header bits.
//
class TransportArchive : public Archive
{
  public:
    TransportArchive(){}
    ~TransportArchive(){}

	// A transport archive converts a message into a byte stream
	// by serializing or "packing".  
	virtual bool pack(Message& msg, MsgVersion version) = 0;
	virtual bool unpack(Message& msg) = 0;

	// Helper functions for common operations
    virtual bool isArchiveValid() = 0;
	virtual void removeHeadMsg() = 0;

protected:

	// common functions to pack/unpack the headers/footers
	// across all transport types
	void packHdr(Message& msg, MsgVersion version = AS5669);
	void unpackHdr(Message& msg, MsgVersion version = AS5669);
	void packFtr(Message& msg, MsgVersion version = AS5669);
	void unpackFtr(Message& msg, MsgVersion version = AS5669);

};

inline void TransportArchive::packHdr(Message& msg, MsgVersion version)
{
    // Pack header as little endian
    setPackMode( Archive::LittleEndian );

	// Packed structure depends on the Version
	if (version == AS5669A)
	{
		*this << (char) 0; // Message type and header compression
		*this << (unsigned short) (msg.getDataLength()+14); // data & header
		*this << (char) ((msg.getScaledPriority() << 6) | (msg.getBroadcast() << 4) | 
			     (msg.getAckNakFlag() << 2) | msg.getDataControlFlagAsChar(version));
		*this << (unsigned long) msg.getDestinationId().val;  // destination
		*this << (unsigned long) msg.getSourceId().val;		// source
	}
	else
	{
		// Priority and acknowledgement, service connection, experimental
		*this << ((char)(msg.getPriority() | (msg.getAckNakFlag() << 4) | 
			          (msg.getServiceConnection() << 6) | (msg.getExperimental() << 7)));
		*this << (char) 2; // version
		*this << (unsigned short) msg.getMessageCode(); // command code
		*this << (unsigned long) msg.getDestinationId().val; // destination
		*this << (unsigned long) msg.getSourceId().val;      // source
		*this << ((unsigned short)(msg.getDataLength() | 
					(msg.getDataControlFlagAsChar(version)<<12)));
		*this << (unsigned short) msg.getSequenceNumber();  // sequence number
	}
}

inline void TransportArchive::packFtr( Message& msg, MsgVersion version )
{
	// Pack as little endian
    setPackMode( Archive::LittleEndian );

	// Footers are only defined for 5669 rev A
	if (version == AS5669A) 
		*this << (unsigned short) msg.getSequenceNumber();
}

inline void TransportArchive::unpackHdr( Message& msg, MsgVersion version )
{
	char temp8; unsigned short temp16;
	unsigned long temp32;

    // Unpack header as little endian
    setPackMode( Archive::LittleEndian );

	// Structure depends on the Version
	if (version == AS5669A)
	{
		*this >> temp8; // message type
		bool hc_present = (temp8 & 0x3);
		*this >> temp16; // data size
		if (hc_present) *this >> temp16;
		*this >> temp8; // message properties
		msg.setScaledPriority((temp8 & 0xC0) >> 6);
		msg.setBroadcast((temp8 & 0x30) >> 4);
		msg.setAckNakFlag((temp8 & 0x0C) >> 2); 
		msg.setDataControlFlagAsChar(temp8 & 0x03, version);
		*this >> temp32; msg.setDestinationId(JAUS_ID(temp32)); // destination
		*this >> temp32; msg.setSourceId(JAUS_ID(temp32)); // source
	}
	else
	{
		*this >> temp8; // message properties
		msg.setPriority(temp8 & 0x0F);
		msg.setAckNakFlag((temp8 & 0x30) >> 4); 
		msg.setServiceConnection((temp8 & 0x40) >> 6);
		msg.setExperimental((temp8 & 0x80) >> 7);
		*this >> temp8; // discard version
		*this >> temp16; msg.setMessageCode(temp16);// command code
		*this >> temp32; msg.setDestinationId(JAUS_ID(temp32)); // destination
		*this >> temp32; msg.setSourceId(JAUS_ID(temp32)); // source
		*this >> temp16; // data control flags
		msg.setDataControlFlagAsChar((temp16 >> 12) & 0xF, version);
		*this >> temp16; msg.setSequenceNumber(temp16);
	}
}

inline void TransportArchive::unpackFtr( Message& msg, MsgVersion version )
{
	// Pack as little endian
    setPackMode( Archive::LittleEndian );

	// Footers are only defined for 5669 rev A
	if (version == AS5669A) 
	{
		unsigned short temp16;
		*this >> temp16; msg.setSequenceNumber(temp16);
	}
}
}} // namespace DeVivo::Junior



#endif

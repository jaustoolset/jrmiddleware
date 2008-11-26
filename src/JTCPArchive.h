/*! 
 ***********************************************************************
 * @file      JTCPArchive.h
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

#ifndef  __JTCP_ARCHIVE_H
#define  __JTCP_ARCHIVE_H

#include "TransportArchive.h"

namespace DeVivo {
namespace Junior {

//
// A Socket Archive is a specialty archive that includes
// on-the-wire header bits for IPC comms
//
class JTCPArchive : public TransportArchive
{
  public:
    JTCPArchive(){}
    ~JTCPArchive(){}

	// Primary function is to pack/unpack messages
	bool pack(Message& msg) { return pack(msg, AS5669);}
	bool pack(Message& msg, MsgVersion version = AS5669);
	bool unpack(Message& msg);

	// Helper functions to read data directly from archive
    bool isArchiveValid();
	void removeHeadMsg();
	MsgVersion getVersion();

	// TCP is special in that we only send
	// the version byte once.  Clients need to be
	// able to remove it from archives before sending.
	void removeVersion(){removeAt(0,1);}

  protected:
	int getHeaderSize();
	int getFooterSize();
    unsigned short getDataLength();
	char* getDataPtr();
};

inline MsgVersion JTCPArchive::getVersion()
{
	if (getArchiveLength() < 1) return UnknownVersion;
	char version; getValueAt(0,version);
	if (version == 1) return AS5669;
	if (version == 2) return AS5669A;

	// catch all...
	return UnknownVersion;
}

inline int JTCPArchive::getHeaderSize()
{
	if (getVersion() == UnknownVersion) return 0; //failure
	if (getVersion() == AS5669) return 19; // header size is fixed

	// For AS5669 revision A, the presence or absence of the
	// header compression fields changes the size of the header.
	// Extract the flags.  Return 2 if flags are non-zero.
	char hc_flags; getValueAt(1, hc_flags); hc_flags &= 0x03;
	return (13 + (hc_flags ? 2 : 0));
}

inline int JTCPArchive::getFooterSize()
{
	if (getVersion() == AS5669A) return 2;
	return 0;
}

inline unsigned short JTCPArchive::getDataLength()
{
	unsigned short length = -1; 

	// Make sure we've got enough to read
	if (getArchiveLength() < getHeaderSize()) return -1;
	setPackMode( Archive::LittleEndian );

	// Location of the data size field depends on the header
	if (getVersion() == AS5669)
	{
		getValueAt(15, length);
		length &= 0x0FFF;
	}
	else if (getVersion() == AS5669A)
	{
		getValueAt(2, length);
		length -= (getHeaderSize()+1);
	}

	// return success
	return (length);
}

inline char* JTCPArchive::getDataPtr()
{
	// make sure the archive is valid
	if (!isArchiveValid()) return NULL;

	// Finding the payload depends on the header size
	return &data[getHeaderSize()];
}

inline bool JTCPArchive::isArchiveValid()
{
	// Make sure we have a proper version
	if (getVersion() == UnknownVersion) return false;

	// make sure we've got at least a complete header
	if (getArchiveLength() < getHeaderSize()) return false;

	// Pull data size from archive and make sure we exceed that
	if (getArchiveLength() < (getDataLength()+getHeaderSize()+getFooterSize())) 
		return false;

	// getting here implies success
	return true;
}

inline void JTCPArchive::removeHeadMsg()
{
	// keep the transport specific header
	int length = getHeaderSize()+getDataLength()+getFooterSize();
	if (length > getArchiveLength()) clear(); // error catching
	else removeAt(1, length - 1); // keep the version byte
}

inline bool JTCPArchive::pack(Message& msg, MsgVersion version)
{
	// Clear any existing data and resize for new message, with header.
	clear(); growBuffer( msg.getDataLength()+getHeaderSize()+getFooterSize());
	setPackMode( Archive::LittleEndian );

	// header depends on the version we're packing for
	if (version == AS5669)
	{
		*this << (char) 1; // version
		setPackMode( Archive::BigEndian );
		*this << (unsigned short) (msg.getDataLength() + 16);
		setPackMode( Archive::LittleEndian );
	}
	else
	{
		*this << (char) 2; //version
	}

	// pack common header, then message body, the common footer
	TransportArchive::packHdr(msg, version);
	append(msg.getPayload());
	TransportArchive::packFtr(msg, version);
	return true;
}

inline bool JTCPArchive::unpack(Message& msg)
{
	char temp8; unsigned short temp16;

	// check for a good archive before unpacking
	if (!isArchiveValid()) return false;

	// set the unpack mode and rewind to the start
	rewind(); setPackMode( Archive::LittleEndian );

	// pull JTCP header data depending on version
	Archive::offset = ((getVersion() == AS5669) ? 3 : 1);

	// unpack common header, message body, and common footer
	unpackHdr(msg, getVersion());
	msg.setPayload( getDataLength(), getDataPtr() );
	Archive::offset += getDataLength();
	unpackFtr(msg, getVersion());
	return true;
}

}} // namespace DeVivo::Junior

#endif
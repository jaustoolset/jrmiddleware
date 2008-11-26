/*! 
 ***********************************************************************
 * @file      SocketArchive.h
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

#ifndef  __SOCKET_ARCHIVE_H
#define  __SOCKET_ARCHIVE_H

#include "TransportArchive.h"

namespace DeVivo {
namespace Junior {

//
// A Socket Archive is a specialty archive that includes
// on-the-wire header bits for IPC comms
//
class SocketArchive : public TransportArchive
{
  public:
    SocketArchive(){}
    ~SocketArchive(){}

	// functions we must define
	bool pack(Message& msg);
	bool unpack(Message& msg);
    bool isArchiveValid();
	void removeHeadMsg(){};

  protected:
	// functions defined for convenience
    unsigned short getDataLength();
	char* getDataPtr();
};

inline unsigned short SocketArchive::getDataLength()
{
	// data size (without headers/footers)
	setPackMode( Archive::LittleEndian );
	unsigned short length;
	getValueAt(12, length); // warning! includes data control flags
	return (length & 0x0FFF); // kill the top 4 bits before returning
}

inline char* SocketArchive::getDataPtr()
{
	// make sure the archive is valid
	if (!isArchiveValid()) return NULL;
	return &data[16];
}

inline bool SocketArchive::isArchiveValid()
{
	// Size must be at least 16 bytes
	if (getArchiveLength() < 16) return false;

	// Pull data size from archive and make sure we exceed that
	if (getArchiveLength() < (getDataLength()+16)) return false;

	// getting here implies success
	return true;
}

inline bool SocketArchive::pack(Message& msg)
{
	// pack header, body, footer
	packHdr(msg, AS5669);
	append(msg.getPayload());
	packFtr(msg, AS5669);
	return true;
}

inline bool SocketArchive::unpack(Message& msg)
{
	// check for a good archive before unpacking
	if (!isArchiveValid()) return false;

	// unpack header, body, footer
	unpackHdr(msg);
	msg.setPayload( getDataLength(), getDataPtr() );
	unpackFtr(msg);
	return true;
}


}} // namespace DeVivo::Junior

#endif

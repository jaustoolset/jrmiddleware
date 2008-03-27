/*! 
 ***********************************************************************
 * @file      TransportArchive.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */

#ifndef  __TRANSPORT_ARCHIVE_H
#define  __TRANSPORT_ARCHIVE_H

#include "Archive.h"

namespace DeVivo {
namespace Junior {

const unsigned short OPC_HeaderSize = sizeof("JAUS01.0");
//
// A Transport Archive is a specialty archive that includes
// on-the-wire header bits (either JUDP or OPC).
//
class TransportArchive : public Archive
{
  public:
    TransportArchive() : Archive() {}
    ~TransportArchive(){}

    virtual void getMsgLength( unsigned short& length ) { }
    virtual void setMsgLength( unsigned short length ) {length = 0;}
    virtual void getHCFlags( unsigned char& flags ) {flags = 0;}
    virtual void setHCFlags( unsigned char flags ) { }
    virtual void getHCLength( unsigned char& length ) {length = 0;}
    virtual void setHCLength( unsigned char length ) { }
    virtual void getHCNumber( unsigned char& num ) { num = 0;}
    virtual void setHCNumber( unsigned char num )  {  }
    virtual unsigned char getHeaderLength() {return 0;}
    virtual void removeHeadMsg() { }
    virtual char* getJausMsgPtr( ) { return &data[0]; }
};


//
// Define a child class for handling JUDP archives.  This includes
// header compression and data size management.
//
class JUDPArchive : public TransportArchive
{
  public:
    JUDPArchive() : TransportArchive()
    {
        // Seed the packed data
        growBuffer(data_length + 5);
        memset(data, 0, 5);
        data[0] = 1;
        data_length = 5;
    }
    ~JUDPArchive(){}

    void getMsgLength( unsigned short& length ) 
    {
        getValueAt(3, length); 
        length = ntohs(length);
    }
    void setMsgLength( unsigned short length )
    {
        unsigned short temp = htons( length );
       *((unsigned short*) &data[3]) = temp;
    }
    void getHCFlags( unsigned char& flags )
    {
        getValueAt( 2, flags);
        flags &= 0xC0; // flags are the highest 2 bits
        flags = flags >> 6;
    }
    void setHCFlags( unsigned char flags )
    {
        // Set the bits without clobbering length
        data[2] = (data[2] & 0x3F) | (flags << 6);
    }
    void getHCLength( unsigned char& length )
    {
        getValueAt( 2, length );
        length &= 0x3F; // length is the lower 6 bits 
    }
    void setHCLength( unsigned char length )
    {
        // Set length bits without clobbering flags
        data[2] = (data[2] & 0xC0) | length;
    }
    void getHCNumber( unsigned char& num ) { getValueAt( 1, num ); }
    void setHCNumber( unsigned char num )  { data[1] = num; }
    unsigned char getHeaderLength() {return 1;}
    char* getJausMsgPtr( ) { return &data[5]; }

    void removeHeadMsg()
    {
        // Remove the first message in the archive.
        // This gets complicated since we need to remove
        // the Header Compression bits associated with it.
        unsigned short length;
        getMsgLength(length);
        removeAt(1, 4+length);
    }
};


//
// Define a child class for handling OPC archives.  They
// don't support anything.
//
class OPCArchive : public TransportArchive
{
  public:
    OPCArchive() : TransportArchive()
    {
        // Seed the packed data
        growBuffer(data_length + OPC_HeaderSize);
        strncpy(data, "JAUS01.0", OPC_HeaderSize);
        data_length = OPC_HeaderSize;
    }
    ~OPCArchive(){}

    void getMsgLength( unsigned short& length ) 
    {
        // Length does not include header
        length = getArchiveLength();
        length -= OPC_HeaderSize;
    }
    char* getJausMsgPtr( ) { return &data[OPC_HeaderSize]; }
    unsigned char getHeaderLength() {return OPC_HeaderSize;}
    void removeHeadMsg()
    {
        // Remove the first message in the archive.
        unsigned short length;
        getMsgLength(length);
        removeAt(OPC_HeaderSize, length);
    }

};


}} // namespace DeVivo::Junior



#endif

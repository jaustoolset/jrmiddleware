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
#include "ChecksumCRC.h"
#include "JrLogger.h"

namespace DeVivo {
namespace Junior {

const unsigned short OPC_HeaderSize = 8; // length of "JAUS01.0"
//
// A Transport Archive is a specialty archive that includes
// on-the-wire header bits (either JUDP or OPC).
//
class TransportArchive : public Archive
{
  public:
    TransportArchive() : Archive() {}
    ~TransportArchive(){}

    virtual void getJausMsgLength( unsigned short& length ) = 0;
    virtual void setJausMsgLength( unsigned short length ) = 0;
    virtual void removeHeadMsg() = 0;
    virtual char* getJausMsgPtr( ) = 0;
    virtual void  setJausMsgData(Archive& msg) = 0; 
    virtual bool isArchiveValid(){return true;}
};


//
// Define a child class for handling JUDP archives.  This includes
// header compression and data size management.
//
class JUDPArchive : public TransportArchive
{
  public:
    JUDPArchive() : TransportArchive(){}
    ~JUDPArchive(){}
    void getJausMsgLength( unsigned short& length ) 
    {
        // Msg length is packed Big Endian
        enum PackMode oldmode = getPackMode();
        setPackMode(BigEndian);
        getValueAt(3, length);
        setPackMode(oldmode);
    }
    void setJausMsgLength( unsigned short length )
    {
        // Msg length is packed Big Endian
        enum PackMode oldmode = getPackMode();
        setPackMode(BigEndian);
        setValueAt(3, length);
        setPackMode(oldmode);
    }
    char* getJausMsgPtr( ) { return &data[5]; }
    void  setJausMsgData(Archive& msg)
    {
        // set the header
        growBuffer(5);
        memset(data, 0, 5);
        data[0] = 1;
        data_length = 5;

        // append the JAUS message and set the message length
        append(msg);
        setJausMsgLength(msg.getArchiveLength());
    }
    void removeHeadMsg()
    {
        // Remove the first message in the archive.
        // This gets complicated since we need to remove
        // the Header Compression bits associated with it.
        unsigned short length;
        getJausMsgLength(length);
        if (length > (getArchiveLength() - 5))
            length = getArchiveLength() - 5;
        removeAt(1, 4+length);
    }
    bool isArchiveValid()
    {
        // Check the header
        if (getArchiveLength() < 5) return false;
        if (data[0] != 1) return false;

        // Check the length
        unsigned short jausMsgLen;
        getJausMsgLength(jausMsgLen);
        if (jausMsgLen > data_length) return false;
        return true;
    }
};


//
// Define a child class for handling OPC archives.  
//
class OPCArchive : public TransportArchive
{
  public:
    OPCArchive() : TransportArchive(){}
    ~OPCArchive(){}

    void getJausMsgLength( unsigned short& length ) 
    {
        // Length does not include header
        length = getArchiveLength();
        length -= OPC_HeaderSize;
    }
    void setJausMsgLength(unsigned short length){};
    char* getJausMsgPtr( ) { return &data[OPC_HeaderSize]; }
    void removeHeadMsg()
    {
        // Remove everything except the header
        removeAt(OPC_HeaderSize, getArchiveLength()-OPC_HeaderSize);
    }
    void  setJausMsgData(Archive& msg)
    {
        // reset the data_length to drop the payload
        growBuffer(OPC_HeaderSize);
        strncpy(data, "JAUS01.0", OPC_HeaderSize);
        data_length = OPC_HeaderSize;

        // append the JAUS message and set the message length
        append(msg);
        setJausMsgLength(msg.getArchiveLength());
    }
    bool isArchiveValid()
    {
        if (getArchiveLength() <= OPC_HeaderSize) return false;
        return true;
    }
};

//
// Define a child class for handling JSerial archives.  This includes
// header compression and data size management.
//
class JSerialArchive : public TransportArchive
{
  public:
    JSerialArchive() : TransportArchive(){}
    ~JSerialArchive(){}
    void getJausMsgLength( unsigned short& length ) 
    {
        char addressSize = usesExplicitAddress() ? 2 : 0;
        getValueAt(11+addressSize, length);
    }
    void setJausMsgLength( unsigned short length )
    {
        // We need to set both the message length and packet length
        char addressSize = usesExplicitAddress() ? 2 : 0;
        setPackMode(LittleEndian);
        setValueAt(11+addressSize, length);
        setValueAt(3, (unsigned short)(length+4));
    }
    char* getJausMsgPtr( ) 
    { 
        char addressSize = usesExplicitAddress() ? 2 : 0;
        return &data[13+addressSize]; 
    }
    void removeHeadMsg()
    {
        char addressSize = usesExplicitAddress() ? 2 : 0;

        // Remove the first message in the archive.
        // This gets complicated since we need to remove
        // the Header Compression bits associated with it.
        unsigned short length;
        getJausMsgLength(length);
        if (length > (getArchiveLength() - 17 - addressSize))
            length = getArchiveLength() - 17 - addressSize;
        removeAt(9+addressSize, 4+length);
    }
    void  setJausMsgData(Archive& msg)
    {
        // Seed the packed data
        growBuffer(17);
        memset(data, 0, 17);
        data[0] = 0x10; // <DLE>
        data[1] = 0x01; // <SOH>
        data[2] = 0x10; // version (no acknowledgement)
        data[5] = 0x10; // <DLE>
        data[6] = 0x02; // <STX>
        data[13] = 0x10; // <DLE>
        data[14] = 0x03; // <ETX>
        data_length = 17;

        // insert the data and update length
        insertAt(13, msg);
        setJausMsgLength(msg.getArchiveLength());
    }
    bool usesExplicitAddress()
    {
        if ((data[7] == 0x10) && 
            (data[8] == 0x02) )
            return true;
        return false;
    }
    bool usesImplicitAddress()
    {
        if ((data[5] == 0x10) && 
            (data[6] == 0x02) )
            return true;
        return false;
    }
    bool isArchiveValid()
    {
        // Make sure we have enough bytes to bother reading
        if (getArchiveLength() < 17)
            return false;

        // Verify the packets version
        if ((data[2] & 0XF0) != 0x10)
        {
            JrDebug << "Invalid serial packet (invalid version)\n";
            return false;
        }

        // Figure out where the header checksum is
        char addressSize = usesExplicitAddress() ? 2 : 0;
        if (!usesExplicitAddress() && !usesImplicitAddress())
        {
            JrDebug << "Invalid serial packet (no STX delineator)\n";
            return false;
        }

        // Verify that the packet matches the given length
        unsigned short packetLength = 0;
        getValueAt(3, packetLength);
        if (packetLength != (getArchiveLength() - 13 - addressSize))
        {
            JrDebug << "Invalid serial packet (invalid size: " << packetLength <<
                " versus " << getArchiveLength() << ")\n";
            return false;
        }

        // Check if the final 4-bytes include a DLE-ETX pair
        if (( data[data_length-4] != 0x10 ) ||
            ( data[data_length-3] != 0x03) )
        {
            JrDebug << "Invalid serial packet (no etx found)\n";
            return false;
        }

        // Check the header checksum.  Compare value
        // from the packet and the one computed locally.
        unsigned short header_checksum;
        getValueAt(7+addressSize, header_checksum);
        unsigned short local_header_checksum = crc_calculate(
                (unsigned char*)&data[2], 0xFFFF, 4+addressSize);
        if (header_checksum != local_header_checksum)
        {
            JrError << "Serial header checksum mismatch (" << header_checksum <<
                " versus " << local_header_checksum << ")\n";
            return false;
        }

        // Check the packet checksum
        unsigned short packet_checksum;
        getValueAt(data_length-2, packet_checksum);
        unsigned short local_packet_checksum = crc_calculate(
                (unsigned char*)&data[7+addressSize], 
                local_header_checksum, getArchiveLength()-4-7-addressSize);
        if (packet_checksum != local_packet_checksum)
        {
            JrError << "Serial packet checksum mismatch (" << packet_checksum <<
                " versus " << local_packet_checksum << ")\n";
        }

        // getting to this point means we have a valid packet
        JrDebug << "Found valid serial packet\n";
        return true;
    }
    void finalizePacket()
    {
        setPackMode(LittleEndian);
        char addressSize = usesExplicitAddress() ? 2 : 0;

        // Compute the checksum for the header
        unsigned short header_check;
        header_check = crc_calculate((unsigned char*)&data[2], 
                                      0xFFFF, 4+addressSize);
        setValueAt(7+addressSize, header_check);

        // Now compute the checksum for the packet.  Note that we have to do
        // this BEFORE padding for DLE transparency.
        unsigned short packet_check;
        packet_check = crc_calculate((unsigned char*)&data[7+addressSize],
                           header_check, getArchiveLength()-4-7-addressSize);
        setValueAt(getArchiveLength()-2, packet_check);

        // insert DLE markers for data fields
        // that coincidentally have the same value as the DLE.
        int SOH_shift = 0;
        for (int i=0; i<data_length; i++)
        {
            // First make sure this is not an actual diagraph
            // (and therefore *should* be a DLE.
            if ((i==0) || (i==5+addressSize+SOH_shift) || (i==data_length-4))
                continue;

            // now check for a DLE equivalent byte
            if (data[i] == 0x10)
            {
                // insert a DLE character at this location
                // If necessary, grow the buffer to accommodate the new data
                growBuffer(data_length + 20);

                // Shift the tail data backward to create an empty region in the buffer
                char* temp_data = (char*) malloc( buffer_size );
                int tail_size = data_length - i;
                memcpy( temp_data, &data[i], tail_size );
                memcpy( &data[i+1], temp_data, tail_size ); 
                free(temp_data);
                
                // now insert the DLE
                data[i] = 0x10;
                data_length += 1;

                // If we inserted an element before the DLE-SOH pair,
                // we need to shift our expectations of where
                // to find the SOH
                if (i<(5+addressSize)) SOH_shift++;

                // manually increment the loop counter, since we
                // inserted the new element.
                i++;
            }
        }
    }
};


//
// Define a child class for handling JTCP archives.  The header
// includes only version and data size.
//
class JTCPArchive : public TransportArchive
{
  public:
    JTCPArchive() : TransportArchive(){}
    ~JTCPArchive(){}
    void getJausMsgLength( unsigned short& length ) 
    {
        // Msg length is packed Big Endian
        enum PackMode oldmode = getPackMode();
        setPackMode(BigEndian);
        getValueAt(1, length);
        setPackMode(oldmode);
    }
    void setJausMsgLength( unsigned short length )
    {
        // Msg length is packed Big Endian
        enum PackMode oldmode = getPackMode();
        setPackMode(BigEndian);
        setValueAt(1, length);
        setPackMode(oldmode);
    }
    char* getJausMsgPtr( ) { return &data[3]; }
    void removeHeadMsg()
    {
        // Remove the first message in the archive.
        unsigned short length;
        getJausMsgLength(length);
        if (length > (getArchiveLength() - 3))
            length = getArchiveLength() - 3;
        removeAt(1, 2+length);
        JrFull << "Removed " << 2+length << " bytes from archive.  New size: " << 
            getArchiveLength() << std::endl;
    }
    void removeVersion()
    {
        // remove the version number
        removeAt(0,1);
    }
    void setJausMsgData(Archive& msg)
    {
        // reset the packet details, reserving space for packet length
        growBuffer(3);
        memset(data, 0, 3);
        data[0] = 1;
        data_length = 3;

        // append the JAUS message and set the message length
        append(msg);
        setJausMsgLength(msg.getArchiveLength());
    }
    bool isArchiveValid()
    {
        // Make sure we have enough bytes to bother reading
        if (getArchiveLength() < 3)
            return false;

        // Verify the packets version
        if (data[0] != 1)
        {
            JrDebug << "Invalid TCP packet (invalid version)\n";
            return false;
        }

        // Verify that the packet meets or exceeds the message length
        unsigned short messageLength = 0;
        getJausMsgLength(messageLength);
        if (messageLength >  (getArchiveLength() - 3))
        {
            JrDebug << "Invalid TCP packet (invalid size: " << messageLength <<
                " versus " << getArchiveLength() << ")\n";
            return false;
        }

        // getting to this point means we have a valid packet
        JrDebug << "Found valid TCP packet\n";
        return true;
    }
};



}} // namespace DeVivo::Junior



#endif

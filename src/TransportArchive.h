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
#include "ChecksumCRC.h"

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
    virtual void  setJausMsgData(Archive& msg) 
    { 
        append(msg);
        setMsgLength( msg.getArchiveLength() );
    }
    virtual void reset() = 0;
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
        reset();
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
    void  setJausMsgData() { }

    void removeHeadMsg()
    {
        // Remove the first message in the archive.
        // This gets complicated since we need to remove
        // the Header Compression bits associated with it.
        unsigned short length;
        getMsgLength(length);
        if (length > (getArchiveLength() - 5))
            length = getArchiveLength() - 5;
        removeAt(1, 4+length);
    }
    void reset()
    {
        // reset the packet details
        growBuffer(5);
        memset(data, 0, data_length);
        data[0] = 1;
        data_length = 5;
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
        reset();
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
        if (length > (getArchiveLength() - OPC_HeaderSize))
            length = getArchiveLength() - OPC_HeaderSize;
        removeAt(OPC_HeaderSize, length);
    }
    void reset()
    {
        // reset the data_length to drop the payload
        growBuffer(OPC_HeaderSize);
        memset(data, 0, data_length);
        strncpy(data, "JAUS01.0", OPC_HeaderSize);
        data_length = OPC_HeaderSize;
    }
};

//
// Define a child class for handling JSerial archives.  This includes
// header compression and data size management.
//
class JSerialArchive : public TransportArchive
{
  public:
    JSerialArchive() : TransportArchive()
    {
        reset();
    }
    ~JSerialArchive(){}

    void getMsgLength( unsigned short& length ) 
    {
        char addressSize = usesExplicitAddress() ? 2 : 0;
        getValueAt(11+addressSize, length);
    }
    void setMsgLength( unsigned short length )
    {
        // We need to set both the message length and packet length
        char addressSize = usesExplicitAddress() ? 2 : 0;
        setPackMode(LittleEndian);
        setValueAt(11+addressSize, length);
        setValueAt(3, (unsigned short)(length+4));
    }
    void getHCFlags( unsigned char& flags )
    {
        getValueAt( 10, flags);
        flags &= 0xC0; // flags are the highest 2 bits
        flags = flags >> 6;
    }
    void setHCFlags( unsigned char flags )
    {
        // Set the bits without clobbering length
        data[10] = (data[10] & 0x3F) | (flags << 6);
    }
    void getHCLength( unsigned char& length )
    {
        getValueAt( 10, length );
        length &= 0x3F; // length is the lower 6 bits 
    }
    void setHCLength( unsigned char length )
    {
        // Set length bits without clobbering flags
        data[10] = (data[10] & 0xC0) | length;
    }
    void getHCNumber( unsigned char& num ) { getValueAt( 9, num ); }
    void setHCNumber( unsigned char num )  { data[9] = num; }
    unsigned char getHeaderLength() 
    {
        char addressSize = usesExplicitAddress() ? 2 : 0;
        return (17+addressSize);
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
        getMsgLength(length);
        if (length > (getArchiveLength() - getHeaderLength()))
            length = getArchiveLength() - getHeaderLength();
        removeAt(9+addressSize, 4+length);
    }
    void reset()
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
    }
    void  setJausMsgData(Archive& msg) 
    { 
        char addressSize = usesExplicitAddress() ? 2 : 0;

        // because of the footer and CRCs, we need to overload this.
        insertAt(13+addressSize, msg);
        setMsgLength(msg.getArchiveLength());
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
        if (getArchiveLength() < getHeaderLength())
            return false;

        // Verify the packets version
        if ((data[2] & 0XF0) != 0x10)
        {
            //printf("Invalid version\n");
            return false;
        }

        // Figure out where the header checksum is
        char addressSize = usesExplicitAddress() ? 2 : 0;
        if (!usesExplicitAddress() && !usesImplicitAddress())
        {
            //printf("No DLE-STX found\n");
            return false;
        }

        // Verify that the packet matches the given length
        unsigned short packetLength = 0;
        getValueAt(3, packetLength);
        if (packetLength != (getArchiveLength() - 13 - addressSize))
        {
            //printf("invalid packet length: %ld versus %ld\n", packetLength, getArchiveLength());
            return false;
        }

        // Check if the final 4-bytes include a DLE-ETX pair
        if (( data[data_length-4] != 0x10 ) ||
            ( data[data_length-3] != 0x03) )
        {
            //printf("didn't find etx: 0x%x%x\n", 
            //    data[data_length-4], data[data_length-3]);
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
            //printf("Header checksum mismatch (%d versus %d)\n",
            //                header_checksum, local_header_checksum);
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
            //printf("Packet checksum mismatch (%d versus %d)\n",
            //                packet_checksum, local_packet_checksum);
            return false;
        }

        // getting to this point means we have a valid packet
        //printf("Found valid archive\n");
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

}} // namespace DeVivo::Junior



#endif

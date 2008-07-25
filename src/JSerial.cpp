/*! 
 ***********************************************************************
 * @file      JSerial.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */

#include "JSerial.h"
#include "Message.h"
#include "OS.h"
#include <fcntl.h>

using namespace DeVivo::Junior;

// Define JSerial constants
const char JSERIAL_DLE = 0x10;
const char JSERIAL_SOH = 0x01;
const char JSERIAL_STX = 0x02;
const char JSERIAL_ETX = 0x03;

// Some values are different in Linux/Windows environment
#ifdef WINDOWS
#define SERIAL_PATH "//./"
#define getlasterror GetLastError()
#else
#define INVALID_HANDLE_VALUE 0
#define SERIAL_PATH "/dev/"
#define getlasterror errno
#define CloseHandle close
#endif


// Class definition
JSerial::JSerial():
   hComm(0),
   _use_opc(0),
   previousByteWasDLE(false)
{
}

JSerial::~JSerial()
{
    if (hComm > 0) CloseHandle(hComm);
    hComm = 0;
}

Transport::TransportError JSerial::configureLink()
{
    unsigned int baudrate = 9600;
    _config.getValue("SerialBaudRate", baudrate);
    std::string parity = "none";
    _config.getValue("SerialParity", parity);
    unsigned char stopbits = 1;
    _config.getValue("SerialStopBits", stopbits);
    unsigned char software_dataflow = 0;
    _config.getValue("SerialSoftwareFlowControl", software_dataflow);

    // Check for valid parameters
    if ( !JrStrCaseCompare(parity, "odd") && 
         !JrStrCaseCompare(parity, "even") &&
         !JrStrCaseCompare(parity, "none") )
    {
        printf("Invalid serial parity in config file.  Using <none>\n");
        parity = "none";
    }
    if ((stopbits != 1) && (stopbits != 2))
    {
        printf("Invalid serial stop bits.  Using 1\n");
        stopbits = 1;
    }

    // debug
    printf("ByteSize:%d   Parity:%s    Stop:%d   Baud:%d  FlowControl:%s\n", 8,
        parity.c_str(), stopbits, baudrate,
        software_dataflow ? "software" : "hardware");


#ifdef WINDOWS
    
    // Get the Data Control Block
    DCB dcb = {0};
    if (!GetCommState(hComm, &dcb)) return InitFailed;

    // Set-up for configured parity, baud, and stop
    dcb.ByteSize = 8;
    dcb.BaudRate = baudrate;
    if (JrStrCaseCompare(parity, "odd"))
        dcb.Parity = ODDPARITY;
    else if (JrStrCaseCompare(parity, "even"))
        dcb.Parity = EVENPARITY;
    else
        dcb.Parity = NOPARITY;
    if (stopbits == 2)
        dcb.StopBits = TWOSTOPBITS;
    else
        dcb.StopBits = ONESTOPBIT;
    if (software_dataflow)
    {
        // Configure for software flow control (XOn/XOff)
        dcb.fInX = 1; dcb.fOutX = 1;
        dcb.fOutxCtsFlow = 0;
    }
    else
    {
        // Configure for hardware flow control (RTS-CTS)
        dcb.fInX = 0; dcb.fOutX = 0;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcb.fOutxCtsFlow = 1;
    }
    SetCommState(hComm, &dcb);
    
    // Get comm timeouts
    COMMTIMEOUTS cto;
    if (GetCommTimeouts(hComm, &cto) == 0)
    {
        printf("Failed to configure serial port (error=%ld)\n", getlasterror);
        return InitFailed;
    }

    // Set the comm timeouts to values we like
    cto.ReadIntervalTimeout = MAXDWORD;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant = 0;
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts(hComm, &cto);

#else

    // Get the current options set
    struct termios options;
    tcgetattr(hComm, &options);

    // set the baud rate
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    // set the character size
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // set the parity
    options.c_cflag &= ~(PARENB | PARODD);
    if (JrStrCaseCompare(parity, "odd"))
    {
        options.c_iflag |= (INPCK | ISTRIP);
        options.c_cflag |= (PARENB | PARODD);
    }
    else if (JrStrCaseCompare(parity, "even"))
    {
        options.c_iflag |= (INPCK | ISTRIP);
        options.c_cflag |= PARENB;
    }

    // set the stop bits
    options.c_cflag &= ~CSTOPB;
    if (stopbits == 2) options.c_cflag |= CSTOPB;

    // Set receiver and local modes
    options.c_cflag |= (CLOCAL | CREAD);

    // flow control (hardware or software)
    if (software_dataflow)
    {
        // Configure for software flow control (XOn/XOff)
        options.c_cflag &= ~CRTSCTS;
        options.c_iflag |= (IXON | IXOFF | IXANY);
    }
    else
    {
        // Configure for hardware flow control (RTS-CTS)
        options.c_cflag |= CRTSCTS;
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    // enable raw output (this prevent interpretation of
    // the data stream for things line CR-LR
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST; 

    // Set the new options
    tcsetattr(hComm, TCSANOW, &options);

    // Make the port non-blocking
    fcntl(hComm, F_SETFL, FNDELAY);

#endif

    return Ok;
}

Transport::TransportError JSerial::initialize( std::string filename )
{
    // Parse the config file
    _config.parseFile(filename);

    // See if Serial has been deactivated.
    char use_serial = 0;
    _config.getValue("EnableSerialInterface", use_serial);
    if (!use_serial)
    {
        printf("Serial communication deactivated in configuration file\n");
        return InitFailed;
    }

    // Pull the com port name
    std::string portname = "COM1";
    _config.getValue("SerialPortName", portname);
    portname = SERIAL_PATH + portname;
    printf("Serial: Using port %s\n", portname.c_str());

#ifdef WINDOWS
    // Open a file for reading/writing to the port
    hComm = CreateFile(portname.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0,
                        OPEN_EXISTING, 0, 0);
#else
    // Open a file descriptor
    hComm = open(portname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
#endif

    // Check for valid response
    if (hComm == INVALID_HANDLE_VALUE) 
    {
        printf("Failed to open port %s (error=%ld)\n", portname.c_str(), getlasterror);
        hComm = 0;
        return InitFailed;
    }

    // Configure the link for parity, baud rate, etc.
    configureLink();
    return Ok;
}

Transport::TransportError JSerial::sendMsg(Message& msg)
{
    TransportError ret = AddrUnknown; 

    // Only send messages if the destination is known to us
    for (int i = 0; i < _map.getList().size(); i++)
    {
        if ((_map.getList()[i].first == msg.getDestinationId()) &&
            (_map.getList()[i].first != msg.getSourceId()))
        {
            // Send to this entry
            ret = sendMsg(msg, _map.getList()[i].second);
        }
    }

    return ret;
}

Transport::TransportError JSerial::sendMsg(Message& msg, HANDLE handle)
{
    // Assume the best...
    Transport::TransportError result = Ok;

    //
    // Creating a byte stream (payload) for the message
    //
    JSerialArchive payload;

    //
    // Now pack the message for network transport 
    //
    Archive msg_archive;
    msg.pack(msg_archive);
    payload.setJausMsgData( msg_archive );

    // Ugh.  5669 requires prefacing any DLE-equivalent characters
    // with a DLE marker.  This is similar to a "\\" marker for
    // when printing a slash through printf.  Manually pad the archive.
    payload.finalizePacket();

#ifdef WINDOWS

    // Write to the open port
    DWORD bytesWritten;
    bool ret = WriteFile(handle, payload.getArchive(), 
                      payload.getArchiveLength(),
                      &bytesWritten, NULL);
    if (ret == 0) 
    {
        printf("Failed to Serial::Write\n");
        result = Failed;
    }

#else

    int bytesWritten = write(handle, payload.getArchive(),
                        payload.getArchiveLength());

#endif

    // make sure we wrote the whole packet
    if (bytesWritten != payload.getArchiveLength())
    {
        printf("Failed to write full packet (%ld of %ld)\n", 
            bytesWritten, payload.getArchiveLength());
        result = Failed;
    }
    //else
    //    printf("Serial: Wrote %ld bytes\n", bytesWritten);

    return result;
}


Transport::TransportError JSerial::recvMsg(MessageList& msglist)
{
    char buffer[5000];
    
    TransportError ret = NoMessages;
 
#ifdef WINDOWS
    // Read from the serial port
    DWORD bytesRead;
    if (!ReadFile(hComm, buffer, 5000, &bytesRead, NULL))
    {
        printf("Failed to read serial port (%ld)\n", getlasterror);
        return Failed;
    }
#else
    int bytesRead = read(hComm, buffer, 5000);
#endif

    // Nothing to do if we didn't read any bytes
    if (bytesRead <= 0) return NoMessages;
    //printf("read %ld bytes\n", bytesRead);

    // We need to process the incoming stream byte-wise, since the 
    // stream may contain DLE-marked instructions.
    for (int i=0; i<bytesRead; i++)
    {
        if (previousByteWasDLE)
        {
            // Previous byte was a DLE marker.  Next byte
            // tells the interpretation of the instruction.
            if (buffer[i] == JSERIAL_DLE)
            {
                // DLE was used to mark a data element that
                // coincidentally had the same value as the
                // DLE marker.  Strip the unnecessary DLE from
                // the packet.
                unusedBytes << buffer[i];
            }
            else
            {
                if (buffer[i] == JSERIAL_SOH)
                {
                    // DLE marks a packet start.  See if the 
                    // unused bytes contain a valid packet
                    if (unusedBytes.isArchiveValid())
                        ret = extractMsgsFromPacket(msglist);

                    // Now clear the unused bytes buffer
                    // so we can start the new packet.
                    unusedBytes.clear();
                }

                // Add the DLE marker (from the previous byte)
                // as well as the current byte to the 
                // unusedBytes buffer.
                unusedBytes << JSERIAL_DLE;
                unusedBytes << buffer[i];
            }

            // Reset the flag indicating previous byte was DLE
            previousByteWasDLE = false;
        }
        else if (buffer[i] == JSERIAL_DLE)
        {
            // DLE marker preceding special byte.
            previousByteWasDLE = true;
        }
        else
        {
            // regular data byte
            unusedBytes << buffer[i];
        }
    }

    // Check packet for completeness.  If so, parse message(s).
    if (unusedBytes.isArchiveValid())
        ret = extractMsgsFromPacket(msglist);
    
    // done processing this read.  return.
    return ret;
}



Transport::TransportError JSerial::broadcastMsg(Message& msg)
{
    // Unlike a send, the broadcast does not check for a valid
    // destination field.  It simply pushes everything across
    // the port using the default connection.
    return sendMsg(msg, hComm);
}

Transport::TransportError JSerial::extractMsgsFromPacket(MessageList& msglist)
{
    TransportError ret = NoMessages;
    unsigned short jausMsgLength;

    // A single packet may have multiple JAUS messages on it, each
    // with there own header compression flags.  We need to parse through
    // the entire packet, remove each message one at a time and
    // adding it to the return list.
    while (unusedBytes.getArchiveLength() > unusedBytes.getHeaderLength())
    {
        // If the message length is zero, this message was only a transport
        // message (probably a Header Compression message).  Nothing more
        // to do.
        unusedBytes.getMsgLength( jausMsgLength );
        if ( jausMsgLength != 0 )
        {
            // Extract the payload into a message
            // UGH!! Two copies here.  Need to eliminate this.
            Archive archive;
            archive.setData( unusedBytes.getJausMsgPtr(), jausMsgLength );
            Message* msg = new Message();
            msg->unpack(archive);

            //
            // Add the source to the transport discovery map.
            //
            _map.addAddress( msg->getSourceId(), hComm );

            // Add the message to the list and change the return value
            msglist.push_back(msg);
            ret = Ok;
        }

        // Remove this message from the archive, so
        // we can process the next message in the packet.
        unusedBytes.removeHeadMsg( );
    }

    // Clear the data buffer and return
    unusedBytes.clear();
    return ret;
}

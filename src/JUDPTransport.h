//  JAUS UDP Transport implementation
#ifndef  __JAUS_UDP_TRANSPORT_H
#define __JAUS_UDP_TRANSPORT_H

#include "Transport.h"
//#include "IPMap.h"
#include "HeaderCompression.h"
#include "AddressMap.h"

// Define a helper class
class IP_ADDRESS
{
  public:
    IP_ADDRESS():addr(0), port(0){};
    IP_ADDRESS(struct sockaddr_in in) :
                   addr(in.sin_addr.s_addr),port(in.sin_port){};
    ~IP_ADDRESS(){};

   bool operator==(IP_ADDRESS in)
   {
       if ((addr == in.addr) && (port == in.port)) return true;
       return false;
   }
    
   unsigned long addr;
   unsigned short port;
};

class JUDPTransport : public Transport
{
public:
    JUDPTransport();
   ~JUDPTransport();

    // All functions are abstract
    TransportError sendMsg(Message& msg);
    TransportError broadcastMsg(Message& msg);
    TransportError recvMsg(Message& msg);
    TransportError initialize(std::string config);

protected:

    AddressMap<IP_ADDRESS> _map;
    int   _socket;
    HeaderCompressionTable _inTable, _outTable;

    // Internal function to help with packing the transport header
    void packHdr( JUDPArchive& packed_msg );

    // Internal functions to help with compression
    void uncompressHeader( JUDPArchive&, JAUS_ID, struct sockaddr_in& );
    void compressHeader  ( JUDPArchive&, JAUS_ID );

};

#endif


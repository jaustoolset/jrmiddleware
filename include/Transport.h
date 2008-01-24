//  Transport class - an abstract class for a Transport instantiation
#ifndef  __TRANSPORT_H
#define __TRANSPORT_H

#include <string>
#include "Message.h"

class Transport
{
public:
    Transport(){};
   ~Transport(){};

    //
    // Define the error codes
    //
    enum TransportError 
    {
        Ok, NoMessages, InvalidConfigFile, InitFailed, AddrUnknown, Failed 
    };

    // All functions are abstract
    virtual TransportError sendMsg(Message& msg) = 0;
    virtual TransportError recvMsg(Message& msg) = 0;
    virtual TransportError broadcastMsg(Message& msg) = 0;
    virtual TransportError initialize(std::string config) = 0;

    // Debugging
    std::string enumToString( TransportError code );
   
protected:

};

inline std::string Transport::enumToString( TransportError code )
{
    switch (code)
    {
        case Transport::Ok:
            return std::string("Success");
        case Transport::NoMessages:
            return std::string("No messages in queue");
        case Transport::InvalidConfigFile:
            return std::string("Invalid config file");
        case Transport::InitFailed:
            return std::string("Initialization failed");
        case Transport::AddrUnknown:
            return std::string("Unknown destination address");
        case Transport::Failed:
            return std::string("Unknown failure");
        default:
            return std::string("Unkown error code");
    }
}


      

#endif


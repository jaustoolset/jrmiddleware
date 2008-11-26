/*! 
 ***********************************************************************
 * @file      Types.h
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
#ifndef __COMMON_TYPES_H
#define __COMMON_TYPES_H

#include <string>
#include <list>
#include <sstream>
#include "OS.h"

namespace DeVivo {
namespace Junior {

static unsigned char getByte(unsigned long in, char num)
{
    return ((unsigned char)(in>>(num*8)));
}

// versions of the header/footer supported
typedef enum { UnknownVersion = 0, OPC, AS5669, AS5669A } MsgVersion;
static std::string VersionEnumToString(MsgVersion v)
{
	if (v == OPC) return "OPC";
	if (v == AS5669) return "AS5669";
	if (v == AS5669A) return "AS5669A";
	return "UnknownVersion";
}
static MsgVersion VersionStringToEnum(std::string v)
{
	if ((v == "OPC") || (v == "opc")) return OPC;
	if ((v == "AS5669") || (v == "as5669")) return AS5669;
	if ((v == "AS5669A") || (v == "as5669A")) return AS5669A;
	return UnknownVersion;
}

// Types for JAUS_ID.  The JAUS ID is simply an unsigned long
// but has to watch out for wildcard bytes (0xFF) during
// comparison operations.
class JAUS_ID
{
  public:
    JAUS_ID(){val=0;};
    JAUS_ID(unsigned long in){val=in;}
    JAUS_ID(std::string str)
    {
        std::stringstream ss; ss << str; ss >> val;
    }
    ~JAUS_ID(){}

    unsigned long val;
    bool operator==(const JAUS_ID& in) const
    {
        // Check for the easy case for computation efficiency.
        if (val == in.val) return true;

        // Each byte may have a wildcard (0xFF), so we need to check bytewise
        // comparisons.
        for (char i=0; i<4; i++)
            if ((getByte(val, i) != 0xFF) &&
                (getByte(in.val, i) != 0xFF) &&
                (getByte(val, i) != getByte(in.val, i)))
            {
                return false;
            }

        // Getting to this point means each byte is equivalent or
        // a wildcard.
        return true;
    }
    bool operator<(const JAUS_ID& in) const
    {
        if (val < in.val) return true;
        return false;
    }
    bool operator!=(const JAUS_ID& in) const
    {
        if (val != in.val) return true;
        return false;
    }
    bool containsWildcards()
    {
        // Each byte may have a wildcard (0xFF), so we need to check each
        for (char i=0; i<4; i++)
            if (getByte(val, i) == 0xFF) return true;
        return false;
    }
};

//
// Define a helper class for IP address (with port).
// Note that this is stored internally in NETWORK BYTE ORDER.
//
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
   std::string toString()
   {
       std::stringstream ss;
       ss << inet_ntoa(*(in_addr*) &addr) << ":" << ntohs(port);
       return ss.str();
   }
   bool fromString(std::string str)
   {
       // extract substrings for address and port (assume the
       // incoming line is of the form "dot-notation-address:port"
       std::string ip_addr_str = str.substr(0, str.find_first_of(":"));
       std::string port_str = str.substr(str.find_first_of(":")+1);
       if (ip_addr_str.empty() || port_str.empty()) return false;

       // populate the data members
       addr = inet_addr(ip_addr_str.c_str());
       port = htons((unsigned short)(strtod(port_str.c_str(), NULL)));
       return true;
   }
    
   unsigned long addr;
   unsigned short port;
};

}} // namespace DeVivo::Junior
#endif




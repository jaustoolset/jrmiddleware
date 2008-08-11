/*! 
 ***********************************************************************
 * @file      Types.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
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

// Define a helper class for IP address (with port)
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
       ss << inet_ntoa(*(in_addr*) &addr) << ":" << port;
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
       port = (unsigned short) (strtod(port_str.c_str(), NULL));
       return true;
   }
    
   unsigned long addr;
   unsigned short port;
};

}} // namespace DeVivo::Junior
#endif




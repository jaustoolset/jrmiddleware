/*! 
 ***********************************************************************
 * @file      IpAddressBook.h
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
#ifndef __IP_ADDRESS_BOOK_H
#define __IP_ADDRESS_BOOK_H

#include "Types.h"
#include "AddressMap.h"
#include "ConfigData.h"

namespace DeVivo {
namespace Junior {


class IpAddressBook : public AddressMap<IP_ADDRESS>
{
public:
    IpAddressBook(){};
   ~IpAddressBook(){};

   // Function to populate an address book from a file
   bool loadFromFile(std::string filename);
   
};

inline bool IpAddressBook::loadFromFile(std::string filename)
{
    // check for null case
    if (filename.empty()) return false;
    if (filename == "") return false;

    JrDebug << "Loading TCP address book from " << filename << std::endl;

    // Open the given file as a config file
    ConfigData addresses;
    if (addresses.parseFile(filename) != ConfigData::Ok) return false;

    // Each key in the address book should be a JAUS ID
    StringList ids;
    addresses.getKeyList(ids);

    // For each key, extract the IP address
    StringListIter iter;
    for (iter = ids.begin(); iter != ids.end(); iter++)
    {
        // Pull the ip_address port string for the id
        std::string ip_string;
        if (addresses.getValue(*iter, ip_string) != ConfigData::Ok)
            continue;

        // Make an IP_ADDRESS structure from the string
        IP_ADDRESS ip_struct;
        if (!ip_struct.fromString(ip_string)) continue;

        // Add to the map
        addAddress(JAUS_ID(*iter), ip_struct);

        JrFull << "Adding TCP address map: " << *iter <<
            " -> " << ip_struct.toString() << std::endl;
    }

    return true;
}

}} // namespace DeVivo::Junior
#endif



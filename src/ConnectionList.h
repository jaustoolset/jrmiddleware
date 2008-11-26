/*! 
 ***********************************************************************
 * @file      ConnectionMgr.h
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
#ifndef __CONNECTION_MGR_H
#define __CONNECTION_MGR_H

#include <vector>
#include "Types.h"
#include "ConfigData.h"
#include "JrLogger.h"

namespace DeVivo {
namespace Junior {



template<class AddressType>
class Connection 
{
public:
    Connection(){_version = UnknownVersion;};
	Connection(JAUS_ID id, AddressType addr, MsgVersion version):
		_address(addr), _id(id), _version(version){};
   ~Connection(){};

    // Define accessors
    AddressType getAddress(){return _address;}
	void setAddress(AddressType addy){_address = addy;}

	JAUS_ID getId(){return _id;}
	void setId(JAUS_ID id){_id = id;}

	MsgVersion getVersion() {return _version;}
	void setVersion(MsgVersion v){_version = v;}

protected:
	AddressType _address;
	JAUS_ID     _id;
	MsgVersion  _version;
};

template<class AddressType>
class ConnectionList
{
public:
	ConnectionList(){};
	~ConnectionList(){};

	typedef std::vector<Connection <AddressType>* > ConnectionListType;

	// Functions to manage the list
    bool addElement(JAUS_ID id, AddressType addr, MsgVersion version);
    bool removeElement(JAUS_ID id);

	// Functions to access the list
	bool getAddrFromId( JAUS_ID id, AddressType& addr );
	bool getMsgVersion( JAUS_ID id, MsgVersion& version );
	bool updateMsgVersion( JAUS_ID id, MsgVersion version );
    ConnectionListType& getList(){return _list;};
   
protected:

	// This should really be a list, but iterators don't seem to like
	// lists of templated class pointers.  We use a vector as a 
	// work around.
	ConnectionListType _list;
};

// define inlines
template<class AddressType>
inline bool ConnectionList<AddressType>::addElement(JAUS_ID id, 
										  AddressType addr, 
										  MsgVersion version)
{
    // Not permitted for zero id's
    if (id.val == 0) return false;

	// check for duplicates
	AddressType prevAddr; MsgVersion prevVersion;
	if (getAddrFromId(id, prevAddr) && (addr == prevAddr))
	{
		// Element with same ID and address found.  Update the version
		// and return success.
		updateMsgVersion(id, version);
		return true;
	}

	// Existing element does not exist.  Create a new connection
	Connection<AddressType>* connection = new Connection<AddressType>(id, addr, version);
	if (connection == NULL) return false;

    // Add this connection to our list
    JrFull << "Adding address book entry for id " << id.val << std::endl;
    _list.push_back(connection);
    return true;
}

template<class S>
inline bool ConnectionList<S>::removeElement(JAUS_ID id)
{
    // Wipe all entries with matching ids
    for (int i=0; i < _list.size(); i++)
        if (_list[i]->getId() == id) _list[i]->setId(0);
    return true;
}

template<class S>
inline bool ConnectionList<S>::getAddrFromId( JAUS_ID id, S& addr )
{
    if (id == 0) return false;

    // Check for a match based on the JAUS_ID
    for (int i=0; i < _list.size(); i++)
    {
        if (_list[i]->getId() == id)
        {
            addr = _list[i]->getAddress();
            return true;
        }
    }
    return false;
}

template<class S>
inline bool ConnectionList<S>::getMsgVersion( JAUS_ID id, MsgVersion& version )
{
    if (id == 0) return false;

    // Check for a match based on the JAUS_ID
    for (int i=0; i < _list.size(); i++)
    {
        if (_list[i]->getId() == id)
        {
            version = _list[i]->getVersion();
            return true;
        }
    }
    return false;
}

template<class S>
inline bool ConnectionList<S>::updateMsgVersion( JAUS_ID id, MsgVersion version )
{
    if (id == 0) return false;

    // Check for a match based on the JAUS_ID
    for (int i=0; i < _list.size(); i++)
    {
        if (_list[i]->getId() == id)
        {
            _list[i]->setVersion(version);
            return true;
        }
    }
    return false;
}


// Specialize the ConnectionList for ip_address support.
// This also allows addresses to be loaded from a configuration file.
class IpAddressBook : public ConnectionList<IP_ADDRESS>
{
public:
    IpAddressBook(){};
   ~IpAddressBook(){};

   // Function to populate an address book from a file
   bool loadFromFile(std::string filename); 
};

// Load the address book from a given configuration file.
// All entries must be of the form:
//   <ip_address_in_dot_notation>:<port>:<version>
// where <version> is the header version used by the remote
// entity.  At present, supported values are: "OPC", "AS5669",
// and "AS5669A".
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
        std::string rhs;
        if (addresses.getValue(*iter, rhs) != ConfigData::Ok) continue;

		// Parse the right hand side.  It should be of the form:
		//  address-in-dot-notation:port:version
	    std::string ip_str = rhs.substr(0,rhs.find_last_of(":"));
		std::string version_str = rhs.substr(rhs.find_last_of(":")+1);
        if (ip_str.empty() || version_str.empty()) continue;

        // Make an IP_ADDRESS structure from the string
        IP_ADDRESS ip_struct;
		if (!ip_struct.fromString(ip_str)) continue;

		// Extract the version string.  We borrow the "stripExtraChars"
		// function to clean-up extraneous quotation marks and end lines.
		addresses.stripExtraChars(version_str);
		MsgVersion version = VersionStringToEnum(version_str); 
		if (version == UnknownVersion)
		{
			JrWarn << "Unknown version specified for ID=" << *iter <<
				" (" << version_str << ").  Discarding entry." << std::endl;
			continue;
		}

        // Add to the map
        addElement(JAUS_ID(*iter), ip_struct, version);

        JrFull << "Adding entry to address map: " << *iter << " -> " << 
            ip_struct.toString() << " (version=" << version_str << ")" << std::endl;
    }

    return true;
}

}} // namespace DeVivo::Junior
#endif


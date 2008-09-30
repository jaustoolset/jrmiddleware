/*! 
 ***********************************************************************
 * @file      AddressMap.h
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
#ifndef __ADDRESS_MAP_H
#define __ADDRESS_MAP_H

#include <vector>
#include "JrLogger.h"

namespace DeVivo {
namespace Junior {



template<class S>
class AddressMap 
{
public:
    AddressMap():_list(){};
   ~AddressMap(){};

    // Define a map element
    typedef std::pair<JAUS_ID, S> Element;

    // Define accessors
    bool getAddrFromId(JAUS_ID id, S& addr);
    bool getIdFromAddr( JAUS_ID& id, S addr);
    bool addAddress(JAUS_ID id, S addr);
    bool removeAddress(JAUS_ID id);
    std::vector<Element>& getList(){return _list;};
   
protected:

    std::vector<std::pair<JAUS_ID, S> > _list;
};

// define inlines
template<class S>
inline bool AddressMap<S>::addAddress(JAUS_ID id, S addr)
{
    // Not permitted for zero id's
    if (id.val == 0) return false;

    // Watch out for duplicate entries
    for (int i=0; i < _list.size(); i++)
    {
        if ((_list[i].first == id) && (_list[i].second == addr))
            return false;
    }

    // Add this pair to the end of the vector
    JrFull << "Adding address book entry for id " << id.val << std::endl;
    _list.push_back(std::make_pair(id, addr));
    return true;
}

template<class S>
inline bool AddressMap<S>::removeAddress(JAUS_ID id)
{
    // Wipe all entries with matching ids
    for (int i=0; i < _list.size(); i++)
        if (_list[i].first == id) _list[i].first = 0;
    return true;
}

template<class S>
inline bool AddressMap<S>::getAddrFromId(JAUS_ID id, S& addr)
{
    if (id == 0) return false;

    // Check for a match based on the JAUS_ID
    for (int i=0; i < _list.size(); i++)
    {
        if (_list[i].first == id)
        {
            addr = _list[i].second;
            return true;
        }
    }
    return false;
}

template<class S>
inline bool AddressMap<S>::getIdFromAddr( JAUS_ID& id, S addr )
{
   // Check for a match based on the addr
    for (int i=0; i < _list.size(); i++)
    {
        if ((_list[i].second == addr) && (_list[i].first != 0))
        {
            id = _list[i].first;
            return true;
        }
    }
    return false;
}
}} // namespace DeVivo::Junior
#endif



/*! 
 ***********************************************************************
 * @file      AddressMap.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __ADDRESS_MAP_H
#define __ADDRESS_MAP_H

#include <vector>

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
        if (_list[i].second == addr)
        {
            id = _list[i].first;
            return true;
        }
    }
    return false;
}
}} // namespace DeVivo::Junior
#endif



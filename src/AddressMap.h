// IPMap provides a mapping service between JAUS_ID and IP address (with port)
#ifndef __ADDRESS_MAP_H
#define __ADDRESS_MAP_H

#include <vector>

template<class S>
class AddressMap 
{
public:
    AddressMap():_list(){};
   ~AddressMap(){};

    // Define accessors
    bool getAddrFromId(JAUS_ID id, S& addr);
    bool getIdFromAddr( JAUS_ID& id, S addr);
    bool addAddress(JAUS_ID id, S addr);
    std::vector<std::pair<JAUS_ID, S> >& getList(){return _list;};
   
protected:

    std::vector<std::pair<JAUS_ID, S> > _list;
};

// define inlines
template<class S>
inline bool AddressMap<S>::addAddress(JAUS_ID id, S addr)
{
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
#endif



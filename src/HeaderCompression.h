// Provides header compression services 
#ifndef __HEADER_COMPRESSION_H
#define __HEADER_COMPRESSION__H

#include <map>

static unsigned char counter = 0;

//
// Define a class that represents an entry in the compression table
//
class CompressionTableEntry
{
    public:
        CompressionTableEntry():number(0), length(0), flags(0){}
       ~CompressionTableEntry(){};
        CompressionTableEntry(const CompressionTableEntry& in)
        {
            number = in.number;
            length = in.length;
            flags  = in.flags;
            archive = in.archive;
        }

        unsigned char number;
        unsigned char length;
        unsigned char flags;
        Archive       archive;
};

//
// Define a mapping of HC number to CompressionTableEntries as a 
// CompressionTable.  There is one CompressionTable per JAUS_ID, and
// this ID-to-CompressionTable is stored in another map.
//
typedef std::map<unsigned char, CompressionTableEntry> CompressionTable;
typedef std::map<unsigned char, CompressionTableEntry>::iterator 
                                                   CompressionTableIter;
typedef std::map<JAUS_ID, CompressionTable> CompressionTableMap;


//
// Define a class to manipulate the compression table
//
class HeaderCompressionTable 
{
public:
    HeaderCompressionTable():_map(){};
   ~HeaderCompressionTable(){};

    // Public functions to compress/uncompress
    bool compress( JAUS_ID id, JUDPArchive&   archive );
    bool uncompress( JAUS_ID id, JUDPArchive& archive );

    // Update an entry within the table
    void update( JAUS_ID id, JUDPArchive& msg );

protected:

    CompressionTableMap _map;

    // Find an entry within the table
    CompressionTableEntry* findEntry( JAUS_ID, JUDPArchive& );
    CompressionTableEntry* searchEntries( JAUS_ID, JUDPArchive& );

};


inline bool HeaderCompressionTable::compress( JAUS_ID id,
                                              JUDPArchive& archive )
{
    // check for mal-formed cases
    if (id == 0) return false;

    // Hunt through the table trying to find a match.
    CompressionTableEntry* entry = searchEntries( id, archive );
    if ((entry == NULL) || (entry->number == 0))
    {
        // Make sure we don't roll the counter
        if (counter >= 255) return false;

        // Didn't find a match, or no current compression. 
        // Propose compression of the JAUS header.
        //printf("Proposing new HC compression: %ld\n", counter+1);
        archive.setHCNumber( ++counter );
        archive.setHCLength( 14 );
        archive.setHCFlags( 1 );

        // New entry.  Create and populate a table entry
        update( id, archive );
    }
    else if (entry->flags == 1)
    {
        //printf("Found existing entry in HC table.  Reproposing...(%ld bytes)\n", entry->length);
        // Found an existing table entry but destination has not
        // yet acceptable compression.  Re-propose.
        archive.setHCNumber( entry->number );
        archive.setHCLength( entry->length );
        archive.setHCFlags( entry->flags );
    }
    else if ((entry->flags == 2) || (entry->flags == 3))
    {
        // Active compression.
        //printf("Compressing outgoing message with HC number %ld (length=%ld)\n",
        //          entry->number, entry->length);
        entry->flags = 3;
        archive.setHCNumber( entry->number );
        archive.setHCLength( entry->length );
        archive.setHCFlags( entry->flags );
        archive.removeAt( 5, entry->length );
    }

    return true;
}

inline bool HeaderCompressionTable::uncompress( JAUS_ID id,
                                                JUDPArchive& archive )
{
    // check for mal-formed cases
    if (id.val == 0) return false;

    // Find the entry for the given JAUS ID and HC number
    CompressionTableEntry* entry = findEntry( id, archive );
    if (entry == NULL)
    {
        // Unrecognized header compression.  This should return an error
        // eventually (not yet implemented)
        return false;
    }

    // Update the data flags for this entry
    entry->flags = 3;
   
    // Insert the HeaderCompression archive back into the incoming message
    archive.insertAt( 5, entry->archive );
    return true;
}


inline CompressionTableEntry* HeaderCompressionTable::searchEntries( 
                                               JAUS_ID id,
                                               JUDPArchive& archive )
{
    // check for mal-formed cases
    if (id.val == 0) return NULL;

    // See if we have a map for this JAUS ID
    if (_map.find( id ) == _map.end()) return NULL;

    // Search through all entries in the map, trying to find a match
    // for the given header.
    char* jausHdr = archive.getJausMsgPtr();
    for (CompressionTableIter iter = _map[id].begin();
         iter != _map[id].end(); ++iter)
    {
        if (memcmp( iter->second.archive.getArchive(), jausHdr, 
                    iter->second.archive.getArchiveLength()) == 0)
        {
            // Found a match
            return &(iter->second);
        }
    }

    // Getting here means we didn't find a match
    return NULL;
}

inline CompressionTableEntry* HeaderCompressionTable::findEntry( 
                                               JAUS_ID id,
                                               JUDPArchive& archive )
{
    // See if we have a map for this id
    if (_map.find( id ) == _map.end()) return NULL;

    // See if we have an entry for this id and HC number
    unsigned char num;
    archive.getHCNumber( num );
    if (_map[id].find(num) == _map[id].end()) return NULL;
 
    // return a pointer to the entry.
    return &(_map[id][num]);
}

inline void HeaderCompressionTable::update( JAUS_ID id,  JUDPArchive& msg )
{
    // Extract the HC values from the archive
    unsigned char flags, length, num;
    msg.getHCFlags( flags );
    msg.getHCLength( length );
    msg.getHCNumber( num );
    char* hdr_ptr = msg.getJausMsgPtr();

    // If we already have an entry, simply update the flags.
    if (( _map.find(id) != _map.end()) && _map[id].find( num )!=_map[id].end())
    {
        _map[id][num].flags  = flags;
        _map[id][num].length = length;
        return;
    }

    // New entry.  Create and populate a table entry
    CompressionTableEntry entry;
    entry.number = num;
    entry.length = length;
    entry.flags  = flags;
    entry.archive.setData( hdr_ptr, length );

    // Update the table with the new entry
    _map[id][num] = entry;
}

#endif



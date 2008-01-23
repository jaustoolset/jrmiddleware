//  Configuration Data class - maintains simple name=value pairings
#ifndef  __CONFIG_DATA_H
#define  __CONFIG_DATA_H

#include <map>
#include <fstream.h>
#include <string.h>

class ConfigData
{
public:
    ConfigData(): _map(){};
    ~ConfigData(){};

    //
    // Define an error enum
    //
    enum ConfigError {Ok, FileNotFound, InvalidFile, ValueNotFound};

    //
    // Functions to parse a config file
    //
    ConfigError parseFile( std::string filename );

    //
    // Functions to access a data item.  This is a templated function,
    // since we don't know the data type yet.
    //
    template <typename T> ConfigError getValue(std::string name, T& value)
    {
        if (_map.count(name) == 0)
            return ValueNotFound;

        value = (T) strtod(_map[name].c_str(), NULL);
        return Ok;
    }

protected:

    //
    // The main element is the mapping between name and value.
    // Both are stored as a string until the value is accessed.
    //
    std::map<std::string, std::string> _map;

    //
    // Internal function to parse a line as "name=value" and add it to the map
    //
    ConfigError parseLine( std::string line );

};

inline ConfigData::ConfigError ConfigData::parseFile( std::string filename )
{
    // Open the file for reading
    fstream instream;
    instream.open( filename.c_str() );
    if (instream.fail())
    {
        printf("Cannot open file: %s\n", filename.c_str());
        return InvalidFile;
    }

    // Parse line by line
    while (instream.good())
    {
        char buffer[80];
        instream.getline( buffer, 80);
        parseLine( std::string(buffer) );
    }

    // Close the file
    instream.close();
    return Ok;
}

inline ConfigData::ConfigError ConfigData::parseLine( std::string line )
{
    // Return immediately if the string is empty
    if (line.empty()) return Ok;

    // Make sure it's a valid name=value pair
    if (line.find("=") == std::string::npos)
    {
        printf("Cannot parse config file.  Line: %s\n", line.c_str());
        return InvalidFile;
    }

    // Break the line into a "name=value" pair
    std::string name, value;
    name = line.substr(0, line.find("="));
    value = line.substr(line.find("=")+1, std::string::npos);
    _map[name] = value;
    return Ok;
}
    
#endif



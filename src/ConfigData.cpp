//  Configuration Data class - maintains simple name=value pairings
#include "ConfigData.h"

ConfigData::ConfigError ConfigData::parseFile( std::string filename )
{
    // Open the file for reading
    ifstream instream;
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

ConfigData::ConfigError ConfigData::parseLine( std::string line )
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

    // clean-up the strings before we add it to the map
    deleteWhitespace(name);
    deleteWhitespace(value);
    _map[name] = value;
    return Ok;
}

void ConfigData::deleteWhitespace(std::string& in)
{
    in = in.substr(in.find_first_not_of(" "), in.find_last_not_of(" ")+1);
}
    


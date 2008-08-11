/*! 
 ***********************************************************************
 * @file      ConfigData.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "ConfigData.h"
#include "JrLogger.h"

using namespace DeVivo::Junior;

ConfigData::ConfigError ConfigData::parseFile( std::string filename )
{
    if (filename.empty()) return InvalidFile;

    // Open the file for reading
    ifstream instream;
    instream.open( filename.c_str() );
    if (instream.fail())
    {
        JrWarn << "Cannot open configuration file: " << filename << std::endl;
        return InvalidFile;
    }

    // Parse line by line
    while (instream.good())
    {
        char buffer[255];
        instream.getline( buffer, 255);
        parseLine( std::string(buffer) );
    }

    // Close the file
    instream.close();
    return Ok;
}

ConfigData::ConfigError ConfigData::parseLine( std::string line )
{
    // Return immediately if the string is too small
    deleteWhitespace(line);
    if ((line.size()) < 3) return Ok;

    // Check for comment lines
    if (line[0]=='#') return Ok;

    // Make sure it's a valid name=value pair
    if (line.find("=") == std::string::npos)
    {
        JrWarn << "Cannot parse config file.  Line: " << line << std::endl;
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
    if (in.empty()) return;
    in = in.substr(in.find_first_not_of(" "), in.find_last_not_of(" ")+1);
}

// 
// Function to get a list of keys from the map
//
void ConfigData::getKeyList(StringList& list)
{
    // clear the list
    list.clear();

    // Loop through the map, pulling each key
    std::map<std::string, std::string>::iterator iter;
    for (iter = _map.begin(); iter != _map.end(); iter++)
        list.push_back(iter->first);
}

    



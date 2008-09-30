/*! 
 ***********************************************************************
 * @file      ConfigData.cpp
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

    



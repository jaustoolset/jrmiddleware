/*! 
 ***********************************************************************
 * @file      ConfigData.h
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
#ifndef  __CONFIG_DATA_H
#define  __CONFIG_DATA_H

#include <list>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include "JrLogger.h"

using namespace std;
namespace DeVivo {
namespace Junior {

// Define a list of strings so we can report the
// map keys back to the requestor
typedef std::list<std::string> StringList;
typedef std::list<std::string>::iterator StringListIter;


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
    // Special case for string handling
    //
    ConfigError getValue(std::string name, std::string& value)
    {
        if (_map.count(name) == 0)
		{
			JrWarn << "Failed to find configuration item: " << name << std::endl;
			return ValueNotFound;
		}
        value = _map[name];
		stripExtraChars(value);
		return Ok;
	}

	void stripExtraChars(std::string& value)
	{
        // strip line-feed, carriage return, and quote marks values
        value = value.substr(0, value.find_last_not_of("\n")+1);
        value = value.substr(0, value.find_last_not_of("\r")+1);
        value = value.substr(0, value.find_last_not_of("\"")+1);
        value = value.substr(value.find_first_not_of("\""));
    }

    //
    // Functions to access a data item.  This is a templated function,
    // since we don't know the data type yet.
    //
    template <typename T> ConfigError getValue(std::string name, T& value)
    {
        if (_map.count(name) == 0)
		{
			JrWarn << "Failed to find configuration item: " << name << std::endl;
			return ValueNotFound;
		}
        value = (T) strtod(_map[name].c_str(), NULL);
        return Ok;
    }

    // 
    // Function to get a list of keys from the map
    //
    void getKeyList(StringList& list);

protected:

    // Helper functions
    void deleteWhitespace(std::string& in);

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
}} // namespace DeVivo::Junior
#endif



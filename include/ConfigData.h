/*! 
 ***********************************************************************
 * @file      ConfigData.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef  __CONFIG_DATA_H
#define  __CONFIG_DATA_H

#include <map>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
namespace DeVivo {
namespace Junior {

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
        if (_map.count(name) == 0) return ValueNotFound;
        value = _map[name];

        // strip line-feed and carriage return values
        value = value.substr(0, value.find_last_not_of("\n")+1);
        value = value.substr(0, value.find_last_not_of("\r")+1);
        return Ok;
    }

    //
    // Functions to access a data item.  This is a templated function,
    // since we don't know the data type yet.
    //
    template <typename T> ConfigError getValue(std::string name, T& value)
    {
        if (_map.count(name) == 0) return ValueNotFound;
        value = (T) strtod(_map[name].c_str(), NULL);
        return Ok;
    }

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



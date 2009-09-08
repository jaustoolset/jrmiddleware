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

#include "Types.h"

namespace DeVivo {
namespace Junior {

//
// This is a default implementation that does nothing.  It is 
// not abstract, however, since we can use the default implementation
// to implicitly use pre-compile defaults.
//
class ConfigData
{
public:
	ConfigData(){};
    ~ConfigData(){};

    //
    // Define an error enum
    //
    enum ConfigError {Ok, FileNotFound, InvalidFile, ValueNotFound};

    //
    // Functions to parse a config file
    //
	virtual ConfigError parseFile( std::string filename ){return Ok;};

	//
	// Access an atribute of an element.  An optional
	// index can be supplied to manage duplicate elements.
	//
	virtual ConfigError getValue(std::string& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};
	virtual ConfigError getValue(int& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};
	virtual ConfigError getValue(unsigned int& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};
	virtual ConfigError getValue(short& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};
	virtual ConfigError getValue(unsigned short& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};
	virtual ConfigError getValue(char& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};
	virtual ConfigError getValue(unsigned char& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0){return ValueNotFound;};

	// Get a list of attributes for a given element
	virtual StringList getAttributes(std::string element){return StringList();};

protected:

};

}} // namespace DeVivo::Junior
#endif



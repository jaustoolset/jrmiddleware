/*! 
 ***********************************************************************
 * @file      XmlConfig.h
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
#ifndef  __XML_CONFIG_DATA_H
#define  __XML_CONFIG_DATA_H

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "ConfigData.h"
#include "tinyxml.h"
#include "JrLogger.h"
#include "Types.h"

namespace DeVivo {
namespace Junior {


// We use TinyXML for parsing the configure file into a DOM.
// The XmlConfig class provides abstraction from the DOM
// and XML, in case we switch to a different solution later.
class XmlConfig : public ConfigData
{
public:
	XmlConfig(){};
    ~XmlConfig(){};

    //
    // Functions to parse a config file
    //
	virtual ConfigError parseFile(const std::string& filename );

	//
	// Access an atribute of an element.  An optional
	// index can be supplied to manage duplicate elements.
	//
	virtual ConfigError getValue(std::string& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);
	virtual ConfigError getValue(int& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);
	virtual ConfigError getValue(unsigned int& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);
	virtual ConfigError getValue(short& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);
	virtual ConfigError getValue(unsigned short& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);
	virtual ConfigError getValue(char& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);
	virtual ConfigError getValue(unsigned char& value,
			  					 const std::string& attribute,
								 const std::string& element,
								 int index = 0);

	// Get a list of attributed for a given element
	virtual StringList getAttributes(std::string element);

protected:

    TiXmlDocument doc;

	// Templated function to access DOM
	template <typename T> ConfigData::ConfigError lookupValue(T& value,
											   const std::string& attribute,
											   const std::string& element,
											   int index = 0);

};

}} // namespace DeVivo::Junior
#endif



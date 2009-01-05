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

#include "tinyxml.h"
#include "JrLogger.h"
#include "Types.h"

namespace DeVivo {
namespace Junior {


// We use TinyXML for parsing the configure file into a DOM.
// The ConfigData class provides abstraction from the DOM
// and XML, in case we switch to a different solution later.
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
    ConfigError parseFile( std::string filename );

	//
	// Access an atribute of an element.  An optional
	// index can be supplied to manage duplicate elements.
	//
	template <typename T> ConfigError getValue(T& value,
											   std::string attribute,
											   std::string element,
											   int index = 0);

	// Get a list of attributed for a given element
	StringList getAttributes(std::string element);

protected:

    TiXmlDocument doc;

};


inline ConfigData::ConfigError ConfigData::parseFile( std::string filename )
{
	doc.LoadFile(filename.c_str());
	if (doc.Error())
	{
		JrError << "Failed to parse config file: " << filename << std::endl;

		// Cast the TinyXML errors to our enum
		if (doc.ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE) 
			return FileNotFound;
		return InvalidFile;
	}
	return Ok;
}

template <typename T> 
inline ConfigData::ConfigError ConfigData::getValue(T& value,
													std::string attribute,
													std::string element,
													int index)

{
	// Get the first occurrence of the requested element
	TiXmlHandle docHandle( &doc );
	TiXmlElement* ele = docHandle.FirstChild("JrConfigData").FirstChild(element.c_str()).ToElement();
	if (ele == NULL) 
	{
		JrWarn << "Failed to find configuration element: " << element << "\n";
		return ValueNotFound;
	}

	// Loop through the index values to find the requested element number
	while ( index > 0 )
	{
		ele = ele->NextSiblingElement(element.c_str());
		if (ele == NULL)
		{
			JrWarn<<"Failed to find configuration element: "<<element <<"\n";
			return ValueNotFound;
		}
		index--;
	}

	// Now that we have the right element, pull the requested attribute.
	if (ele->QueryValueAttribute(attribute.c_str(), &value) != TIXML_SUCCESS)
	{
		JrWarn << "Failed to find configuration attribute: " << attribute<<"\n";
		return ValueNotFound;
	}

	// Success!
	JrDebug << "Found config value: " << attribute << " = " << value << std::endl;
	return Ok;
}


inline StringList ConfigData::getAttributes(std::string element)
{
	StringList ret;

	// Get the first occurrence of the requested element
	TiXmlHandle docHandle( &doc );
	TiXmlElement* ele = docHandle.FirstChild("JrConfigData").FirstChild(element.c_str()).ToElement();
	if (ele == NULL) return ret; 

	// Walk through the attributes, returning a string for each
	for (TiXmlAttribute* att = ele->FirstAttribute(); att != NULL; att = att->Next())
	{
		//JrDebug << "Found attribute: " << att->Name() << "\n";
		ret.push_back(att->Name());
	}
	return ret;
}


}} // namespace DeVivo::Junior
#endif



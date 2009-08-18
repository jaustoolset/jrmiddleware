/*! 
 ***********************************************************************
 * @file      XmlConfig.cpp
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
#include "XmlConfig.h"
using namespace DeVivo::Junior;


ConfigData::ConfigError XmlConfig::parseFile( std::string filename )
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

// getValue implementations make use of templated accessor
ConfigData::ConfigError XmlConfig::getValue(std::string value,
			  					 std::string attribute,
								 std::string element,
								 int index)
{
	return lookupValue(value, attribute, element, index);
}

ConfigData::ConfigError XmlConfig::getValue(int value,
			  					 std::string attribute,
								 std::string element,
								 int index)
{
	return lookupValue(value, attribute, element, index);
}

template <typename T> 
ConfigData::ConfigError XmlConfig::lookupValue(T& value,
											   std::string attribute,
											   std::string element,
											   int index)

{
	// Get the first occurrence of the requested element
	TiXmlHandle docHandle( &doc );
	TiXmlElement* ele = docHandle.FirstChild("JrXmlConfig").FirstChild(element.c_str()).ToElement();
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


StringList XmlConfig::getAttributes(std::string element)
{
	StringList ret;

	// Get the first occurrence of the requested element
	TiXmlHandle docHandle( &doc );
	TiXmlElement* ele = docHandle.FirstChild("JrXmlConfig").FirstChild(element.c_str()).ToElement();
	if (ele == NULL) return ret; 

	// Walk through the attributes, returning a string for each
	for (TiXmlAttribute* att = ele->FirstAttribute(); att != NULL; att = att->Next())
	{
		JrDebug << "Found attribute: " << att->Name() << "\n";
		ret.push_back(att->Name());
	}
	return ret;
}

/*! 
 ***********************************************************************
 * @file      JrLogger.cpp
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

#include "JrLogger.h"

using namespace std;
using namespace DeVivo::Junior;

//
// Return a reference to the active stream.
//
ostream& Logger::getStream(enum LogMsgType type)
{
    // Check the message type against the current level.
    // If the message type is too high for the current level,
    // return a closed stream (no output).
    if (type > _level) return nullstream;

    // If the file stream is not open, just use standard out
    return (filestream.is_open() ? filestream : std::cout);
}

//
// Start a new enty on the active stream
//
ostream& Logger::startMsg(std::string filename, int line, enum LogMsgType type)
{
    // get the desired stream
    ostream& stream = getStream(type);

    // flush any previous messages
    stream.flush();

    // Insert formatted text
    stream << "(" << filename << ", line " << line << ") " << enum2Str(type) << ": ";
    return stream;
}

//
// helper function to convert the numberation to a string literal
//
std::string Logger::enum2Str(enum LogMsgType type)
{
    if (type==error) return "ERROR";
    if (type==info) return "INFO";
    if (type==warning) return "WARNING";
    if (type==debug) return "DEBUG";
    if (type==full) return "FULL";
    return "UNKNOWN";
}

//
// Open the given log file for output
//
void Logger::openOutputFile(std::string filename)
{
    // If the stream is current open, close it.
    closeOutputFile();

    // Open the given filename
    filestream.open(filename.c_str(), fstream::out | fstream::app);
    if (!filestream.is_open()) 
        JrError << "Unable to open log file: " << filename << std::endl;
}

// 
// closes the log file.  Subsequent log outputs
// will be forced to standard out.
//
void Logger::closeOutputFile()
{
    if (filestream.is_open())
    {
        filestream.flush();
        filestream.close();
    }
}




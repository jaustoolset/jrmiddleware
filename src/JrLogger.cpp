/*! 
 ***********************************************************************
 * @file      JrLogger.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
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




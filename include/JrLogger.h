/*! 
 ***********************************************************************
 * @file      JrLogger.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef  __JR_LOGGER_H
#define  __JR_LOGGER_H

#include <string>
#include <fstream>
#include <iostream>


// USE THESE MACROS FOR ALL LOG OUTPUT
#define JrError (Logger::get()->startMsg(__FILE__, __LINE__, Logger::error))
#define JrInfo (Logger::get()->startMsg(__FILE__, __LINE__, Logger::info))
#define JrWarn (Logger::get()->startMsg(__FILE__, __LINE__, Logger::warning))
#define JrDebug (Logger::get()->startMsg(__FILE__, __LINE__, Logger::debug))
#define JrFull (Logger::get()->startMsg(__FILE__, __LINE__, Logger::full))

using namespace std;
namespace DeVivo {
namespace Junior {

class Logger
{
public:
    Logger(){_level=none;}
    ~Logger(){closeOutputFile();}

    // Define enumeration for debug levels
    enum LogMsgType {none = 0, error, info, warning, debug, full};
    std::string enum2Str(enum LogMsgType type);

    // Logger is a static class (one per process).  We 
    // supply a function to get the only instance.
    static Logger* get();

    // Functions for getting the active stream
    ostream& getStream(enum LogMsgType type);

    // Function to start a new log entry.  This 
    // return the stream, but also inserts standard debugging text
    ostream& startMsg(std::string filename, int line, enum LogMsgType type);

    // Functions to change the debug level
    enum LogMsgType getMsgLevel(){return _level;}
    void setMsgLevel(enum LogMsgType level){_level = level;}

    // Functions to open and close the output file
    void openOutputFile(std::string filename);
    void closeOutputFile();

protected:

    // Current debug level
    enum LogMsgType _level;

    // We use both a real file stream and a "null stream"
    // that doesn't output anything
    fstream filestream, nullstream;
};


// Define an inlines to get the static Logger object
inline Logger* Logger::get()
{
    static Logger logger;
    return &logger;
}

}} // namespace DeVivo::Junior
#endif



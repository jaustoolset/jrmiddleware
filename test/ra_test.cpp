/*! 
 ***********************************************************************
 * @file      jr_test.cpp
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
#include "JuniorRA.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <signal.h>
#include "OS.h"

const int MaxBufferSize = 50000;

using namespace DeVivo::Junior;

// Define a signal handler, so we can clean-up properly
static int exit_flag = 0;
static void handle_exit_signal( int signum )
{
    exit_flag = 1;
}

// Define a sleep function 
#ifndef WINDOWS
void Sleep(unsigned long milliseconds)
{
    usleep(milliseconds * 1000);
}
#endif

// Define the path name for the config file
#ifdef WINDOWS
#define CONFIG_PATH_NAME "c:/windows/system32/"
#else
#define CONFIG_PATH_NAME "/usr/bin/"
#endif
#undef CONFIG_PATH_NAME
#define CONFIG_PATH_NAME ""

int main(int argc, char* argv[])
{
    int counter = 0;

    // Make sure an id is specified
    if (argc < 2)
    {
        printf("usage: ocu <my id> <dest id: optional>\n");
        return 1;
    }

    // Get the local id from the command line
    unsigned long myid;
    std::stringstream s; s << argv[1];
    s >> myid;

    // Get the command line argument for sending a message
    unsigned long dest = 0;
    if (argc > 2)
    {
        std::stringstream t; t << argv[2];
        t >> dest;
    }

    // Connect to the Run-Time Engine
    int handle;
    std::string file_path = std::string(CONFIG_PATH_NAME) + 
                            std::string("junior.cfg");
    if (JrConnect(myid, file_path.c_str(), &handle) != Ok)
    {
        printf("Init failed.  Terminating execution\n");
        return -1;
    }

    // Catch the termination signals
    signal( SIGINT, handle_exit_signal );
    signal( SIGTERM, handle_exit_signal );
    signal( SIGABRT, handle_exit_signal );

    // Make a data buffer for incoming/outgoing messages.
    char buffer[MaxBufferSize];
    unsigned short datasize = 2016;

    // Manually create the message to be sent
    buffer[0] = (char) 6; // priority
    buffer[1] = (char) 2; // version
    *(unsigned short*) &buffer[2] = (unsigned short) 0x4502;
    *(unsigned long*) &buffer[4] = (unsigned long) dest;
    *(unsigned long*) &buffer[8] = (unsigned long) myid;
    *(unsigned short*) &buffer[12] = (unsigned short) 0;
    *(unsigned short*) &buffer[14] = (unsigned short) 0x6212;

    // Do stuff
    while(!exit_flag)
    {
        if (dest != 0)
        {
            // Send the message
            printf("Sending message %ld (size=%ld)\n", ++counter, datasize);
            JrErrorCode result = RaSend(handle, datasize, buffer);
            if ( result != Ok) printf("Sendto failed (%d)\n", result);
        }

        // check for incoming messages
        for (int i=0; i<100; i++)
        {
            unsigned int buffersize = MaxBufferSize;
            JrErrorCode ret = RaReceive(handle, &buffersize, buffer);
            if (ret == Ok)
            {
                // Print the message
                printf("Message received (size=%ld): ", buffersize);
                int psize = (buffersize > 16) ? 16 : buffersize;
                for (int i=0; i < psize; i++)
                    printf(" %x", (char)buffer[i]);
                printf("\n");
            }               

            Sleep(1);
        }
    }

    // clean-up
    JrDisconnect(handle);
}

  

/***********           LICENSE HEADER   *******************************
JR Middleware
Copyright (c)  2008-2019, DeVivo AST, Inc
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

       Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

       Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.

       Neither the name of the copyright holder nor the names of 
its contributors may be used to endorse or promote products derived from 
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*********************  END OF LICENSE ***********************************/
#include "JuniorAPI.h"
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

// Define a time stamp function
unsigned long GetTimestamp()
{
#ifdef WINDOWS
    return (unsigned long)(GetTickCount());
#else
    struct timeval tv; struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec*1000 + (unsigned long)(tv.tv_usec/1000));
#endif
}


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
    // Make sure an id is specified
    if (argc < 2)
    {
        printf("usage: ocu <my id> <dest id: optional> <msg size: optional>\n");
        return 1;
    }

    // Get the local id from the command line
    unsigned int myid;
    std::stringstream s; s << argv[1];
    s >> myid;

    // Get the command line argument for sending a message
    unsigned int dest = 0;
    if (argc > 2)
    {
        std::stringstream t; t << argv[2];
        t >> dest;
    }

    // Get the command line argument for message size
    unsigned int size = 0;
    if (argc > 3)
    {
        std::stringstream u; u << argv[3];
        u >> size;
		if (size < 10) size = 10;
    }


    // Connect to the Run-Time Engine
    long handle;
    std::string file_path = std::string(CONFIG_PATH_NAME) + 
                            std::string("jr_config.xml");
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
    int counter = 0;         
    unsigned int sender;
    unsigned int datasize = size;
    int prevMsg  =0;

    // Randomize
    srand(GetTimestamp());

    // Do stuff
    while(!exit_flag)
    {
        // Create a random message size (unless command line arguments
        // indicate something else
        if (size == 0)
        {
            do
            {
                datasize = (unsigned int) rand();
            } while ((datasize > MaxBufferSize) || (datasize < 10));
        }

        // Assign it a random message id.  Since we have to test both AS5669
		// and AS5669A, make half the messages without a message code.
        unsigned short msg_id = (unsigned short) rand();
		if ((msg_id % 2) == 0) msg_id = 0;

        if (dest != 0)
        {
            // Send a message of the given size, with a counter and size element
			unsigned short packedId =htons(msg_id);
            int packedCounter = htonl(++counter);
            unsigned int packedSize = htonl(datasize);
			memcpy( &buffer[0], (void*) &packedId, 2);
			memcpy( &buffer[2], (void*) &packedCounter, 4);
			memcpy( &buffer[6], (void*) &packedSize, 4);

            //if ((counter % 500) == 0)
                printf("Sending message %ld (id=%ld, size=%ld)\n", counter, msg_id, datasize);
            JrErrorCode result = JrSend(handle, dest, datasize, buffer, 6, ExperimentalFlag, msg_id);
            if ( result != Ok)
                printf("Sendto failed (%d)\n", result);
        }

        // check for incoming messages
        for (int i=0; i<500; i++)
        {
            unsigned int buffersize = MaxBufferSize; msg_id = 0; int flags = 0;
            JrErrorCode ret = JrReceive(handle, &sender, &buffersize, buffer, NULL, &flags, &msg_id);
            if (ret == Ok)
            {
                // Pull off the data that was embedded in teh message.
		        unsigned short id; memcpy( &id, &buffer[0], 2); id = ntohs(id);
                int msgcount; memcpy( &msgcount, &buffer[2], 4); msgcount = ntohl(msgcount);
                unsigned int size; memcpy( &size, &buffer[6], 4); size = htonl(size);
                if (size != buffersize) printf("WARNING: SIZE INCONSISTENT (msg=%ld, buffer=%ld)\n", size, buffersize);
                if (id != msg_id) printf("WARNING: ID INCONSISTENT (msg=%ld, buffer=%ld)\n", msg_id, id);
                if ((prevMsg+1) != msgcount) printf("WARNING: Messages not in sequence (prev=%ld, this=%ld)\n", prevMsg, msgcount);
                //if ((msgcount % 500) == 0)
                    printf("Incoming Msg: Sender = %ld, Count = %ld, ID = %ld, Size = %ld, Flags = %ld\n", sender, msgcount, msg_id, buffersize, flags);
                prevMsg = msgcount;
            }  

            // check for exit condition
            if (exit_flag) break;

            // Sleep a bit before calling receive again
            Sleep(1);
        }
    }

    // clean-up
    JrDisconnect(handle);
}

  

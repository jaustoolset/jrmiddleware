/*! 
 ***********************************************************************
 * @file      jr_test.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "JuniorAPI_v1.h"
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


int main(int argc, char* argv[])
{
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
    if (JrConnect(myid, "junior.cfg", &handle) != Ok)
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
    unsigned long sender;
    unsigned int datasize;
    int prevMsg  =0;

    // Randomize
    srand(rand());

    // Do stuff
    while(!exit_flag)
    {
        // Create a random message size.
        do
        {
            datasize = (unsigned int) rand();
        } while ((datasize > MaxBufferSize) || (datasize < 10));

        // Assign it a random message id
        unsigned short msg_id = (unsigned short) rand();

        if (dest != 0)
        {
            // Send a message of the given size, with a counter and size element
            *((int*)buffer) = htonl(++counter);
            *((unsigned int*) &buffer[4]) = htonl(datasize);
            *((unsigned short*) &buffer[8]) = htons(msg_id);

            //if ((counter % 500) == 0)
                printf("Sending message %ld (id=%ld, size=%ld)\n", counter, msg_id, datasize);
            JrErrorCode result = JrSend(handle, dest, msg_id, datasize, buffer, 6, ExperimentalFlag);
            if ( result != Ok)
                printf("Sendto failed (%d)\n", result);
        }

        // check for incoming messages
        for (int i=0; i<500; i++)
        {
            unsigned int buffersize = MaxBufferSize; msg_id = 0; int flags = 0;
            JrErrorCode ret = JrReceive(handle, &sender, &msg_id, &buffersize, buffer, NULL, &flags);
            if (ret == Ok)
            {
                // Pull off the data that was embedded in teh message.
                int msgcount = ntohl(*((int*) buffer));
                unsigned int size = ntohl(*((unsigned int*) &buffer[4]));
                unsigned short id = ntohs(*((unsigned short*) &buffer[8]));
                if (size != buffersize) printf("WARNING: SIZE INCONSISTENT (msg=%ld, buffer=%ld)\n", size, buffersize);
                if (id != msg_id) printf("WARNING: ID INCONSISTENT (msg=%ld, buffer=%ld)\n", msg_id, id);
                if ((prevMsg+1) != msgcount) printf("WARNING: Messages not in sequence (prev=%ld, this=%ld)\n", prevMsg, msgcount);
                //if ((msgcount % 500) == 0)
                    printf("Incoming Msg: Sender = %ld, Count = %ld, ID = %ld, Size = %ld, Flags = %ld\n", sender, msgcount, msg_id, buffersize, flags);
                prevMsg = msgcount;
            }               

            Sleep(1);
        }
    }

    // clean-up
    JrDisconnect(handle);
}

  

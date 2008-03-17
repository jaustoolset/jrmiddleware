/*! 
 ***********************************************************************
 * @file      OCU.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#define JR_SEND_MESSAGE_ID
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

int main(int argc, char* argv[])
{
    // If no arguments are specified, we can randomly select an ID and don't
    // specified a destination.
    unsigned long myid = JrGetTimestamp();
    if (argc > 1) 
    {
        std::stringstream s; s << argv[1];
        s >> myid;
    }

    // Get the command line argument for sending a message
    unsigned long dest = 0;
    if (argc > 2)
    {
        std::stringstream t; t << argv[2];
        t >> dest;
    }

    // Connect to the Run-Time Engine
    int handle;
    if (connect(myid, "junior.cfg", &handle) != Ok)
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
    unsigned short datasize;
    int prevMsg  =0;

    // Randomize
    srand(JrGetTimestamp());

    // Do stuff
    while(!exit_flag)
    {
        // Create a random message size.
        do
        {
            datasize = (unsigned short) rand();
        } while ((datasize > MaxBufferSize) || (datasize < 8));

        // Assign it a random message id
        unsigned short msg_id = (unsigned short) rand();

        if (dest != 0)
        {
            // Send a message of the given size, with a counter and size element
            *((int*)buffer) = ++counter;
            *((unsigned short*) &buffer[4]) = datasize;
            *((unsigned short*) &buffer[6]) = msg_id;

            //if ((counter % 500) == 0)
                printf("Sending message %ld (id=%ld, size=%ld)\n", counter, msg_id, datasize);
            JrErrorCode result = sendto(handle, dest, msg_id, datasize, buffer, 6, 0);
            if ( result != Ok)
                printf("Sendto failed (%d)\n", result);
        }

        // check for incoming messages
        for (int i=0; i<500; i++)
        {
            unsigned int buffersize = MaxBufferSize; msg_id = 0;
            JrErrorCode ret = recvfrom(handle, &sender, &msg_id, &buffersize, buffer, NULL);
            if (ret == Ok)
            {
                // Pull off the data that was embedded in teh message.
                int msgcount = *((int*) buffer);
                unsigned short size = *((unsigned short*) &buffer[4]);
                unsigned short id = *((unsigned short*) &buffer[6]);
                if (size != buffersize) printf("WARNING: SIZE INCONSISTENT (msg=%ld, buffer=%ld)\n", size, buffersize);
                if (id != msg_id) printf("WARNING: ID INCONSISTENT (msg=%ld, buffer=%ld)\n", msg_id, id);
                if ((prevMsg+1) != msgcount) printf("WARNING: Messages not in sequence (prev=%ld, this=%ld)\n", prevMsg, msgcount);
                //if ((msgcount % 500) == 0)
                    printf("Incoming Msg: Sender = %ld, Count = %ld, ID = %ld, Size = %ld)\n", sender, msgcount, msg_id, buffersize);
                prevMsg = msgcount;
            }               

            JrSleep(1);
        }
    }

    // clean-up
    disconnect(handle);
}

  

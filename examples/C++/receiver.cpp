/*! 
 ***********************************************************************
 * @file      receiver.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#include "JuniorAPI.h"
#include <list>
#include <string>

// Define some message types
const unsigned short QueryId   = 1;
const unsigned short ReportId  = 2;
const unsigned short MsgString = 3;

// Define the maximum message size we can support
const unsigned short MaxMsgSize = 100;

//
// The "receiver" program listens for ID requests, and prints
// incoming messages.
//
int main(int argc, char* argv[])
{
    JrErrorCode ret;
    char buffer[MaxMsgSize]; 
    unsigned int buffersize;
    unsigned int sender;
    unsigned short msg_id;

    // Assign a random id
    srand((unsigned int)(time(0)));
    unsigned int myid = (unsigned int) rand();

    // Initiate a connection to the Junior Run-Time Engine.
    // We need to use the returned handle in all subsequent calls.
    int handle;
    if (JrConnect(myid, NULL, &handle) != Ok)
    {
        printf("Init failed.  Terminating execution\n");
        return -1;
    }

    // Every loop, handle incoming messages.
    while(1)
    {
        buffersize = MaxMsgSize;
        ret = JrReceive(handle, &sender, &buffersize, buffer, 
                        NULL, NULL, &msg_id);
        if (ret == Ok)
        {
            // Handle different message types differently.
            if (msg_id == QueryId)
            {
                // Respond with a ReportId
                JrSend(handle, sender, 0, buffer, 6, 0, ReportId);
            }
            else if (msg_id == MsgString)
            {
                // Print the string
                printf("%s\n", std::string(buffer, buffersize).c_str());
            }
            else
            {
                printf("Unknown message type received.\n");
            }
        }

    // Sleep a bit before looping again
    usleep(1000);
    }
}

  

/*! 
 ***********************************************************************
 * @file      sender.cpp
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
#include <list>
#include <algorithm>

// Define some message types
const unsigned short QueryId   = 1;
const unsigned short ReportId  = 2;
const unsigned short MsgString = 3;

// Define the maximum message size we can support
const unsigned short MaxMsgSize = 100;

//
// The "sender" program transmits a message to every known recipient.
//
int main(int argc, char* argv[])
{
    JrErrorCode ret;
    char buffer[MaxMsgSize]; 
    unsigned int buffersize;
    unsigned long sender;
    unsigned short msg_id;

    // Assign a random id
    srand((unsigned long)(time(0)));
    unsigned long myid = (unsigned long) rand();

    // Initiate a connection to the Junior Run-Time Engine.
    // We need to use the returned handle in all subsequent calls.
    int handle;
    if (connect(myid, "junior.cfg", &handle) != Ok)
    {
        printf("Init failed.  Terminating execution\n");
        return -1;
    }

    // Create an list for known destinations
    std::list<unsigned long> destinations;
    std::list<unsigned long>::iterator iter;

    // Every loop, check for incoming messages before broadcasting
    // a search for new destinations.  Then send a message to
    // every known destination.
    while(1)
    {
        // Check for incoming messages
        do
        {
            buffersize = MaxMsgSize;
            ret = recvfrom(handle, &sender, &msg_id, &buffersize, buffer, NULL);
            if ((ret == Ok)  && (msg_id == ReportId))
            {
                // If this sender is not in our list of destinations, add it.
                if (std::find(destinations.begin(), destinations.end(), sender) == destinations.end())
                {
                    printf("Adding new destination: %ld\n", sender);
                    destinations.push_back(sender);
                }
            }
        } while (ret == Ok);

        // Broadcast a query for ids
        broadcast(handle, QueryId, 0, buffer, 6);

        // Send a message to each known destination
        for (iter = destinations.begin(); iter != destinations.end(); iter++)
        {
            sprintf(buffer, "Message sent from %ld\0", myid);
            sendto(handle, *iter, MsgString, strlen(buffer), buffer, 6, 0);
        }

    // Sleep a bit before looping again
    usleep(5000000);
    }
}

  

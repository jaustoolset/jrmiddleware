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
    unsigned int sender;
    unsigned short msg_id;

    // Assign a random id
    srand((unsigned int)(time(0)));
    unsigned int myid = (unsigned int) rand();

	// Use config file, if specified
	const char* config_file = NULL;
	if (argc >= 2) config_file = argv[1];

    // Initiate a connection to the Junior Run-Time Engine.
    // We need to use the returned handle in all subsequent calls.
    long handle;
    if (JrConnect(myid, config_file, &handle) != Ok)
    {
        printf("Init failed.  Terminating execution\n");
        return -1;
    }

    // Create an list for known destinations
    std::list<unsigned int> destinations;
    std::list<unsigned int>::iterator iter;

    // Every loop, check for incoming messages before broadcasting
    // a search for new destinations.  Then send a message to
    // every known destination.
    while(1)
    {
        // Check for incoming messages
        do
        {
            buffersize = MaxMsgSize;
            ret = JrReceive(handle, &sender, &buffersize, buffer, 
                            NULL, NULL, &msg_id);
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
        JrBroadcast(handle, 0, buffer, 6, QueryId);

        // Send a message to each known destination
        for (iter = destinations.begin(); iter != destinations.end(); iter++)
        {
            sprintf(buffer, "Message sent from %ld\0", myid);
            JrSend(handle, *iter, strlen(buffer), buffer, 6, 0, MsgString);
        }

    // Sleep a bit before looping again
    usleep(5000000);
    }
}

  

/*! 
 ***********************************************************************
 * @file      sender.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 *  This file is part of Jr Middleware.
 *
 *  Jr Middleware is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Jr Middleware is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Jr Middleware.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************
 */
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

  

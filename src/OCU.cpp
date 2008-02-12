//  DEVIVO HEADER GOES HERE!!!

// The following file represents the 'main' application for the 
// OCU portion of the JAUS Guide Demonstration.
#include "JuniorAPI.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include "OS.h"

const int MaxBufferSize = 50000;

int main(int argc, char* argv[])
{
    // If no arguments are specified, we can randomly select an ID and don't
    // specified a destination.
    unsigned long myid = JrRandomValue();
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
    if (connect(myid, &handle) != Ok)
    {
        printf("Init failed.  Terminating execution\n");
        return -1;
    }

    // Make a data buffer for incoming/outgoing messages.
    char buffer[MaxBufferSize];
    int counter = 0;         
    unsigned long sender;
    unsigned short datasize;

    // Broadcast a message, announcing our availability.
    //broadcast(handle, 0, buffer, 15);

    // Do stuff
    while(1)
    {
        // Create a random message size.
        srand(JrRandomValue());
        do
        {
            datasize = (unsigned short) rand();
        } while (datasize > MaxBufferSize);

        if (dest != 0)
        {
//            sprintf(buffer, "This is message %d.\0", ++counter);
//            if (int result = sendto(handle, dest, strlen(buffer), buffer, 6, 1) != 0)
//                printf("Sendto failed (%d)\n", result);
            
//            sprintf(buffer, "Urgent broadcast %d.\0", ++counter);
//            broadcast(handle, strlen(buffer), buffer, 15);


            // Send a message of the given size, with a counter and size element
            // included
            *((int*)buffer) = ++counter;
            *((unsigned short*) &buffer[4]) = datasize;
            printf("Sending message %ld (size=%ld)\n", counter, datasize);
            JrErrorCode result = sendto(handle, dest, datasize, buffer, 6, 1);
            if ( result != Ok)
                printf("Sendto failed (%d)\n", result);
        }

        // check for incoming messages
        for (int i=0; i<10; i++)
        {
            unsigned int buffersize = MaxBufferSize;
            JrErrorCode ret = recvfrom(handle, &sender, &buffersize, buffer, NULL);
            if (ret == Ok)
            {
                // Pull off the data that was embedded in teh message.
                int msgcount = *((int*) buffer);
                unsigned short size = *((unsigned short*) &buffer[4]);
                if (size != buffersize) printf("WARNING: SIZE INCONSISTENT (msg=%ld, buffer=%ld)\n", size, buffersize);
                printf("Incoming Msg: Sender = %ld, Count = %ld, Size = %ld)\n", sender, msgcount, buffersize);
            }

            JrSleep(50);
        }
    }
}

  

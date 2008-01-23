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
    int handle = connect(myid);

    // Make a data buffer for incoming/outgoing messages.
    char buffer[MaxBufferSize];
    int counter = 0;         
    unsigned long sender;
    unsigned short datasize;

    // Do stuff
    while(1)
    {
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

            if (int result = sendto(handle, dest, datasize, buffer, 6, 1) != 0)
                printf("Sendto failed (%d)\n", result);
            //printf("Sending broadcast\n");
            //sprintf(buffer, "Urgent broadcast %d.\0", ++counter);
            //broadcast(handle, strlen(buffer), buffer, 15);
            //printf("Back from broadcast\n");

        }

        // check for incoming messages
        for (int i=0; i<100; i++)
        {
            int ret = recvfrom(handle, &sender, MaxBufferSize, buffer, NULL);
            if (ret > 0)
            {
                //std::string incoming_data(buffer, ret);
                //printf("Incoming Msg: %s (sender = %ld)\n", incoming_data.c_str(), sender);
                if (ret != datasize) printf("Error: Received wrong message size (sent=%d, recd=%d)\n", datasize, ret);
                printf("Incoming Msg: Sender = %ld, Size = %ld)\n", sender, ret);
            }

            JrSleep(50);
        }
    }
}

  

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

//#include "stdafx.h"
#include "JuniorAPI.h"
#include "OS.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <signal.h>
#include <time.h>
#ifdef WINDOWS
#include <windows.h>
#endif

// Define a sleep function 
#ifndef WINDOWS
void Sleep(unsigned long milliseconds){    usleep(milliseconds * 1000);	}
#endif

//#define _DEBUG_MODE
#ifdef _DEBUG_MODE
#define DPRINTF printf
#define DEBUG_MODE_TEXT	"ON"
#define DEBUG_MODE_VAR	1
#else
#define DPRINTF //
#define DEBUG_MODE_TEXT	"OFF"
#define DEBUG_MODE_VAR	0
#endif

// Return the current time (in milliseconds).
// This is defined in OS.h, but the Windows DLL does
// not expose the unmangled function.
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
#define CONFIG_PATH_NAME ""


const int MaxBufferSize = 70000;

static int exit_flag = 0;		// Define a signal handler, so we can clean-up properly
static void handle_exit_signal( int signum ){			exit_flag = 1;				}

/*     ----------  C O N S T A N T S  ------------    */
#define ALL_TESTS			0	//run all implemented tests
#define SIMPLE_TIME			5	//simple timing test is run if "test" == 0 or 0x0001 (first bit set)
#define HARD_TIME			6	//harder timing test is run if "test" == 0 or 0x0002 (second bit set)
#define LISTENER			9   //replies with msg only
#define REGRESSION_PRIORITY	1	//command line mapping to test functionality
#define REGRESSION_ACKNAK	2

//////////////////////////////////////////////////////////////////////////
/****                          CHANGE THESE                          ****/
//////////////////////////////////////////////////////////////////////////
#define SLEEP_TIME			1		//default milliseconds for sleep calls
#define LOOPY				50000	//100000	//loop count max for all tests
//////////////////////////////////////////////////////////////////////////

/*     ----------    G L O B A L S    ------------    */
	char		  progname[] = "jr-char1";
    unsigned int myid;
    unsigned int dest = 0;
    unsigned int size = 0;
    unsigned int test = 0;  //default = all

	int totalMsgsSent		= 0;
	int totalMsgsRcvd		= 0;
	int priorityMsgsSent	= 0;
	int acknakMsgsSent		= 0;
	int simpleMsgsSent		= 0;
	int hardMsgsSent		= 0;
	int totalErrorsDetected = 0;
	int outOfSequenceMsgs	= 0;
	int ackFailures			= 0;
	int delay_t				= SLEEP_TIME;
	int num_msgs            = LOOPY;
	int verbose				= 0;

	long starttime, endtime;
	double totalTime =0.0;

/*     ----------     W H A T    T E S T     ----------     */
std::string whatTest(int testid)				//returns a text string description of the test identifier
{
	switch (testid) 	// run some tests...
	{
		case ALL_TESTS:
			return("ALL TESTS");				break;
		case SIMPLE_TIME:
			return("SIMPLE TIME TEST");			break;
 		case HARD_TIME:
				return("HARD TIME TEST");		break;
		case REGRESSION_PRIORITY:		
			return("PRIORITY TEST");			break;
		case REGRESSION_ACKNAK:
			return("ACK/NAK TEST");				break;
		default: 
			return("UNKNOWN");			
	}
}
/*     ----------     S T A R T   T I M E     ----------     */
void startTime()
{
	starttime=GetTimestamp();
	//printf("Start Time: %ld\n", starttime);
}
/*     ----------     E L A P S E D   T I M E     ----------     */
void showElapsedTime()
{
	endtime=GetTimestamp();
	//printf("End time: %ld\n", endtime);
	double elapsedTime = ((double)(endtime-starttime)/1000.0); // in seconds
	printf("Summary: %s\n", whatTest(test).c_str()); 
	printf(" -----------------------------------------------\n");
	printf("    Elapsed Time            =	%2.4f seconds   ( %2.4f minutes )\n", elapsedTime, elapsedTime/60.0 );
	printf("    Total Messages Sent     =	%ld\n", totalMsgsSent);
	printf("      Priority Test Msgs    =	%ld\n", priorityMsgsSent);
	printf("      Ack/Nak Msgs          =	%ld\n", acknakMsgsSent);
	printf("      Simple Time Msgs      =	%ld\n", simpleMsgsSent);
	printf("      Large Msgs            =	%ld\n", hardMsgsSent);
	printf("    Total Messages Received =	%ld\n", totalMsgsRcvd);
	printf("    Total Errors Detected   =	%ld\n", totalErrorsDetected);
	printf("    Out of Sequence Msgs    =	%ld\n", outOfSequenceMsgs);
	printf("    Ack / Nak Failures      =	%ld  (counted only for ack/nak testing)\n", ackFailures);
	printf("    _________________________________________________________\n");
	printf("         \n");
	totalErrorsDetected += totalMsgsSent-totalMsgsRcvd; //account for messages not returned
	printf("    %2.4f percent failure rate  \n", (float)(totalErrorsDetected)/(float)(totalMsgsSent)*(100.0)  );
	printf("    %3.6f average round trip latency in milliseconds per message \n", (totalTime / (float)totalMsgsSent ));
	printf("            where Total Snd/Rcv time = %3.6f ms  \n", totalTime );
}
/*     ----------     S H O W   H E L P     ----------     */
void showHelp()
{
	printf("Usage: <my id> <dest id> <test: optional> <delay: optional> <num messages: optional> <verbose: optional>\n");
	printf("----------------------------------------------------------------------------\n");
	printf("<my id>    A unique (for your network) identifier for this JR node.\n");
	printf("<dest id>  A the identifier for the node you want to test against.\n");
	printf("<test>     A the specific test you want to run.  Default: Echo Responder (9).\n");
	printf("   The tests are identified as follows:\n");
	printf("      0 - All Tests (ONLY THOSE IMPLEMENTED)\n");
	printf("      1 - Priority Regression test\n");
	printf("      2 - Ack/Nak Regression test\n");
	printf("      3 - Multi-Packet Test  (NOT IMPLEMENTED)\n");
	printf("      4 - TBD test (NOT IMPLEMENTED) test\n");
	printf("      5 - Simple Time test\n");
	printf("      6 - Hard Time Test\n");
	printf("      7 - TBD test (NOT IMPLEMENTED) test\n");
	printf("      8 - TBD test (NOT IMPLEMENTED) test\n");
	printf("      9 - Echo Responder(Swap SRC & DEST and Return)\n");
	printf("<delay>    Command the sleep time between message sends (priority sends exluded).\n");
	printf("           The default is SLEEP_TIME (above).\n");
	printf("<num msgs> The number of messages sent per test\n");
	printf("           The default is LOOPY (above).\n");
	printf("<verbose>  Any value other than 0 will show additional detail.\n");
	printf("           The default is 0 (not verbose).\n");
	printf("----------------------------------------------------------------------------\n");
	printf("\n");
	printf("Typical: jr-char1 2 3 0  /* This ID 2 sends to ID 3 messages for all tests.      */\n");
	printf("         jr-char1 3 2    /* On another machine | process be the echo-responder.  */\n");
	printf("\n");
	printf("Debug:   jr-char1 2 3 0 50 100 1 /* ID 2 sends to ID 3 messages for all tests.      */\n");
	printf("         jr-char1 3 2 9 50 100 1 /* On another machine | process be the echo-responder.  */\n");
	printf("\n");
	printf("press ENTER to continue...\n");
	getchar();
}
/*     ----------  P A R S E   C M D   L I N E  ----------     */
void parseCmdLine(int argc, char* argv[])
{
    // Get the local id from the command line
    std::stringstream s; s << argv[1];
    s >> myid;

    // Get the command line argument for sending a message
    if (argc > 2)
    {
        std::stringstream t; t << argv[2];
        t >> dest;
    }

    // Get the command line argument for test number
    if (argc > 3)
    {
        std::stringstream u; u << argv[3];
        u >> test;
    }

    // Get the command line argument for delay time
    if (argc > 4)
    {
        std::stringstream v; v << argv[4];
        v >> delay_t;
    }

	// Get the command line argument for number of messages
    if (argc > 5)
    {
        std::stringstream v; v << argv[5];
        v >> num_msgs;
    }

    // Get the command line argument for verbose option
    if (argc > 6)
    {
        std::stringstream w; w << argv[6];
        w >> verbose;
    }
}
/*     ----------            E C H O   M O D E            ----------     */
void echoMode(long handle, unsigned int myid, unsigned int dest)
{
   // Make a data buffer for incoming/outgoing messages.
    char buffer[MaxBufferSize];
    int counter = 0;         
    unsigned int sender;
    //unsigned int datasize;
    int prevMsg  =0;
	unsigned short msg_id;
	int priority;

    printf("      This program will echo all messages back with src & dest switched.\n");

#ifdef WINDOWS
	//#include <windows.h> and add Kernel32.lib to link parameters
	SetPriorityClass ( GetCurrentProcess(), HIGH_PRIORITY_CLASS );
		/*
		REALTIME_PRIORITY_CLASS------highest
		HIGH_PRIORITY_CLASS
		ABOVE_NORMAL_PRIORITY_CLASS
		NORMAL_PRIORITY_CLASS
		BELOW_NORMAL_PRIORITY_CLASS
		IDLE_PRIORITY_CLASS------lowest
		*/
#endif

    while(!exit_flag)
    {
        // check for incoming messages
        unsigned int buffersize = MaxBufferSize; msg_id = 0; int flags = 0;
        JrErrorCode ret = JrReceive(handle, &sender, &buffersize, buffer, &priority, &flags, &msg_id);

        if (ret == Ok)
        {
			// Pull off the data that was embedded in the message.
			//long rcvTime=(long)clock();
			//int msgcount = ntohl(*((int*) buffer));
			//unsigned int sbuffersize = ntohl(*((unsigned int*) &buffer[4]));
			//unsigned short smsg_id = ntohs(*((unsigned short*) &buffer[8]));
			//int spriority = ntohl(*((int*) &buffer[10]));
			//int sflags = ntohl(*((int*) &buffer[14]));
			//long sndTime = ntohl(*((long*) &buffer[18]));
			//  IT APPEARS that between processes, the ticks are different.  No time to dig into this...
			//double age = ((((double)(rcvTime-sndTime)) / CLOCKS_PER_SEC)*1000.0);
			//DPRINTF("%ld RDD: source=%ld, dest=%ld, msg_id=%ld, size=%ld, priority=%ld, flags=%ld, age=%2.0f\n", msgcount, sender, myid, smsg_id, sbuffersize, spriority, sflags, age);
			if ((verbose)||( DEBUG_MODE_VAR ))
			printf("%ld ECHO: src=%ld, dest=%ld, id=%ld, size=%ld, priority=%ld, flags=%ld\n", totalMsgsSent, sender, myid, msg_id, buffersize, priority, flags);
			// Swap the SRC & DEST and send it back
			JrErrorCode result = JrSend(handle, sender, buffersize, buffer, priority, flags, msg_id);
			if ( result != Ok)
			{ 
				totalErrorsDetected++;   
				DPRINTF("ECHO Sendto failed (%d)\n", result);  
			}
			else 
				totalMsgsSent++;
        }
        else
    		Sleep(delay_t/10); //this must run faster...
    }
}
/*     ----------   M E S S A G E   S E N D E R    ----------     */
JrErrorCode sender( long handle, unsigned int myid, unsigned int dest, unsigned short msg_id, unsigned int datasize, int priority, int flags)
{
	char buffer[MaxBufferSize];  // Make a data buffer for incoming/outgoing messages.
    int counter = 0;
	unsigned int dsize = datasize;

	if (datasize < 32) dsize = 32; //must have space for the test values

	int packedMsgsSent = htonl(totalMsgsSent);
	memcpy( &buffer[0], (void*) &packedMsgsSent, 4);
	unsigned int packedSize = htonl(dsize);
	memcpy( &buffer[4], (void*) &packedSize, 4);
	unsigned short packedId= htons(msg_id);
	memcpy( &buffer[8], (void*) &packedId, 2);
	int packedPrio = htonl(priority);
	memcpy( &buffer[10], (void*) &packedPrio, 4);
	int packedFlags = htonl(flags);
	memcpy( &buffer[14], (void*) &packedFlags, 4);

	DPRINTF("%ld SND: src=%ld, dest=%ld, id=%ld, size=%ld, priority=%ld, flags=%ld\n", totalMsgsSent, myid, dest, msg_id, dsize, priority, flags);
	long packedSndTime = htonl(GetTimestamp());
	memcpy( &buffer[18], (void*) &packedSndTime, sizeof(long));
	return( JrSend(handle, dest, dsize, buffer, priority, flags, msg_id) );
}
/*     ----------   M E S S A G E   S C O R E R  ----------     */
void score(long handle, unsigned int myid, unsigned int dest)
{
	// This function needs a way to record all message numbers that have been returned so that 
	// when a message comes out of sequence we can record it as sent, late, low priority or ...

	char buffer[MaxBufferSize];  // Make a data buffer for incoming/outgoing messages.
    unsigned int sender;
	int priority = 0, previous_priority = 0;

	// check for incoming messages, loop either 100 times or exit after receipt of 4msgs
	int received = 0;
	for (int k = 0; k<100; k++) //arbitrary count of 10 
	{
        unsigned int buffersize = MaxBufferSize; unsigned short msg_id = 0; int flags = 0;
		JrErrorCode ret = JrReceive(handle, &sender, &buffersize, buffer, &priority, &flags, &msg_id);
        if (ret == Ok)
		{   
			long rcvTime=(long)GetTimestamp();

			// Pull off the data that was embedded in the message.
			int msgcount; memcpy( (void*) &msgcount, &buffer[0], 4); msgcount = ntohl(msgcount);
			unsigned int sbuffersize; memcpy( (void*) &sbuffersize, &buffer[4], 4); sbuffersize = ntohl(sbuffersize);
			unsigned short smsg_id; memcpy( (void*) &smsg_id, &buffer[8], 2); smsg_id = ntohs(smsg_id);
			int spriority; memcpy( (void*) &spriority, &buffer[10], 4); spriority = ntohl(spriority);
			int sflags; memcpy( (void*) &sflags, &buffer[14], 4); sflags = ntohl(sflags);
			long sndTime; memcpy( (void*) &sndTime, &buffer[18], sizeof(long)); sndTime = ntohl(sndTime);

			double age = rcvTime-sndTime;
			totalTime += age;
			if ((verbose)||( DEBUG_MODE_VAR ))
			    printf("snd: %ld, rcv: %ld, age: %g, total: %g\n", sndTime, rcvTime, age, totalTime);

			//show the echo with return values from jr API call
			//DPRINTF("%ld RCV: source=%ld, dest=%ld, msg_id=%ld, size=%ld, priority=%ld, flags=%ld\n", totalMsgsRcvd, sender, myid, msg_id, buffersize, priority, flags);
			//show the echo with data from buffer (message data)
			if ((verbose)||( DEBUG_MODE_VAR ))
				printf("%ld Sent, %ld Rcvd RT: src=%ld, dest=%ld, id=%ld, size=%ld, priority=%ld, flags=%ld, age=%2.0f\n", msgcount,totalMsgsRcvd,sender, myid, smsg_id, sbuffersize, spriority, sflags, age);

			if (smsg_id != msg_id)			totalErrorsDetected++; 
			if (sbuffersize != buffersize)	totalErrorsDetected++; 
			//if (msgcount != totalMsgsRcvd)	outOfSequenceMsgs++; 
			if ((priority > previous_priority) && (previous_priority != 0)) outOfSequenceMsgs++;
			previous_priority = priority;

			totalMsgsRcvd++;
			received++;				// receive a few messages if pending
			if(( received > 3 ) || ((totalMsgsRcvd + totalErrorsDetected) >= totalMsgsSent)) break;
		
		}
        else
            Sleep(delay_t/100); //this must run faster...
	}
}
/*     ----------   C A T C H   U P   ( P U R G E   R O U N D    T R I P    L O O P )  ----------     */
void catchUp(long handle, unsigned int myid, unsigned int dest)
{
	// let the echo-responder catch up
	int Ketchup = 0; //only let the catch-up routine run N iterations
	while (totalMsgsSent > (totalMsgsRcvd + totalErrorsDetected))	
	{
		score( handle, myid, dest );
		if ( Ketchup++ > 10 ) break; //where N is 10
	}
}
/*     ----------   P R I O R I T Y   T E S T  ----------     */
void priorityTest(long handle, unsigned int myid, unsigned int dest)
{		

    unsigned int datasize = 26;  //arbitrary, but holds the data fields necessary
	unsigned short msg_id   = 1; 
	int flags = ExperimentalFlag;

    for (int i=0; i<num_msgs; i++)  // Execute defined number of messages
    {
		if(exit_flag)break;
		for (int priority=0; priority<4; priority++) // Cycle through priorities 0 to 3
		{
			// Send a message of the given size, with a counter and size element
			JrErrorCode result = sender( handle, myid, dest, msg_id, datasize, priority, flags );
			if ( result != Ok)
			{ 
				totalErrorsDetected++; 
				DPRINTF("Sendto failed (%d)\n", result);
			}
			else 
			{ 
				priorityMsgsSent++; 	totalMsgsSent++;
				score( handle, myid, dest );
			}
		}
		Sleep(delay_t);
	}
	catchUp(handle, myid, dest);
}

/*     ----------   A C K  /  N A K   T E S T  ----------     */
void acknakTest(long handle, unsigned int myid, unsigned int dest)
{				
    unsigned int datasize = 26;  //arbitrary, but holds the data fields necessary
	unsigned short msg_id   = 2; 
	int priority = 6;
	int flags = ExperimentalFlag | GuaranteeDelivery;

    for (int i=0; i<num_msgs; i++)  // Execute defined number of messages
    {
		if(exit_flag)break;
		// Send a message of the given size, with a counter and size element
		JrErrorCode result = sender( handle, myid, dest, msg_id, datasize, priority, flags );
		if ( result != Ok)
		{ 
			totalErrorsDetected++; ackFailures++;
			DPRINTF("Sendto failed (%d)\n", result);
		}
		else 
		{ 
			acknakMsgsSent++; 	totalMsgsSent++;
			score(handle, myid, dest);
		}
		Sleep(delay_t);
	}
	catchUp(handle, myid, dest);
}
/*     ----------   S I M P L E   T I M E   T E S T  ----------     */
void simpleTimeTest(long handle, unsigned int myid, unsigned int dest)
{		
    unsigned int datasize = 26;  //arbitrary, but holds the data fields necessary
	unsigned short msg_id   = 3; 
	int priority = 6;
	int flags = ExperimentalFlag;
	int cycle = 0;

    for (int i=0; i<num_msgs; i++)  // Execute defined number of messages
    {
		if(exit_flag)break;
				
		switch (cycle++)  // Cycle through message sizes of 16, 24, 32, 64, 128, 1024, 2048
		{
			case	0:	datasize = 18; break;
			case    1:	datasize = 24; break;
			case    2:	datasize = 32; break;
			case	3:	datasize = 64; break;
			case    4:	datasize = 128; break;
			case    5:	datasize = 1024; break;
			case    6:	datasize = 2048; break;
			default :   break;
		}
		if (cycle > 6) cycle = 0;

		if(exit_flag)break;
		// Send a message of the given size, with a counter and size element
		JrErrorCode result = sender( handle, myid, dest, msg_id, datasize, priority, flags );
		if ( result != Ok)
		{ 
			totalErrorsDetected++; 
			DPRINTF("Sendto failed (%d)\n", result);
		}
		else 
		{ 
			simpleMsgsSent++; 	totalMsgsSent++;
			score(handle, myid, dest);
		}
		Sleep(delay_t);
	}
	catchUp(handle, myid, dest);
}

/*     ----------   H A R D   T I M E   T E S T  ----------     */
void hardTimeTest(long handle, unsigned int myid, unsigned int dest)
{		
    unsigned int datasize = 26;  //arbitrary, but holds the data fields necessary
	unsigned short msg_id   = 4; 
	int priority = 6;
	int flags = ExperimentalFlag;
	int cycle = 0;

    for (int i=0; i<num_msgs; i++)  // Execute defined number of messages
    {
		if(exit_flag)break;
				
		switch (cycle++)  // Cycle through message sizes of 10, 11, 18, 64, 128, 1024, 2048
		{
			case	0:	datasize = 1024; break;
			case    1:	datasize = 2048; break;
			case    2:	datasize = 4096; break;
			case	3:	datasize = 8192; break;
			case    4:	datasize = 16384; break;
			case    5:	datasize = 32768; break;
			case    6:	datasize = 512; break; //65536; break;
			default :   break;
		}
		if (cycle > 6) cycle = 0;
		if(exit_flag)break;
		// Send a message of the given size, with a counter and size element
		JrErrorCode result = sender( handle, myid, dest, msg_id, datasize, priority, flags );
		if ( result != Ok)
		{ 
			totalErrorsDetected++; 
			DPRINTF("Sendto failed (%d)\n", result);
		}
		else 
		{ 
			hardMsgsSent++; 	totalMsgsSent++;
			score(handle, myid, dest);
		}
		Sleep(delay_t);
	}
	catchUp(handle, myid, dest);
}

/*     ----------           M A I N         ----------     */
int main(int argc, char* argv[])
{
	// Offer a help screen and ensure the source ID is specified
	if  (argc < 2)
    {
		showHelp();
        return 1;
    }

	test = LISTENER;  //set default
	parseCmdLine(argc, argv);

    // Connect to the JR Run-Time Engine
    long handle;
    std::string file_path = std::string(CONFIG_PATH_NAME) + 
                            std::string("jr_config.xml");
    if (JrConnect(myid, file_path.c_str(), &handle) != Ok)
    {
        printf("Init failed.  Terminating execution\n");
        return -1;
    }

	if (dest == 0){ printf("Destination is 0, exiting\n");  return -1; }

    // Catch the termination signals
    signal( SIGINT, handle_exit_signal );
    signal( SIGTERM, handle_exit_signal );
    signal( SIGABRT, handle_exit_signal );

	printf("SLEEP TIME = %ld, Loops/Test = %d (x4 for priority), Debug Mode = %s \n", delay_t, num_msgs, DEBUG_MODE_TEXT);
	switch (test) 	// run some tests...
	{
		case ALL_TESTS:
			printf(" ALL TEST EXECUTION\n");	startTime();
			acknakTest(handle, myid, dest);	//Call the Test	
			simpleTimeTest(handle, myid, dest);
			hardTimeTest(handle, myid, dest);	
			priorityTest(handle, myid, dest);	
			showElapsedTime();	printf("press ENTER to continue...\n");	getchar();	break;
		case SIMPLE_TIME:
			printf(" SIMPLE TIME TEST EXECUTION\n");	startTime();
			simpleTimeTest(handle, myid, dest);	//Call the Test	
			showElapsedTime();	printf("press ENTER to continue...\n");	getchar();	break;
 		case HARD_TIME:
			printf(" HARD TIME TEST EXECUTION\n");	startTime();
			hardTimeTest(handle, myid, dest);	//Call the Test	
			showElapsedTime();	printf("press ENTER to continue...\n");	getchar();	break;
		case REGRESSION_PRIORITY:		
			printf(" PRIORITY TEST EXECUTION\n");	startTime();
			priorityTest(handle, myid, dest);	//Call the Test	
			showElapsedTime();	printf("press ENTER to continue...\n");	getchar();	break;
		case REGRESSION_ACKNAK:
			printf(" ACK/NAK TEST EXECUTION\n");	startTime();
			acknakTest(handle, myid, dest);	//Call the Test	
			showElapsedTime();	printf("press ENTER to continue...\n");	getchar();	break;
		case LISTENER:
			echoMode(handle, myid, dest);			break;
		default: ;
	}
    JrDisconnect(handle);  // clean-up
}


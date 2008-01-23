// This is the principle header file used by application's to access
// the Junior Toolset.  It defines all public functions and structures.
#ifndef __JUNIOR_API_H
#define __JUNIOR_API_H

// Convenient typedefs, enumerations and constants
typedef enum {Ok, NoMessages, InitFailed, AddrUnknown, Timeout, UnknownError} ErrorCode;
const unsigned char GuarenteeDelivery = 0x01;
const int JrMaxPriority = 15;

// Functional interface
int sendto(int handle,
           unsigned long destination, 
           unsigned int size, 
           const char* buffer,
           int priority,
           int flags);

int recvfrom(int handle,
             unsigned long* source,
             unsigned int bufsize,
             char* buffer,
             int* priority);

int broadcast(int handle,
              unsigned int bufsize,
              const char* buffer,
              int priority);

int connect(unsigned long id);

#endif



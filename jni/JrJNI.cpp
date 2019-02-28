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
#include "JrJNI.h"
#include "JuniorAPI.h"

JNIEXPORT jint JNICALL Java_DeVivo_JrInterface_JrConnect
  (JNIEnv *env, jobject, jint id, jstring filename, jlongArray pHandle)
{
    long handle;

    // Pull config file name from character array
    const char *cfgfn = env->GetStringUTFChars( filename, 0 );
    //printf("Calling new JrConnect (id=%ld, config=%s)...\n", id, cfgfn);

    // Connect with the given id and config file name
    int ret = ((int) JrConnect(id, cfgfn, &handle));

    // If successful, return the handle as an array element
    if (ret == 0)
    {
        jlong *body = env->GetLongArrayElements(pHandle, 0);
        body[0]=handle;
        //printf("Connected with handle=%ld (%ld)\n", handle, body[0]);
        env->ReleaseLongArrayElements(pHandle, body, 0);
    }
    return ret;
}

JNIEXPORT jint JNICALL Java_DeVivo_JrInterface_JrDisconnect
  (JNIEnv *, jobject, jlong handle)
{
    //printf("Calling JrDisconnect...\n");
    return ((int) JrDisconnect(handle));
}

JNIEXPORT jint JNICALL Java_DeVivo_JrInterface_JrSend
  (JNIEnv *env, jobject, jlong handle, jint dest, jint length, jbyteArray data)
{
    // Pin the pointer for the incoming data stream
    jbyte *body = env->GetByteArrayElements(data, 0);
    //printf("Sending %ld bytes to %ld\n", length, dest);

    // Now perform the send, freeing the pointer before we return
    int ret = ((int) JrSend(handle, dest, length, (const char*) body));
    env->ReleaseByteArrayElements(data, body, 0);
    return ret;
}


JNIEXPORT jint JNICALL Java_DeVivo_JrInterface_JrReceive
  (JNIEnv *env, jobject, jlong handle, jintArray pSource, jintArray pLength, jbyteArray data)
{
    // Pin the pointer for the incoming data stream
    jbyte *body = env->GetByteArrayElements(data, 0);
    jint *source = env->GetIntArrayElements(pSource, 0);
    jint *length = env->GetIntArrayElements(pLength, 0);
    //char buffer[20000];

    // Now perform the receive, freeing the pointers before we return
    int ret = ((int) JrReceive(handle, (unsigned int*) &source[0], (unsigned int*) &length[0], (char*) &body[0]));
    //if (ret == 0) printf("Got message from %ld (size=%ld)\n", source[0], length[0]);
    //fflush(stdout);

    // Free the pinned pointers to return the data
    env->ReleaseByteArrayElements(data, body, 0);
    env->ReleaseIntArrayElements(pLength, length, 0);
    env->ReleaseIntArrayElements(pSource, source, 0);
    return ret;
}

/*! 
 ***********************************************************************
 * @file      JrJNI.cpp
 * @author    Woody English, DeVivo AST, Inc.  
 * @date      2009/02/15
 *
 *  Copyright (C) 2009. DeVivo AST, Inc
 *
 *	This program is free software: you can redistribute it and/or modify  it 
 *  under the terms of the Jr Middleware Open Source License which can be 
 *  found at http://www.jrmiddleware.com/osl.html.  This program is 
 *  distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 *  PARTICULAR PURPOSE.  See the Jr Middleware Open Source License for more 
 *  details.
 *	
 *  For more information, please contact DeVivo AST at info@devivoast.com
 *  or by mail at 2225 Drake Ave, Suite 2, Huntsville, AL  35805.
 *
 *  The Jr Middleware Open Source License does not permit incorporating your 
 *  program into proprietary programs. If this is what you want to do, 
 *  use the Jr Middleware Commercial License. More information can be 
 *  found at: http://www.jrmiddleware.com/licensing.html.
 ************************************************************************
 */
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

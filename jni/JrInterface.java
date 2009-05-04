/*! 
 ***********************************************************************
 * @file      JrInterface.java
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

package DeVivo;

/**
 *
 * @author Dave
 */
public class JrInterface {

    static {System.loadLibrary("JrJNI");}

    public JrInterface()
    {
    }

    public int JrConnect(int id, String config_file_name)
    {
        // Disconnect existing connections before createing new one...
        JrDisconnect();
        int ret = (int) JrConnect(id, config_file_name, handle);
        return ret;
    }

    public int JrDisconnect()
    {
        if (handle[0] != 0)
        {
            JrDisconnect(handle[0]);
            handle[0]=0;
        }
        return 0;
    }

    public int JrSend(JrMessage msg)
    {
        return ((int) JrSend(handle[0], msg.getDestinationId(), msg.getLength(), msg.getByteArray()));
    }

    public JrMessage JrReceive()
    {
        // Reset the length, allowing Junior to write to the entire buffer
		length[0] = MaxMsgSize;

        // Now call receive, returning immediately if there is nothing to do
        int ret = ((int) JrReceive(handle[0], source, length, buffer));
		if (ret != 0)
		{
			if (ret!=1) System.out.printf("Receive message error (code=%d)\n", ret);
			return null;
		}

        // Create a JrMessage to return
        JrMessage msg = new JrMessage(length[0], buffer);
        msg.setSourceId(source[0]);
        return msg;
    }

	private int MaxMsgSize = 50000;
	private long[] handle = new long[1];
	private int[] length = new int[1];
	private int[] source = new int[1];
	private byte[] buffer = new byte[MaxMsgSize];

    // JNI interfaces...
    private native long JrConnect(int id, String filename, long[] pHandle);
    private native int JrDisconnect(long handle);
	private native int JrSend(long handle, int dest, int length, byte[] data);
	private native int JrReceive(long handle, int[] source, int[] length, byte[] data);

}

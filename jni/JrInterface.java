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

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
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 *
 * @author Dave
 */
public class JrMessage {
    public JrMessage(int bufsize)
    {
        _sourceId = _destId = 0;
        _data = ByteBuffer.allocate(bufsize);
        _data.order(ByteOrder.LITTLE_ENDIAN);
    }
    public JrMessage(int length, byte[] buffer)
    {
        _sourceId = _destId = 0;
        _data = ByteBuffer.allocate(length);
        _data.put(buffer, 0, length);
        _data.order(ByteOrder.LITTLE_ENDIAN);
    }
    public int getSourceId()
    {
        return _sourceId;
    }
    public void setSourceId(int id)
    {
        _sourceId = id;
    }
    public int getDestinationId()
    {
        return _destId;
    }
    public void setDestinationId(int id)
    {
        _destId = id;
    }
    public int getLength()
    {
        return _data.array().length;
    }
    public int getMsgId()
    {
        try
        {
            // Get the first two bytes of the buffer
            int temp = _data.getShort(0);
            if (temp < 0) temp += 0x10000;
            return temp;
        }
        catch (Exception e) {} // nothing to do since we return 0 anyway
        return 0;
    }
    public ByteBuffer getByteBuffer()
    {
        _data.rewind();
        return _data;
    }
    public byte[] getByteArray()
    {
        return _data.array();
    }

    // Private member data.  Note that we use an output stream since that
    // class will actually "own" the data and manage the buffer size.  We then
    // cast the buffer back to an input stream for reading.
    private int _sourceId;
    private int _destId;
    private ByteBuffer _data;
}

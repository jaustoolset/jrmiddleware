/*! 
 ***********************************************************************
 * @file      JrMessage.java
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2009/02/15
 *
 *  Copyright (C) 2009. DeVivo AST, Inc
 *
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

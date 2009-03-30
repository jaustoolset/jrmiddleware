/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
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
	private int[] handle = new int[1];
	private int[] length = new int[1];
	private int[] source = new int[1];
	private byte[] buffer = new byte[MaxMsgSize];

    // JNI interfaces...
    private native int JrConnect(int id, String filename, int[] pHandle);
    private native int JrDisconnect(int handle);
    private native int JrSend(int handle, int dest, int length, byte[] data);
    private native int JrReceive(int handle, int[] source, int[] length, byte[] data);

}

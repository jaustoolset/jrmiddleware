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
        handle = new int[1];
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
        // Create a local buffer to get the data
        byte[] buffer = new byte[25000];
        int[] length = new int[1]; length[0]=25000;
        int[] source = new int[1];

        // Now call receive, returning immediately if there is nothing to do
        int ret = ((int) JrReceive(handle[0], source, length, buffer));
        if (ret != 0) return null;
        //System.out.printf("Got a message from %d (size=%d)\n", source[0], length[0]);

        // Create a JrMessage to return
        JrMessage msg = new JrMessage(length[0], buffer);
        msg.setSourceId(source[0]);
        return msg;
    }

    private int[] handle;

    // JNI interfaces...
    private native int JrConnect(int id, String filename, int[] pHandle);
    private native int JrDisconnect(int handle);
    private native int JrSend(int handle, int dest, int length, byte[] data);
    private native int JrReceive(int handle, int[] source, int[] length, byte[] data);

}

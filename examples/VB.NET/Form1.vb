Public Class Form1

    ' List external functions defined by Junior
    Declare Function JrConnect Lib "Junior.dll" ( _
                ByVal id As Int32, _
                ByVal config_file As String, _
                ByRef handle As Int32) As Int32
    Declare Function JrDisconnect Lib "Junior.dll" ( _
            ByVal handle As Int32) As Int32
    Declare Function JrBroadcast Lib "Junior.dll" ( _
            ByVal handle As Int32, _
            ByVal buffer_size As UInt32, _
            ByVal buffer() As Byte, _
            ByVal priority As Int32, _
            ByVal msg_id As UInt16) As Int32
    Declare Function JrReceive Lib "Junior.dll" ( _
            ByVal handle As Int32, _
            ByRef source As UInt32, _
            ByRef buffer_size As UInt32, _
            ByVal buffer() As Byte, _
            ByRef priority As Int32, _
            ByRef flags As Integer, _
            ByRef msg_id As UInt16) As Int32
    Declare Function JrSend Lib "Junior.dll" ( _
            ByVal handle As Int32, _
            ByVal destination As UInt32, _
            ByVal buffer_size As UInt32, _
            ByVal buffer() As Byte, _
            ByVal priority As Int32, _
            ByVal flags As Int32, _
            ByVal msg_id As UInt16) As Int32

    'Define Class members
    Protected tHandle As Int32
    Protected tMap As New SortedList(Of String, UInt32)
    Protected tMap2 As New SortedList(Of UInt32, String)

    Private Sub ConnectButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ConnectButton.Click
        Dim ret As Integer
        Dim i As New Random()
        Dim myid As UInt32 = i.Next

        ' Check for invalid inputs
        If MyIdBox.TextLength = 0 Then
            MsgBox("Id string cannot be zero")
            Return
        End If

        ' Connect to the RTE using a random identifer.  Keep the handle.
        ret = JrConnect(myid, "", tHandle)
        If (ret <> 0) Then
            MsgBox("JrConnect returned error: " + ret.ToString)
            Return
        End If

        ' If we successfully connect, enable the GUI controls and 
        ' timers that send announcements and monitor incoming msgs
        ConnectButton.Enabled = False
        MyIdBox.Enabled = False
        DisconnectButton.Enabled = True
        SendButton.Enabled = True
        RecvTimer.Enabled = True
        DestinationsBox.Enabled = True
        AnnounceTimer.Enabled = True
    End Sub

    Private Sub DisconnectButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles DisconnectButton.Click
        ' Broadcast the disconnect message
        Dim stream(0) As Byte
        JrBroadcast(tHandle, 0, stream, 6, 3)

        ' Disconnect from the RTE, reset the handle, and turn off GUI controls
        JrDisconnect(tHandle)
        tHandle = 0
        ConnectButton.Enabled = True
        MyIdBox.Enabled = True
        DisconnectButton.Enabled = False
        SendButton.Enabled = False
        RecvTimer.Enabled = False
        DestinationsBox.Enabled = False
        AnnounceTimer.Enabled = False
    End Sub

    Private Sub SendButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles SendButton.Click
        Dim ret As Integer = 0
        Dim stream(5000) As Byte
        Dim destination As String

        ' If there is no message to send, return immediately.
        If OutgoingBox.TextLength = 0 Then
            Return
        End If

        ' If there are no destinations selected, return immediately.
        If DestinationsBox.CheckedItems.Count = 0 Then
            Return
        End If

        ' Convert the string to a byte stream (ASCI English encoding)
        stream = System.Text.Encoding.GetEncoding(1252).GetBytes(OutgoingBox.Text)

        ' For each checked destination, send the message
        For Each destination In DestinationsBox.CheckedItems
            ret = JrSend(tHandle, tMap(destination), stream.Length, stream, 6, 0, 2)
            If ret <> 0 Then
                MsgBox("Failed to send to " + " (" + destination + ")")
            End If
        Next

        ' Clear the outgoing text box, indicating that we've sent the message
        OutgoingBox.Clear()
    End Sub

    Private Sub Timer1_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles RecvTimer.Tick
        Dim ret As Integer
        Dim stream(5000) As Byte
        Dim source As UInt32
        Dim msg_id As UInt16
        Dim bufsize As UInt32 = 5000
        Dim priority As Int32
        Dim flags As Int32

        ' Try to receive a message into a byte buffer
        ret = JrReceive(tHandle, source, bufsize, stream, priority, flags, msg_id)

        ' Check for valid messages
        If ret = 1 Then
            Return
        ElseIf ret <> 0 Then
            MsgBox("Error on receive (" + ret.ToString + ")")
        ElseIf msg_id = 1 Then
            ' Received an announcement.  If we don't know about this destination,
            ' add it to the list.
            Dim name As String = System.Text.Encoding.GetEncoding(1252).GetString(stream, 0, bufsize)
            If tMap.ContainsKey(name) = False Then
                tMap.Add(name, source)
                tMap2.Add(source, name)
                DestinationsBox.Items.Add(name)
            End If
        ElseIf msg_id = 2 Then
            ' Standard message.  Post to the text box, along with the source name.
            IncomingBox.AppendText("(from " + tMap2(source) + "): " + _
                 System.Text.Encoding.GetEncoding(1252).GetString(stream, 0, bufsize) + vbNewLine)
        ElseIf msg_id = 3 Then
            ' Disconnect message.  Remove from the maps.
            Dim name As String = tMap2(source)
            tMap.Remove(name)
            tMap2.Remove(source)
            DestinationsBox.Items.Remove(name)
        End If
    End Sub

    Private Sub Form1_Quit(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Leave
        JrDisconnect(tHandle)
        tHandle = 0
        RecvTimer.Enabled = False
        AnnounceTimer.Enabled = False
    End Sub

    Private Sub AnnounceTimer_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles AnnounceTimer.Tick
        Dim ret As Integer
        Dim stream() As Byte

        ' The announcement message has our name (string) encoded.
        stream = System.Text.Encoding.GetEncoding(1252).GetBytes(MyIdBox.Text)

        ' Broadcast to everyone.
        ret = JrBroadcast(tHandle, stream.Length, stream, 6, 1)
        If ret <> 0 Then
            MsgBox("Failed to broadcast (" + ret.ToString + ")")
        End If

    End Sub
End Class

<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Me.ConnectButton = New System.Windows.Forms.Button
        Me.Label1 = New System.Windows.Forms.Label
        Me.DisconnectButton = New System.Windows.Forms.Button
        Me.SendButton = New System.Windows.Forms.Button
        Me.OutgoingBox = New System.Windows.Forms.RichTextBox
        Me.Label2 = New System.Windows.Forms.Label
        Me.Label3 = New System.Windows.Forms.Label
        Me.IncomingBox = New System.Windows.Forms.RichTextBox
        Me.RecvTimer = New System.Windows.Forms.Timer(Me.components)
        Me.MyIdBox = New System.Windows.Forms.RichTextBox
        Me.DestinationsBox = New System.Windows.Forms.CheckedListBox
        Me.Label4 = New System.Windows.Forms.Label
        Me.AnnounceTimer = New System.Windows.Forms.Timer(Me.components)
        Me.SuspendLayout()
        '
        'ConnectButton
        '
        Me.ConnectButton.Location = New System.Drawing.Point(53, 51)
        Me.ConnectButton.Name = "ConnectButton"
        Me.ConnectButton.Size = New System.Drawing.Size(55, 26)
        Me.ConnectButton.TabIndex = 0
        Me.ConnectButton.Text = "Connect"
        Me.ConnectButton.UseVisualStyleBackColor = True
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Location = New System.Drawing.Point(12, 25)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(35, 13)
        Me.Label1.TabIndex = 2
        Me.Label1.Text = "My ID"
        '
        'DisconnectButton
        '
        Me.DisconnectButton.Enabled = False
        Me.DisconnectButton.Location = New System.Drawing.Point(114, 51)
        Me.DisconnectButton.Name = "DisconnectButton"
        Me.DisconnectButton.Size = New System.Drawing.Size(76, 26)
        Me.DisconnectButton.TabIndex = 3
        Me.DisconnectButton.Text = "Disconnect"
        Me.DisconnectButton.UseVisualStyleBackColor = True
        '
        'SendButton
        '
        Me.SendButton.Enabled = False
        Me.SendButton.Location = New System.Drawing.Point(35, 228)
        Me.SendButton.Name = "SendButton"
        Me.SendButton.Size = New System.Drawing.Size(155, 29)
        Me.SendButton.TabIndex = 5
        Me.SendButton.Text = "Send Message"
        Me.SendButton.UseVisualStyleBackColor = True
        '
        'OutgoingBox
        '
        Me.OutgoingBox.Location = New System.Drawing.Point(35, 116)
        Me.OutgoingBox.Name = "OutgoingBox"
        Me.OutgoingBox.Size = New System.Drawing.Size(155, 103)
        Me.OutgoingBox.TabIndex = 6
        Me.OutgoingBox.Text = ""
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(32, 100)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(76, 13)
        Me.Label2.TabIndex = 7
        Me.Label2.Text = "My Message..."
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Location = New System.Drawing.Point(227, 100)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(104, 13)
        Me.Label3.TabIndex = 9
        Me.Label3.Text = "Incoming message..."
        '
        'IncomingBox
        '
        Me.IncomingBox.Location = New System.Drawing.Point(227, 116)
        Me.IncomingBox.Name = "IncomingBox"
        Me.IncomingBox.Size = New System.Drawing.Size(213, 141)
        Me.IncomingBox.TabIndex = 8
        Me.IncomingBox.Text = ""
        '
        'RecvTimer
        '
        '
        'MyIdBox
        '
        Me.MyIdBox.Location = New System.Drawing.Point(53, 20)
        Me.MyIdBox.MaxLength = 15
        Me.MyIdBox.Multiline = False
        Me.MyIdBox.Name = "MyIdBox"
        Me.MyIdBox.Size = New System.Drawing.Size(137, 25)
        Me.MyIdBox.TabIndex = 10
        Me.MyIdBox.Text = ""
        '
        'DestinationsBox
        '
        Me.DestinationsBox.FormattingEnabled = True
        Me.DestinationsBox.Location = New System.Drawing.Point(230, 28)
        Me.DestinationsBox.Name = "DestinationsBox"
        Me.DestinationsBox.Size = New System.Drawing.Size(209, 49)
        Me.DestinationsBox.TabIndex = 11
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Location = New System.Drawing.Point(227, 12)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(53, 13)
        Me.Label4.TabIndex = 12
        Me.Label4.Text = "Send to..."
        '
        'AnnounceTimer
        '
        Me.AnnounceTimer.Interval = 1000
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(452, 273)
        Me.Controls.Add(Me.Label4)
        Me.Controls.Add(Me.DestinationsBox)
        Me.Controls.Add(Me.MyIdBox)
        Me.Controls.Add(Me.Label3)
        Me.Controls.Add(Me.IncomingBox)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.OutgoingBox)
        Me.Controls.Add(Me.SendButton)
        Me.Controls.Add(Me.DisconnectButton)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.ConnectButton)
        Me.Name = "Form1"
        Me.Text = "Form1"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents ConnectButton As System.Windows.Forms.Button
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents DisconnectButton As System.Windows.Forms.Button
    Friend WithEvents SendButton As System.Windows.Forms.Button
    Friend WithEvents OutgoingBox As System.Windows.Forms.RichTextBox
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents IncomingBox As System.Windows.Forms.RichTextBox
    Friend WithEvents RecvTimer As System.Windows.Forms.Timer
    Friend WithEvents MyIdBox As System.Windows.Forms.RichTextBox
    Friend WithEvents DestinationsBox As System.Windows.Forms.CheckedListBox
    Friend WithEvents Label4 As System.Windows.Forms.Label
    Friend WithEvents AnnounceTimer As System.Windows.Forms.Timer

End Class

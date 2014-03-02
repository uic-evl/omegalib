using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;

namespace MissionControl
{
    public class Connection
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        public bool Connect(string address, int port)
        {
            if(myClient == null)
            {
                myClient = new TcpClient();
                myBufferSize = 1024;
                myBuffer = new byte[myBufferSize];
            }
            try
            {
                myClient.Connect(address, port);
            }
            catch(SocketException e)
            {
                System.Windows.Forms.MessageBox.Show(
                    e.Message, "Connection Error", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Warning);
                return false;
            }
            return true;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void Poll()
        {
            // See if some data is available.
            while(myClient.Available != 0)
            {
                // Read message header.
                myClient.GetStream().Read(myBuffer, 0, 4);

                String header = Encoding.UTF8.GetString(myBuffer, 0, 4);

                // Read message payload
                myClient.GetStream().Read(myBuffer, 0, 4);
                int size = BitConverter.ToInt32(myBuffer, 0);
                // Resize the buffer if needed.
                if(myBufferSize < size)
                {
                    myBufferSize = size;
                    myBuffer = new byte[myBufferSize];
                }
                myClient.GetStream().Read(myBuffer, 0, size);

                // Handle some default message types
                // smsg: log message, just print it to the console.
                if(header == "smsg")
                {
                    String message = Encoding.UTF8.GetString(myBuffer, 0, size);
                    // Ignore messages containing only a newline)
                    if(message != "\n")
                    {
                        message += System.Environment.NewLine;
                        MainWindow.Instance.PrintMessage(message);
                    }
                }
                if(header == "help")
                {
                    String message = Encoding.UTF8.GetString(myBuffer, 0, size);
                    string[] items = message.Split('|');
                    for(int i = 0; i < items.Length; i += 2)
                    {
                        MainWindow.Instance.AddAutocompletionItem(items[i]);
                    }
                }
                // Stats message: contains a string of stat names. Pass it to the stats
                // window, so it can initialize the stats list.
                if(header == "strq")
                {
                    String message = Encoding.UTF8.GetString(myBuffer, 0, size);
                    string[] items = message.Split(new char[] {'|'}, StringSplitOptions.RemoveEmptyEntries);
                    MainWindow.Instance.StatsWindow.SetStats(items);
                }
                // Stats update: contains a string of stat ids followed by current, min, max and average values (as strings). 
                if(header == "stup")
                {
                    String message = Encoding.UTF8.GetString(myBuffer, 0, size);
                    string[] items = message.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                    MainWindow.Instance.StatsWindow.UpdateStats(items);
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void SendCommand(string cmd)
        {
            if(myClient != null && myClient.Connected)
            {
                try
                {
                    byte[] header = Encoding.UTF8.GetBytes("scmd");
                    myClient.GetStream().Write(header, 0, 4);
                    byte[] cmdBytes = Encoding.UTF8.GetBytes(cmd);
                    byte[] cmdLengthBytes = BitConverter.GetBytes(cmdBytes.Length);
                    myClient.GetStream().Write(cmdLengthBytes, 0, cmdLengthBytes.Length);
                    myClient.GetStream().Write(cmdBytes, 0, cmdBytes.Length);
                }
                catch(System.IO.IOException e)
                {
                    MainWindow.Instance.PrintMessage("Connection closed.");
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void SendMessage(string headerString)
        {
            SendMessage(headerString, "");
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void SendMessage(string headerString, string messagePayload)
        {
            if(myClient != null && myClient.Connected)
            {
                try
                {
                    byte[] header = Encoding.UTF8.GetBytes(headerString);
                    myClient.GetStream().Write(header, 0, 4);
                    byte[] cmdBytes = Encoding.UTF8.GetBytes(messagePayload);
                    byte[] cmdLengthBytes = BitConverter.GetBytes(cmdBytes.Length);
                    myClient.GetStream().Write(cmdLengthBytes, 0, cmdLengthBytes.Length);
                    myClient.GetStream().Write(cmdBytes, 0, cmdBytes.Length);
                }
                catch(System.IO.IOException e)
                {
                    MainWindow.Instance.PrintMessage("Connection closed.");
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public bool Connected
        {
            get
            {
                return (myClient != null && myClient.Connected);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private TcpClient myClient;
        private byte[] myBuffer;
        private int myBufferSize;
    }
}

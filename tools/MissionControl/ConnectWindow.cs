using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MissionControl
{
    public partial class ConnectWindow: Form
    {
        public ConnectWindow(Connection connection)
        {
            InitializeComponent();
            myConnection = connection;
        }

        Connection myConnection;

        private void myConnectButton_Click(object sender, EventArgs e)
        {
            string address = myAddressBox.Text;
            int port = int.Parse(myPortBox.Text);
            myConnection.Connect(address, port);
            Close();
        }
    }
}

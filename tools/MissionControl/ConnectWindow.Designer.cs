namespace MissionControl
{
    partial class ConnectWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if(disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.myAddressBox = new System.Windows.Forms.TextBox();
            this.myPortBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.myConnectButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // myAddressBox
            // 
            this.myAddressBox.Location = new System.Drawing.Point(10, 32);
            this.myAddressBox.Name = "myAddressBox";
            this.myAddressBox.Size = new System.Drawing.Size(186, 22);
            this.myAddressBox.TabIndex = 0;
            this.myAddressBox.Text = "localhost";
            // 
            // myPortBox
            // 
            this.myPortBox.Location = new System.Drawing.Point(203, 32);
            this.myPortBox.Name = "myPortBox";
            this.myPortBox.Size = new System.Drawing.Size(100, 22);
            this.myPortBox.TabIndex = 1;
            this.myPortBox.Text = "22500";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(11, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 17);
            this.label1.TabIndex = 2;
            this.label1.Text = "Address";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(203, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(34, 17);
            this.label2.TabIndex = 3;
            this.label2.Text = "Port";
            // 
            // myConnectButton
            // 
            this.myConnectButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myConnectButton.Location = new System.Drawing.Point(231, 62);
            this.myConnectButton.Name = "myConnectButton";
            this.myConnectButton.Size = new System.Drawing.Size(75, 33);
            this.myConnectButton.TabIndex = 4;
            this.myConnectButton.Text = "Connect";
            this.myConnectButton.UseVisualStyleBackColor = true;
            this.myConnectButton.Click += new System.EventHandler(this.myConnectButton_Click);
            // 
            // ConnectWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(312, 104);
            this.Controls.Add(this.myConnectButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.myPortBox);
            this.Controls.Add(this.myAddressBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "ConnectWindow";
            this.Text = "ConnectWindow";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox myAddressBox;
        private System.Windows.Forms.TextBox myPortBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button myConnectButton;

    }
}
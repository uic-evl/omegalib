namespace MissionControl
{
    partial class MainWindow
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
            this.components = new System.ComponentModel.Container();
            this.myOutputBox = new FastColoredTextBoxNS.FastColoredTextBox();
            this.myPromptBox = new FastColoredTextBoxNS.FastColoredTextBox();
            this.myMenuStrip = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.connectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.disconnectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.macroEditorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statsViewerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.myConnectionPoller = new System.Windows.Forms.Timer(this.components);
            this.myMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // myOutputBox
            // 
            this.myOutputBox.AllowDrop = true;
            this.myOutputBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.myOutputBox.AutoScrollMinSize = new System.Drawing.Size(2, 17);
            this.myOutputBox.BackBrush = null;
            this.myOutputBox.BackColor = System.Drawing.Color.Black;
            this.myOutputBox.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.myOutputBox.DisabledColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))));
            this.myOutputBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.myOutputBox.ForeColor = System.Drawing.Color.White;
            this.myOutputBox.Location = new System.Drawing.Point(0, 27);
            this.myOutputBox.Name = "myOutputBox";
            this.myOutputBox.Paddings = new System.Windows.Forms.Padding(0);
            this.myOutputBox.ReadOnly = true;
            this.myOutputBox.SelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
            this.myOutputBox.ShowLineNumbers = false;
            this.myOutputBox.Size = new System.Drawing.Size(739, 400);
            this.myOutputBox.TabIndex = 0;
            // 
            // myPromptBox
            // 
            this.myPromptBox.AcceptsReturn = false;
            this.myPromptBox.AllowDrop = true;
            this.myPromptBox.BackBrush = null;
            this.myPromptBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.myPromptBox.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.myPromptBox.DisabledColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))));
            this.myPromptBox.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.myPromptBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.myPromptBox.Location = new System.Drawing.Point(0, 432);
            this.myPromptBox.Multiline = false;
            this.myPromptBox.Name = "myPromptBox";
            this.myPromptBox.Paddings = new System.Windows.Forms.Padding(0);
            this.myPromptBox.SelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
            this.myPromptBox.ShowLineNumbers = false;
            this.myPromptBox.ShowScrollBars = false;
            this.myPromptBox.Size = new System.Drawing.Size(739, 33);
            this.myPromptBox.TabIndex = 1;
            this.myPromptBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.myPromptBox_KeyPress);
            // 
            // myMenuStrip
            // 
            this.myMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.toolsToolStripMenuItem});
            this.myMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.myMenuStrip.Name = "myMenuStrip";
            this.myMenuStrip.Size = new System.Drawing.Size(739, 28);
            this.myMenuStrip.TabIndex = 2;
            this.myMenuStrip.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.connectToolStripMenuItem,
            this.disconnectToolStripMenuItem,
            this.quitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(44, 24);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // connectToolStripMenuItem
            // 
            this.connectToolStripMenuItem.Name = "connectToolStripMenuItem";
            this.connectToolStripMenuItem.Size = new System.Drawing.Size(151, 24);
            this.connectToolStripMenuItem.Text = "Connect...";
            this.connectToolStripMenuItem.Click += new System.EventHandler(this.connectToolStripMenuItem_Click);
            // 
            // disconnectToolStripMenuItem
            // 
            this.disconnectToolStripMenuItem.Name = "disconnectToolStripMenuItem";
            this.disconnectToolStripMenuItem.Size = new System.Drawing.Size(151, 24);
            this.disconnectToolStripMenuItem.Text = "Disconnect";
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(151, 24);
            this.quitToolStripMenuItem.Text = "Quit";
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.macroEditorToolStripMenuItem,
            this.statsViewerToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(57, 24);
            this.toolsToolStripMenuItem.Text = "Tools";
            // 
            // macroEditorToolStripMenuItem
            // 
            this.macroEditorToolStripMenuItem.Name = "macroEditorToolStripMenuItem";
            this.macroEditorToolStripMenuItem.Size = new System.Drawing.Size(164, 24);
            this.macroEditorToolStripMenuItem.Text = "Macro Editor";
            this.macroEditorToolStripMenuItem.Click += new System.EventHandler(this.macroEditorToolStripMenuItem_Click);
            // 
            // statsViewerToolStripMenuItem
            // 
            this.statsViewerToolStripMenuItem.Name = "statsViewerToolStripMenuItem";
            this.statsViewerToolStripMenuItem.Size = new System.Drawing.Size(164, 24);
            this.statsViewerToolStripMenuItem.Text = "Stats Viewer";
            this.statsViewerToolStripMenuItem.Click += new System.EventHandler(this.statsViewerToolStripMenuItem_Click);
            // 
            // myConnectionPoller
            // 
            this.myConnectionPoller.Enabled = true;
            this.myConnectionPoller.Tick += new System.EventHandler(this.myConnectionPoller_Tick);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(739, 465);
            this.Controls.Add(this.myPromptBox);
            this.Controls.Add(this.myOutputBox);
            this.Controls.Add(this.myMenuStrip);
            this.MainMenuStrip = this.myMenuStrip;
            this.Name = "MainWindow";
            this.Text = "omegalib Mission Control";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainWindow_FormClosing);
            this.myMenuStrip.ResumeLayout(false);
            this.myMenuStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private FastColoredTextBoxNS.FastColoredTextBox myOutputBox;
        private FastColoredTextBoxNS.FastColoredTextBox myPromptBox;
        private System.Windows.Forms.MenuStrip myMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem connectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem disconnectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem quitToolStripMenuItem;
        private System.Windows.Forms.Timer myConnectionPoller;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem macroEditorToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem statsViewerToolStripMenuItem;
    }
}


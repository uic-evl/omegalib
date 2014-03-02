namespace MissionControl
{
    partial class MacroWindow
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
            this.myMacroList = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.myShortcutBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.myScriptBox = new FastColoredTextBoxNS.FastColoredTextBox();
            this.mySaveButton = new System.Windows.Forms.Button();
            this.myDeleteButton = new System.Windows.Forms.Button();
            this.myRunButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // myMacroList
            // 
            this.myMacroList.Dock = System.Windows.Forms.DockStyle.Left;
            this.myMacroList.FormattingEnabled = true;
            this.myMacroList.ItemHeight = 16;
            this.myMacroList.Location = new System.Drawing.Point(0, 0);
            this.myMacroList.Name = "myMacroList";
            this.myMacroList.Size = new System.Drawing.Size(139, 554);
            this.myMacroList.TabIndex = 0;
            this.myMacroList.SelectedIndexChanged += new System.EventHandler(this.myMacroList_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(145, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(61, 17);
            this.label1.TabIndex = 1;
            this.label1.Text = "Shortcut";
            // 
            // myShortcutBox
            // 
            this.myShortcutBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.myShortcutBox.Location = new System.Drawing.Point(148, 30);
            this.myShortcutBox.Name = "myShortcutBox";
            this.myShortcutBox.Size = new System.Drawing.Size(698, 22);
            this.myShortcutBox.TabIndex = 2;
            this.myShortcutBox.TextChanged += new System.EventHandler(this.myShortcutBox_TextChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(146, 74);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(44, 17);
            this.label2.TabIndex = 3;
            this.label2.Text = "Script";
            // 
            // myScriptBox
            // 
            this.myScriptBox.AllowDrop = true;
            this.myScriptBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.myScriptBox.AutoScrollMinSize = new System.Drawing.Size(2, 17);
            this.myScriptBox.BackBrush = null;
            this.myScriptBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.myScriptBox.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.myScriptBox.DisabledColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))), ((int)(((byte)(180)))));
            this.myScriptBox.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.myScriptBox.Location = new System.Drawing.Point(149, 96);
            this.myScriptBox.Name = "myScriptBox";
            this.myScriptBox.Paddings = new System.Windows.Forms.Padding(0);
            this.myScriptBox.SelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
            this.myScriptBox.ShowLineNumbers = false;
            this.myScriptBox.Size = new System.Drawing.Size(697, 423);
            this.myScriptBox.TabIndex = 4;
            this.myScriptBox.TextChanged += new System.EventHandler<FastColoredTextBoxNS.TextChangedEventArgs>(this.myScriptBox_TextChanged);
            // 
            // mySaveButton
            // 
            this.mySaveButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.mySaveButton.Enabled = false;
            this.mySaveButton.Location = new System.Drawing.Point(445, 525);
            this.mySaveButton.Name = "mySaveButton";
            this.mySaveButton.Size = new System.Drawing.Size(114, 23);
            this.mySaveButton.TabIndex = 5;
            this.mySaveButton.Text = "New macro";
            this.mySaveButton.UseVisualStyleBackColor = true;
            this.mySaveButton.Click += new System.EventHandler(this.mySaveButton_Click);
            // 
            // myDeleteButton
            // 
            this.myDeleteButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myDeleteButton.Enabled = false;
            this.myDeleteButton.Location = new System.Drawing.Point(566, 525);
            this.myDeleteButton.Name = "myDeleteButton";
            this.myDeleteButton.Size = new System.Drawing.Size(127, 23);
            this.myDeleteButton.TabIndex = 6;
            this.myDeleteButton.Text = "Delete macro";
            this.myDeleteButton.UseVisualStyleBackColor = true;
            this.myDeleteButton.Click += new System.EventHandler(this.myDeleteButton_Click);
            // 
            // myRunButton
            // 
            this.myRunButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myRunButton.Enabled = false;
            this.myRunButton.Location = new System.Drawing.Point(699, 525);
            this.myRunButton.Name = "myRunButton";
            this.myRunButton.Size = new System.Drawing.Size(149, 23);
            this.myRunButton.TabIndex = 7;
            this.myRunButton.Text = "Run macro";
            this.myRunButton.UseVisualStyleBackColor = true;
            this.myRunButton.Click += new System.EventHandler(this.myRunButton_Click);
            // 
            // MacroWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(858, 554);
            this.Controls.Add(this.myRunButton);
            this.Controls.Add(this.myDeleteButton);
            this.Controls.Add(this.mySaveButton);
            this.Controls.Add(this.myScriptBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.myShortcutBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.myMacroList);
            this.Name = "MacroWindow";
            this.Text = "MacroWindow";
            this.Load += new System.EventHandler(this.MacroWindow_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox myMacroList;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox myShortcutBox;
        private System.Windows.Forms.Label label2;
        private FastColoredTextBoxNS.FastColoredTextBox myScriptBox;
        private System.Windows.Forms.Button mySaveButton;
        private System.Windows.Forms.Button myDeleteButton;
        private System.Windows.Forms.Button myRunButton;
    }
}
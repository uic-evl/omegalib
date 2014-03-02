namespace MissionControl
{
    partial class ScriptControlWindow
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
            this.myLayout = new System.Windows.Forms.FlowLayoutPanel();
            this.myColorDialog = new System.Windows.Forms.ColorDialog();
            this.SuspendLayout();
            // 
            // myLayout
            // 
            this.myLayout.AutoSize = true;
            this.myLayout.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.myLayout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.myLayout.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.myLayout.Location = new System.Drawing.Point(0, 0);
            this.myLayout.Name = "myLayout";
            this.myLayout.Size = new System.Drawing.Size(423, 372);
            this.myLayout.TabIndex = 0;
            // 
            // ScriptControlWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(423, 372);
            this.Controls.Add(this.myLayout);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "ScriptControlWindow";
            this.Text = "ScriptControlWindow";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.FlowLayoutPanel myLayout;
        private System.Windows.Forms.ColorDialog myColorDialog;
    }
}
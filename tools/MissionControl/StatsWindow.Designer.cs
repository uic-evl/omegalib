namespace MissionControl
{
    partial class StatsWindow
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
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            this.myChart = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.myStatsBox = new System.Windows.Forms.CheckedListBox();
            this.myTimer = new System.Windows.Forms.Timer(this.components);
            this.myStatLabel = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.myChartAreaBox = new System.Windows.Forms.ComboBox();
            this.myDeleteAreasButton = new System.Windows.Forms.Button();
            this.myNewAreaButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.myChart)).BeginInit();
            this.SuspendLayout();
            // 
            // myChart
            // 
            this.myChart.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            chartArea1.Name = "ChartArea1";
            this.myChart.ChartAreas.Add(chartArea1);
            legend1.Name = "Legend1";
            this.myChart.Legends.Add(legend1);
            this.myChart.Location = new System.Drawing.Point(2, 2);
            this.myChart.Name = "myChart";
            this.myChart.Size = new System.Drawing.Size(491, 495);
            this.myChart.TabIndex = 0;
            this.myChart.Text = "chart1";
            // 
            // myStatsBox
            // 
            this.myStatsBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.myStatsBox.FormattingEnabled = true;
            this.myStatsBox.Location = new System.Drawing.Point(497, 0);
            this.myStatsBox.Name = "myStatsBox";
            this.myStatsBox.Size = new System.Drawing.Size(245, 310);
            this.myStatsBox.TabIndex = 1;
            this.myStatsBox.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.myStatsBox_ItemCheck);
            this.myStatsBox.SelectedIndexChanged += new System.EventHandler(this.myStatsBox_SelectedIndexChanged);
            // 
            // myTimer
            // 
            this.myTimer.Enabled = true;
            this.myTimer.Interval = 1000;
            this.myTimer.Tick += new System.EventHandler(this.myTimer_Tick);
            // 
            // myStatLabel
            // 
            this.myStatLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myStatLabel.AutoSize = true;
            this.myStatLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.myStatLabel.Location = new System.Drawing.Point(494, 328);
            this.myStatLabel.Name = "myStatLabel";
            this.myStatLabel.Size = new System.Drawing.Size(101, 17);
            this.myStatLabel.TabIndex = 2;
            this.myStatLabel.Text = "options for X";
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(494, 345);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(76, 17);
            this.label2.TabIndex = 3;
            this.label2.Text = "Chart Area";
            // 
            // myChartAreaBox
            // 
            this.myChartAreaBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myChartAreaBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.myChartAreaBox.FormattingEnabled = true;
            this.myChartAreaBox.Location = new System.Drawing.Point(497, 365);
            this.myChartAreaBox.Name = "myChartAreaBox";
            this.myChartAreaBox.Size = new System.Drawing.Size(224, 24);
            this.myChartAreaBox.TabIndex = 4;
            this.myChartAreaBox.SelectedIndexChanged += new System.EventHandler(this.myChartAreaBox_SelectedIndexChanged);
            // 
            // myDeleteAreasButton
            // 
            this.myDeleteAreasButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myDeleteAreasButton.Location = new System.Drawing.Point(361, 468);
            this.myDeleteAreasButton.Name = "myDeleteAreasButton";
            this.myDeleteAreasButton.Size = new System.Drawing.Size(127, 23);
            this.myDeleteAreasButton.TabIndex = 5;
            this.myDeleteAreasButton.Text = "Delete all areas";
            this.myDeleteAreasButton.UseVisualStyleBackColor = true;
            this.myDeleteAreasButton.Click += new System.EventHandler(this.myDeleteAreasButton_Click);
            // 
            // myNewAreaButton
            // 
            this.myNewAreaButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.myNewAreaButton.Location = new System.Drawing.Point(500, 396);
            this.myNewAreaButton.Name = "myNewAreaButton";
            this.myNewAreaButton.Size = new System.Drawing.Size(75, 23);
            this.myNewAreaButton.TabIndex = 6;
            this.myNewAreaButton.Text = "Add area";
            this.myNewAreaButton.UseVisualStyleBackColor = true;
            this.myNewAreaButton.Click += new System.EventHandler(this.myNewAreaButton_Click);
            // 
            // StatsWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(742, 509);
            this.Controls.Add(this.myNewAreaButton);
            this.Controls.Add(this.myDeleteAreasButton);
            this.Controls.Add(this.myChartAreaBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.myStatLabel);
            this.Controls.Add(this.myStatsBox);
            this.Controls.Add(this.myChart);
            this.Name = "StatsWindow";
            this.Text = "StatsWindow";
            this.Load += new System.EventHandler(this.StatsWindow_Load);
            ((System.ComponentModel.ISupportInitialize)(this.myChart)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart myChart;
        private System.Windows.Forms.CheckedListBox myStatsBox;
        private System.Windows.Forms.Timer myTimer;
        private System.Windows.Forms.Label myStatLabel;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox myChartAreaBox;
        private System.Windows.Forms.Button myDeleteAreasButton;
        private System.Windows.Forms.Button myNewAreaButton;
    }
}
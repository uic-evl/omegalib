using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

namespace MissionControl
{
    ///////////////////////////////////////////////////////////////////////////////////////////
    public partial class StatsWindow: Form
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        public StatsWindow()
        {
            InitializeComponent();
            myStats = new Dictionary<string, Stat>();

            foreach(ChartArea ca in myChart.ChartAreas)
            {
                myChartAreaBox.Items.Add(ca.Name);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void SetStats(string[] statId)
        {
            foreach(string id in statId)
            {
                Stat s = new Stat(id);

                myStats.Add(id, s);
                myStatsBox.Items.Add(s);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void UpdateStats(string[] data)
        {
            for(int i = 0; i < data.Length; i += 5)
            {
                Stat st = myStats[data[i]];

                // If the statistic data series does not exist, create it now.
                if(myChart.Series.IsUniqueName(st.Id))
                {
                    Series series = myChart.Series.Add(st.Id);
                    series.ChartArea = "ChartArea1";
                    series.ChartType = SeriesChartType.Spline;
                    series.SetCustomProperty("LineTension", "0.2");
                    series.ShadowOffset = 2;
                    series.BorderWidth = 2;
                }

                st.Text = st.Id + "(cur:" + data[i + 1] + " min:" + data[i + 2] + " max:" + data[i + 3] + "avg:" + data[i + 4] + ")";

                myChart.Series[st.Id].Points.Add(double.Parse(data[i + 1]));
            }
            myStatsBox.Refresh();

        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void StatsWindow_Load(object sender, EventArgs e)
        {
            myConnection = MainWindow.Instance.Connection;

            // Send a stats request message.
            myConnection.SendMessage("strq");
        }

        private Connection myConnection;
        private Dictionary<string, Stat> myStats;

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myStatsBox_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            // Any time the checked state changes, reset all the series
            myChart.Series.Clear();

            Stat s = (Stat)myStatsBox.Items[e.Index];
            if(e.NewValue == CheckState.Checked)
            {
                s.Enabled = true;
            }
            else
            {
                s.Enabled = false;
            }

            // Send request for new set of statistics
            string statIds = "";
            foreach(Stat st in myStats.Values)
            {
                if(st.Enabled)
                {
                    statIds += st.Id + " ";
                }
            }
            statIds = statIds.TrimEnd();

            myConnection.SendMessage("sten", statIds);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myTimer_Tick(object sender, EventArgs e)
        {
            // Request statistics update
            myConnection.SendMessage("stup");
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myNewAreaButton_Click(object sender, EventArgs e)
        {
            int nextca = myChart.ChartAreas.Count + 1;
            myChart.ChartAreas.Add("ChartArea" + nextca);
            myChartAreaBox.Items.Clear();
            foreach(ChartArea ca in myChart.ChartAreas)
            {
                myChartAreaBox.Items.Add(ca.Name);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myChartAreaBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            Stat s = (Stat)myStatsBox.SelectedItem;
            if(s != null && s.Enabled)
            {
                string chartAreaId = (string)myChartAreaBox.SelectedItem;
                myChart.Series[s.Id].ChartArea = chartAreaId;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myDeleteAreasButton_Click(object sender, EventArgs e)
        {
            for(int i = 0; i < myStats.Count; i++)
            {
                myStatsBox.SetItemChecked(i, false);
            }
            myChart.Series.Clear();
            myChart.ChartAreas.Clear();
            myChartAreaBox.Items.Clear();
            myChart.ChartAreas.Add("ChartArea1");
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myStatsBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            Stat s = (Stat)myStatsBox.SelectedItem;
            if(s != null)
            {
                myStatLabel.Text = "Options for " + s.Id;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    class Stat
    {
        public Stat(string id)
        {
            Id = id;
            Text = id;
        }

        public string Id;
        public string Text;
        public bool Enabled;
        public override string ToString()
        {
            return Text;
        }
    }
}

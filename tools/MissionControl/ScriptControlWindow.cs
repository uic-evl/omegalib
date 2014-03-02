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
    public partial class ScriptControlWindow: Form
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        public ScriptControlWindow(string script)
        {
            myVariables = new Dictionary<string, string>();
            InitializeComponent();

            string[] lines = script.Split('\n');
            string parsedScript = "";
            foreach(string line in lines)
            {
                if(line.StartsWith("%slider"))
                {
                    string[] args = line.Split(' ');
                    createSlider(args);
                }
                else if(line.StartsWith("%color"))
                {
                    string[] args = line.Split(' ');
                    createColorChooser(args);
                }
                else if(line.StartsWith("%%"))
                {
                    // just ignore
                }
                else
                {
                    parsedScript += line + '\n';
                }
            }

            myScript = parsedScript;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void createSlider(string[] args)
        {
            if(args.Length == 5)
            {
                Label sliderLabel = new Label();
                sliderLabel.Text = args[1];
                myLayout.Controls.Add(sliderLabel);

                TrackBar slider = new TrackBar();
                slider.Minimum = int.Parse(args[2]);
                slider.Maximum = int.Parse(args[3]);
                slider.Value = int.Parse(args[4]);
                slider.Name = args[1];
                slider.ValueChanged += new EventHandler(slider_ValueChanged);

                slider.Width = myLayout.Width;

                myLayout.Controls.Add(slider);

                myVariables.Add(args[1], slider.Value.ToString());
            }
            else
            {
                MessageBox.Show("Not enough arguments to create a slider control. Syntax is %slider <name> <min> <max> <value>", "Syntax Error", MessageBoxButtons.OK);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void createColorChooser(string[] args)
        {
            if(args.Length == 3)
            {
                Label colorLabel = new Label();
                colorLabel.Text = args[1];
                myLayout.Controls.Add(colorLabel);

                Button colorButton = new Button();
                colorButton.Text = "";
                colorButton.Name = args[1];
                colorButton.Click += new EventHandler(colorButton_Click);
                colorButton.Width = myLayout.Width;
                myLayout.Controls.Add(colorButton);

                Color col = ColorTranslator.FromHtml(args[2]);
                colorButton.BackColor = col;

                myVariables.Add(args[1], "\"" + ColorTranslator.ToHtml(col) + "\"");
            }
            else
            {
                MessageBox.Show("Not enough arguments to create a color chooser control. Syntax is %color <name> <defaultColor>", "Syntax Error", MessageBoxButtons.OK);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        void colorButton_Click(object sender, EventArgs e)
        {
            Button btn = (Button)sender;
            DialogResult res = myColorDialog.ShowDialog(this);
            if(res == System.Windows.Forms.DialogResult.OK)
            {
                // Add trailing FF for alpha component.
                Color c = myColorDialog.Color;
                string colorString = "#" + c.R.ToString("X2") + c.G.ToString("X2") + c.B.ToString("X2") + "FF";
                myVariables[btn.Name] = "\"" + colorString + "\"";
                btn.BackColor = myColorDialog.Color;
                runScript();
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        void slider_ValueChanged(object sender, EventArgs e)
        {
            TrackBar slider = (TrackBar)sender;
            myVariables[slider.Name] = slider.Value.ToString();
            runScript();
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        void runScript()
        {
            string parsedScript = myScript;
            // substitute variables in the script
            foreach(KeyValuePair<string, string> var in myVariables)
            {
                parsedScript = parsedScript.Replace("%" + var.Key + "%", var.Value);
            }
            MainWindow.Instance.Connection.SendCommand(parsedScript);
        }

        private string myScript;
        Dictionary<string, string> myVariables;
    }
}

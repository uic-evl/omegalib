using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using FastColoredTextBoxNS;

namespace MissionControl
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    public partial class MainWindow: Form
    {
        TextStyle infoStyle = new TextStyle(Brushes.White, null, FontStyle.Regular);
        TextStyle warningStyle = new TextStyle(Brushes.BurlyWood, null, FontStyle.Regular);
        TextStyle errorStyle = new TextStyle(Brushes.Red, null, FontStyle.Regular);

        ///////////////////////////////////////////////////////////////////////////////////////////
        public static MainWindow Instance
        {
            get
            {
                return myInstance;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public MainWindow()
        {
            myInstance = this;
            InitializeComponent();

            myConnection = new Connection();

            //create autocomplete popup menu
            myPromptPopupMenu = new AutocompleteMenu(myPromptBox);
            myPromptPopupMenu.MinFragmentLength = 1;
            //size of popupmenu
            myPromptPopupMenu.Items.MaximumSize = new System.Drawing.Size(300, 400);
            myPromptPopupMenu.Items.Width = 300;

            myAutocompletionList = new List<string>();

            // Load macros
            myMacros = new List<Macro>();
            int i = 0;
            foreach(string macroShortcut in Properties.Settings.Default.MacroShortcuts)
            {
                Macro m = new Macro();
                m.Shortcut = macroShortcut;
                m.Script = Properties.Settings.Default.MacroScripts[i++];
                myMacros.Add(m);
            }

            myConnection.Connect("localhost", 22500);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public List<Macro> Macros
        {
            get
            {
                return myMacros;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public Connection Connection
        {
            get
            {
                return myConnection;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public StatsWindow StatsWindow
        {
            get
            {
                return myStatsWindow;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public List<string> AutocompletionList
        {
            get
            {
                return myAutocompletionList;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void AddAutocompletionItem(string item)
        {
            if(!myAutocompletionList.Contains(item))
            {
                myAutocompletionList.Add(item);
                myPromptPopupMenu.Items.SetAutocompleteItems(myAutocompletionList);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        public void PrintMessage(string message)
        {
            //some stuffs for best performance
            myOutputBox.BeginUpdate();
            myOutputBox.Selection.BeginUpdate();
            //remember user selection
            var userSelection = myOutputBox.Selection.Clone();
            //goto end of the text
            myOutputBox.Selection.Start = myOutputBox.LinesCount > 0 ? new Place(myOutputBox[myOutputBox.LinesCount - 1].Count, myOutputBox.LinesCount - 1) : new Place(0, 0);
            //add text with predefined style
            myOutputBox.InsertText(message, infoStyle);
            //restore user selection
            if(!userSelection.IsEmpty || userSelection.Start.iLine < myOutputBox.LinesCount - 2)
            {
                myOutputBox.Selection.Start = userSelection.Start;
                myOutputBox.Selection.End = userSelection.End;
            }
            else
            {
                myOutputBox.DoCaretVisible();//scroll to end of the text
            }
            //
            myOutputBox.Selection.EndUpdate();
            myOutputBox.EndUpdate();
            myOutputBox.GoEnd();
        }

        private static MainWindow myInstance;
        private Connection myConnection;
        private List<string> myAutocompletionList;
        private AutocompleteMenu myPromptPopupMenu;

        private List<Macro> myMacros;
        private MacroWindow myMacroWindow;
        private StatsWindow myStatsWindow;

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ConnectWindow connectWindow = new ConnectWindow(myConnection);
            connectWindow.ShowDialog(this);
            if(myConnection.Connected)
            {
                myConnection.SendMessage("help");
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myConnectionPoller_Tick(object sender, EventArgs e)
        {
            if(myConnection.Connected)
            {
                myConnection.Poll();
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myPromptBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            if(e.KeyChar == (char)Keys.Return)
            {
                e.Handled = true;
                myConnection.SendCommand(myPromptBox.Text);

                if(!myAutocompletionList.Contains(myPromptBox.Text))
                {
                    myAutocompletionList.Add(myPromptBox.Text);
                    myPromptPopupMenu.Items.SetAutocompleteItems(myAutocompletionList);
                }

                myPromptBox.Text = "";
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void macroEditorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(myMacroWindow == null)
            {
                myMacroWindow = new MacroWindow();
            }
            if(!myMacroWindow.Visible)
            {
                myMacroWindow.Show(this);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void MainWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            // save macros
            Properties.Settings.Default.MacroShortcuts.Clear();
            Properties.Settings.Default.MacroScripts.Clear();
            foreach(Macro m in myMacros)
            {
                Properties.Settings.Default.MacroShortcuts.Add(m.Shortcut);
                Properties.Settings.Default.MacroScripts.Add(m.Script);
            }
            Properties.Settings.Default.Save();
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void statsViewerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(myStatsWindow == null)
            {
                myStatsWindow = new StatsWindow();
            }
            if(!myStatsWindow.Visible)
            {
                myStatsWindow.Show(this);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    public class Macro
    {
        public string Shortcut;
        public string Script;

        public override string ToString()
        {
            return Shortcut;
        }
    }
}

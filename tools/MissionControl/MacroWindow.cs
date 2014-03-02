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
    public partial class MacroWindow: Form
    {
        TextStyle preprocessorStyle = new TextStyle(Brushes.DarkOrange, null, FontStyle.Bold);
        TextStyle keywordStyle = new TextStyle(Brushes.DarkBlue, null, FontStyle.Bold);
        string keywordList;
        string preprocessorList;

        ///////////////////////////////////////////////////////////////////////////////////////////
        public MacroWindow()
        {
            InitializeComponent();

            //create autocomplete popup menu
            AutocompleteMenu popupMenu = new AutocompleteMenu(myScriptBox);
            popupMenu.MinFragmentLength = 1;
            //size of popupmenu
            popupMenu.Items.MaximumSize = new System.Drawing.Size(300, 400);
            popupMenu.Items.Width = 300;
            popupMenu.Items.SetAutocompleteItems(MainWindow.Instance.AutocompletionList);


            List<string> autocompletionList = MainWindow.Instance.AutocompletionList;
            keywordList = "";
            foreach(string item in autocompletionList)
            {
                string[] tokens = item.Split('(');
                keywordList += tokens[0] + "|";
            }
            keywordList = keywordList.Trim('|');
            keywordList = @"\b(" + keywordList + @")\b";

            preprocessorList = @"(%%|%slider|%color|%button)";
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void MacroWindow_Load(object sender, EventArgs e)
        {
            foreach(Macro m in MainWindow.Instance.Macros)
            {
                myMacroList.Items.Add(m);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myMacroList_SelectedIndexChanged(object sender, EventArgs e)
        {
            Macro m = (Macro)myMacroList.SelectedItem;
            if(m != null)
            {
                myShortcutBox.Text = m.Shortcut;
                myScriptBox.Text = m.Script;

                mySaveButton.Enabled = false;
                myRunButton.Enabled = true;
                myDeleteButton.Enabled = true;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myShortcutBox_TextChanged(object sender, EventArgs e)
        {
            mySaveButton.Enabled = true;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void mySaveButton_Click(object sender, EventArgs e)
        {
            Macro newMacro = new Macro();
            newMacro.Script = myScriptBox.Text;
            newMacro.Shortcut = myShortcutBox.Text;

            MainWindow.Instance.Macros.Add(newMacro);
            myMacroList.Items.Add(newMacro);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myDeleteButton_Click(object sender, EventArgs e)
        {
            myShortcutBox.Text = "";
            myScriptBox.Text = "";

            Macro oldMacro = (Macro)myMacroList.SelectedItem;
            myMacroList.Items.Remove(oldMacro);
            MainWindow.Instance.Macros.Remove(oldMacro);

            mySaveButton.Enabled = false;
            myRunButton.Enabled = false;
            myDeleteButton.Enabled = false;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myRunButton_Click(object sender, EventArgs e)
        {
            Connection conn = MainWindow.Instance.Connection;
            string script = myScriptBox.Text;

            if(script.StartsWith("%%"))
            {
                ScriptControlWindow scw = new ScriptControlWindow(script);
                scw.Show();
            }
            else
            {
                conn.SendCommand(myScriptBox.Text);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        private void myScriptBox_TextChanged(object sender, FastColoredTextBoxNS.TextChangedEventArgs e)
        {
            Macro macro = (Macro)myMacroList.SelectedItem;
            macro.Script = myScriptBox.Text;

            //clear old styles of chars
            e.ChangedRange.ClearStyle(keywordStyle, preprocessorStyle);
            e.ChangedRange.SetStyle(keywordStyle, keywordList, System.Text.RegularExpressions.RegexOptions.IgnoreCase);
            e.ChangedRange.SetStyle(preprocessorStyle, preprocessorList, System.Text.RegularExpressions.RegexOptions.IgnoreCase);
        }
    }
}

// SPDX-FileCopyrightText: Copyright (c) 2022 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using Microsoft.Win32;
using System;
using System.Resources;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using static VietTypeConfig.VietTypeRegistrar;

namespace VietTypeConfig {
    public partial class Form1 : Form {
        private readonly ResourceManager rm = new ResourceManager(typeof(Form1));
        private readonly bool oobe = false;

        public Form1(string[] args) {
            if (args.Length > 0) {
                oobe = string.Equals(args[0], "-oobe", StringComparison.InvariantCultureIgnoreCase);
            }
            InitializeComponent();
            udOptimizeMultilang.MouseWheel += (sender, e) => ((HandledMouseEventArgs)e).Handled = true;
        }

        private void Form1_Load(object sender, EventArgs e) {
            var settings = Settings.LoadSettings();
            settingsBindingSource.DataSource = settings;
            try {
                UpdateEnabled();
            } catch (Exception ex) {
                btnEnable.Enabled = false;
                MessageBox.Show(string.Format(rm.GetString("cannotFindLibraryMessage"), ex.Message), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
            cbVietnameseUI.Checked = Thread.CurrentThread.CurrentUICulture.TwoLetterISOLanguageName == "vi";
            cbVietnameseUI.CheckedChanged += cbVietnameseUI_CheckedChanged;
        }

        private void CloseForm(object sender, EventArgs e) {
            Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e) {
            var settings = settingsBindingSource.DataSource as Settings;
            if (DialogResult == DialogResult.OK && settings != null) {
                try {
                    Settings.SaveSettings(settings);
                } catch (Exception ex) {
                    MessageBox.Show(string.Format(rm.GetString("cannotSaveSettingsMessage"), ex.Message), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void btnAbout_Click(object sender, EventArgs e) {
            var message = string.Format(rm.GetString("aboutMessage"), Version.ProductVersion, Version.VcsRevision);
            MessageBox.Show(message, rm.GetString("aboutTitle"), MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void UpdateEnabled() {
            var activated = IsProfileActivated();
            btnEnable.Enabled = activated >= 0;
            btnEnable.Text = (activated == S_OK) ? rm.GetString("btnEnable_DisableText") : rm.GetString("btnEnable.Text");
        }

        private void btnEnable_Click(object sender, EventArgs e) {
            Activate(IsProfileActivated() != S_OK);
        }

        private void cbVietnameseUI_CheckedChanged(object sender, EventArgs e) {
            try {
                using (var regKey = Registry.CurrentUser.CreateSubKey(Settings.Subkey)) {
                    regKey.SetValue("ui_language", cbVietnameseUI.Checked ? "vi" : "en");
                    Application.Restart();
                }
            } catch (Exception ex) {
                MessageBox.Show(string.Format(rm.GetString("cannotSaveSettingsMessage"), ex.Message), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void Activate(bool newState) {
            var result = newState ? ActivateProfiles() : DeactivateProfiles();
            if (result == S_OK) {
                if (newState) {
                    if (MessageBox.Show(rm.GetString("setDefaultQuestion"), "VietType", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
                        try {
                            Settings.SetDefault();
                        } catch (Exception ex) {
                            MessageBox.Show(string.Format(rm.GetString("cannotSaveSettingsMessage"), ex.Message), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                    }
                    MessageBox.Show(rm.GetString("enableSuccessMessage"), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Information);
                } else {
                    MessageBox.Show(rm.GetString("disableSuccessMessage"), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            } else {
                var message = Marshal.GetExceptionForHR(result).Message;
                MessageBox.Show(string.Format(rm.GetString("cannotConfigureMessage"), message), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            UpdateEnabled();
        }

        private void Form1_Shown(object sender, EventArgs e) {
            if (IsProfileActivated() != S_OK) {
                if (oobe || MessageBox.Show(rm.GetString("notEnabledQuestion"), "VietType", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
                    Activate(true);
                }
            }
        }
    }
}

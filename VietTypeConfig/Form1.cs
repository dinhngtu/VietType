using System;
using System.Resources;
using System.Runtime;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using VietTypeConfig.Properties;
using static VietTypeConfig.VietTypeRegistrar;

namespace VietTypeConfig {
    public partial class Form1 : Form {
        private readonly ResourceManager rm = new ResourceManager(typeof(Form1));

        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            var settings = Settings.LoadSettings();
            settingsBindingSource.DataSource = settings;
            cbOptimizeMultilang.SelectedIndex = settings.OptimizeMultilang;
            try {
                UpdateEnabled();
            } catch (Exception ex) {
                btnEnable.Enabled = false;
                MessageBox.Show(string.Format(rm.GetString("cannotFindLibraryMessage"), ex.Message), "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
        }

        private void CloseForm(object sender, EventArgs e) {
            Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e) {
            var settings = settingsBindingSource.DataSource as Settings;
            if (DialogResult == DialogResult.OK && settings != null) {
                settings.OptimizeMultilang = cbOptimizeMultilang.SelectedIndex;
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

        private void Activate(bool newState) {
            var result = newState ? ActivateProfiles() : DeactivateProfiles();
            if (result == S_OK) {
                if (newState) {
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
                if (MessageBox.Show(rm.GetString("notEnabledQuestion"), "VietType", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
                    Activate(true);
                }
            }
        }
    }
}

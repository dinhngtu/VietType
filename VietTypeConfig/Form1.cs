using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

using static VietTypeConfig.Constants;

namespace VietTypeConfig {
    public partial class Form1 : Form {
        private static readonly Guid CLSID_Registrar = Guid.Parse("{F912E34C-E4CF-4C66-884C-2D54D28154DC}");

        readonly Lazy<IVietTypeRegistrar> registrar = new Lazy<IVietTypeRegistrar>(() => {
            return Activator.CreateInstance(Type.GetTypeFromCLSID(CLSID_Registrar, true)) as IVietTypeRegistrar;
        });

        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            settingsBindingSource.DataSource = Settings.LoadSettings();
            UpdateEnabled();
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
                    MessageBox.Show($"Cannot save settings: {ex.Message}", "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void btnAbout_Click(object sender, EventArgs e) {
            var message = $"VietType {Version.ProductVersion} ({Version.VcsRevision})\n" +
                "Copyright © 2018 Dinh Ngoc Tu.\n" +
                "VietType is free software, licensed under the GNU General Public License. " +
                "VietType is distributed WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY " +
                "or FITNESS FOR A PARTICULAR PURPOSE.See the included LICENSE file for more details.";
            MessageBox.Show(message, "About VietType", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void UpdateEnabled() {
            try {
                var activated = registrar.Value.IsProfileActivated();
                btnEnable.Enabled = activated >= 0;
                btnEnable.Text = (activated == S_OK) ? "&Disable VietType" : "&Enable VietType";
            } catch (Exception ex) {
                btnEnable.Enabled = false;
                MessageBox.Show($"Cannot find VietType library: {ex.Message}", "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnEnable_Click(object sender, EventArgs e) {
            var activated = registrar.Value.IsProfileActivated();
            var result = (activated == S_OK) ? registrar.Value.DeactivateProfiles() : registrar.Value.ActivateProfiles();
            if (result == S_OK) {
                if (activated == S_OK) {
                    MessageBox.Show($"Successfully disabled VietType.", "VietType", MessageBoxButtons.OK, MessageBoxIcon.Information);
                } else {
                    MessageBox.Show($"Successfully enabled VietType. Switch to VietType in the language bar to start using it.", "VietType", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            } else {
                var message = Marshal.GetExceptionForHR(result).Message;
                MessageBox.Show($"Cannot configure VietType: {message}", "VietType", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            UpdateEnabled();
        }
    }
}

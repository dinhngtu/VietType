using System;
using System.Windows.Forms;

namespace VietTypeConfig {
    public partial class Form1 : Form {
        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            settingsBindingSource.DataSource = Settings.LoadSettings();
        }

        private void CloseForm(object sender, EventArgs e) {
            Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e) {
            var settings = settingsBindingSource.DataSource as Settings;
            if (DialogResult == DialogResult.OK && settings != null) {
                Settings.SaveSettings(settings);
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
    }
}

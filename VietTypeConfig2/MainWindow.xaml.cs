// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using System.Runtime.InteropServices;
using System.Windows;

namespace VietTypeConfig2 {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        public MainWindow() {
            InitializeComponent();
            var vm = DataContext as MainWindowViewModel;
            vm.RequestClose += Vm_RequestClose;
            vm.RequestAbout += Vm_RequestAbout;
            vm.ToggleEnabledCompleted += Vm_ToggleEnabledCompleted;
        }

        private void Vm_RequestClose(object sender, System.EventArgs e) {
            Close();
        }

        private void Vm_RequestAbout(object sender, System.EventArgs e) {
            MessageBox.Show(
                string.Format(
                    "VietType {0} ({1})\nCopyright © 2018 Dinh Ngoc Tu.\nVietType is free software, licensed under the GNU General Public License. VietType is distributed WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the included LICENSE file for more details.",
                    Version.ProductVersion, Version.VcsRevision),
                "VietType",
                MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private void Vm_ToggleEnabledCompleted(object sender, MainWindowViewModel.ToggleEventArgs e) {
            if (e.ErrorCode == 0) {
                if (e.IsEnabled) {
                    MessageBox.Show("Successfully enabled VietType. Switch to VietType in the language bar to start using it.", "VietType", MessageBoxButton.OK, MessageBoxImage.Information);
                } else {
                    MessageBox.Show("Successfully disabled VietType.", "VietType", MessageBoxButton.OK, MessageBoxImage.Information);
                }
            } else {
                var message = Marshal.GetExceptionForHR(e.ErrorCode).Message;
                MessageBox.Show(string.Format("Cannot configure VietType: {0}", message), "VietType", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}

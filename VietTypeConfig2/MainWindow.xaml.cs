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
                string.Format(LocalizationManager.Instance["MainWindow_AboutFormat"], Version.ProductVersion, Version.VcsRevision),
                LocalizationManager.Instance["MainWindow_ProgramName"],
                MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private void Vm_ToggleEnabledCompleted(object sender, MainWindowViewModel.ToggleEventArgs e) {
            if (e.ErrorCode == 0) {
                if (e.IsEnabled) {
                    MessageBox.Show(
                        LocalizationManager.Instance["MainWindow_EnableOkText"],
                        LocalizationManager.Instance["MainWindow_ProgramName"],
                        MessageBoxButton.OK, MessageBoxImage.Information);
                } else {
                    MessageBox.Show(
                        LocalizationManager.Instance["MainWindow_DisableOkText"],
                        LocalizationManager.Instance["MainWindow_ProgramName"],
                        MessageBoxButton.OK, MessageBoxImage.Information);
                }
            } else {
                var message = Marshal.GetExceptionForHR(e.ErrorCode).Message;
                MessageBox.Show(
                    string.Format(LocalizationManager.Instance["MainWindow_ToggleFailFormat"], message),
                    LocalizationManager.Instance["MainWindow_ProgramName"],
                    MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}

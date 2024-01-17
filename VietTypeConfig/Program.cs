// SPDX-FileCopyrightText: Copyright (c) 2022 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using Microsoft.Win32;
using System;
using System.Globalization;
using System.Threading;
using System.Windows.Forms;

namespace VietTypeConfig {
    internal static class Program {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args) {
            using (var regKey = Registry.CurrentUser.CreateSubKey(Settings.Subkey)) {
                var uiLanguage = regKey.GetValue("ui_language") as string;
                if (uiLanguage != null) {
                    try {
                        Thread.CurrentThread.CurrentUICulture = new CultureInfo(uiLanguage);
                    } catch { }
                }
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1(args));
        }
    }
}

// SPDX-License-Identifier: GPL-3.0-only

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
        static void Main() {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}

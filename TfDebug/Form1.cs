using System.Runtime.InteropServices;

namespace TfDebug {
    public partial class Form1 : Form {
        readonly NativeWindow _monitor;

        public Form1() {
            InitializeComponent();
            _monitor = new ImeMonitor(textBox1.Handle, LogMessage);
            button1.Click += (s, e) => richTextBox1.Clear();
            FormClosing += (s, e) => _monitor.ReleaseHandle();
        }

        void LogMessage(string message) {
            if (richTextBox1.InvokeRequired) {
                richTextBox1.Invoke(new Action<string>(LogMessage), message);
                return;
            }
            try {
                richTextBox1.AppendText(message + Environment.NewLine);
                // ScrollToCaret can sometimes fail if the control is busy or handles are being recreated
                richTextBox1.SelectionStart = richTextBox1.Text.Length;
                richTextBox1.ScrollToCaret();
            }
            catch (COMException) {
                // Ignore transient COM errors during rapid updates
            }
        }
    }
}

namespace TfDebug {
    class ImeMonitor : NativeWindow {
        readonly Action<string> _logger;

        public ImeMonitor(IntPtr handle, Action<string> logger) {
            AssignHandle(handle);
            _logger = logger;
        }

        protected override void WndProc(ref Message m) {
            switch (m.Msg) {
                case 0x0200: // WM_MOUSEMOVE
                case 0x0215: // WM_CAPTURECHANGED
                case 0x0113: // WM_TIMER
                case 0x0020: // WM_SETCURSOR
                    base.WndProc(ref m);
                    return;

                case int msg when (msg >= 0x010D && msg <= 0x0110) || (msg >= 0x0281 && msg <= 0x0291): // IME Messages
                case 0x0100: // WM_KEYDOWN
                case 0x0101: // WM_KEYUP
                case 0x0102: // WM_CHAR
                    _logger($"{DateTime.Now:HH:mm:ss.fff} | MSG: {GetImeMsgName(m.Msg)} | W: {m.WParam:X} | L: {m.LParam:X} | {GetImeDetails(m.Msg, m.WParam, m.LParam)}");
                    break;
            }
            base.WndProc(ref m);
        }

        string GetImeDetails(int msg, IntPtr wParam, IntPtr lParam) {
            switch (msg) {
                case 0x010F: // WM_IME_COMPOSITION
                    var l = (long)lParam;
                    var flags = new List<string>();
                    if ((l & 0x0001) != 0) flags.Add("GCS_COMPREADSTR");
                    if ((l & 0x0002) != 0) flags.Add("GCS_COMPREADATTR");
                    if ((l & 0x0004) != 0) flags.Add("GCS_COMPREADCLAUSE");
                    if ((l & 0x0008) != 0) flags.Add("GCS_COMPSTR");
                    if ((l & 0x0010) != 0) flags.Add("GCS_COMPATTR");
                    if ((l & 0x0020) != 0) flags.Add("GCS_COMPCLAUSE");
                    if ((l & 0x0080) != 0) flags.Add("GCS_CURSORPOS");
                    if ((l & 0x0100) != 0) flags.Add("GCS_DELTASTART");
                    if ((l & 0x0200) != 0) flags.Add("GCS_RESULTREADSTR");
                    if ((l & 0x0400) != 0) flags.Add("GCS_RESULTREADCLAUSE");
                    if ((l & 0x0800) != 0) flags.Add("GCS_RESULTSTR");
                    if ((l & 0x1000) != 0) flags.Add("GCS_RESULTCLAUSE");
                    if ((l & 0x2000) != 0) flags.Add("CS_INSERTCHAR");
                    if ((l & 0x4000) != 0) flags.Add("CS_NOMOVECARET");
                    return string.Join("|", flags);

                case 0x0282: // WM_IME_NOTIFY
                    return (long)wParam switch {
                        0x0001 => "IMN_CLOSESTATUSWINDOW",
                        0x0002 => "IMN_OPENSTATUSWINDOW",
                        0x0003 => "IMN_CHANGECANDIDATE",
                        0x0004 => "IMN_CLOSECANDIDATE",
                        0x0005 => "IMN_OPENCANDIDATE",
                        0x0006 => "IMN_SETCONVERSIONMODE",
                        0x0007 => "IMN_SETSENTENCEMODE",
                        0x0008 => "IMN_SETOPENSTATUS",
                        0x0009 => "IMN_SETCANDIDATEPOS",
                        0x000A => "IMN_SETCOMPOSITIONFONT",
                        0x000B => "IMN_SETCOMPOSITIONWINDOW",
                        0x000C => "IMN_SETSTATUSWINDOWPOS",
                        0x000D => "IMN_GUIDELINE",
                        0x000E => "IMN_PRIVATE",
                        _ => $"IMN_0x{(long)wParam:X}"
                    };

                case 0x0281: // WM_IME_SETCONTEXT
                    var isc = (long)lParam;
                    /*
                    var iscFlags = new List<string>();
                    if ((isc & 0x80000000) != 0) iscFlags.Add("ISC_SHOWUICOMPOSITIONWINDOW");
                    if ((isc & 0x40000000) != 0) iscFlags.Add("ISC_SHOWUIGUIDELINE");
                    if ((isc & 0x00000001) != 0) iscFlags.Add("ISC_SHOWUICANDIDATEWINDOW(0)");
                    if ((isc & 0x00000002) != 0) iscFlags.Add("ISC_SHOWUICANDIDATEWINDOW(1)");
                    if ((isc & 0x00000004) != 0) iscFlags.Add("ISC_SHOWUICANDIDATEWINDOW(2)");
                    if ((isc & 0x00000008) != 0) iscFlags.Add("ISC_SHOWUICANDIDATEWINDOW(3)");
                    */
                    return $"Active: {wParam != IntPtr.Zero}, Flags: 0x{isc:X}";

                case 0x0285: // WM_IME_SELECT
                    return $"Select: {wParam != IntPtr.Zero}, hKL: {lParam:X}";

                case 0x0100: // WM_KEYDOWN
                case 0x0101: // WM_KEYUP
                case 0x0290: // WM_IME_KEYDOWN
                case 0x0291: // WM_IME_KEYUP
                    return $"VK: {(System.Windows.Forms.Keys)(long)wParam}";

                case 0x0102: // WM_CHAR
                case 0x0286: // WM_IME_CHAR
                    return $"Char: '{(char)(long)wParam}'";

                default:
                    return "";
            }
        }

        string GetImeMsgName(int msg) => msg switch {
            0x0100 => "WM_KEYDOWN",
            0x0101 => "WM_KEYUP",
            0x0102 => "WM_CHAR",
            0x010D => "WM_IME_STARTCOMPOSITION",
            0x010E => "WM_IME_ENDCOMPOSITION",
            0x010F => "WM_IME_COMPOSITION",
            0x0201 => "WM_LBUTTONDOWN",
            0x0202 => "WM_LBUTTONUP",
            0x0281 => "WM_IME_SETCONTEXT",
            0x0282 => "WM_IME_NOTIFY",
            0x0283 => "WM_IME_CONTROL",
            0x0284 => "WM_IME_COMPOSITIONFULL",
            0x0285 => "WM_IME_SELECT",
            0x0286 => "WM_IME_CHAR",
            0x0288 => "WM_IME_REQUEST",
            0x0290 => "WM_IME_KEYDOWN",
            0x0291 => "WM_IME_KEYUP",
            _ => $"0x{msg:X4}"
        };
    }
}

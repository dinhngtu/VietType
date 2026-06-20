using System;
using System.Runtime.InteropServices;

namespace VietTypeConfig2 {
    internal class VersionDetector {
        public static Lazy<VersionDetector> Instance { get; } = new Lazy<VersionDetector>(() => {
            var instance = new VersionDetector();
            instance.DetectVersion();
            return instance;
        });

        VIETTYPE_INFORMATION_V1 _info;

        void DetectVersion() {
            unsafe {
                fixed (VIETTYPE_INFORMATION_V1* pinfo = &_info) {
                    var hr = VietTypeRegistrar.GetVersionInfo(pinfo, (uint)sizeof(VIETTYPE_INFORMATION_V1));
                    if (hr < 0) {
                        throw new ExternalException("GetVersionInfo failed", hr);
                    }
                }

                if (_info.Version < 1 || _info.MaxVersion < _info.Version) {
                    throw new Exception("GetVersionInfo returned unexpected result");
                }
            }
        }

        public uint MaxVersion => _info.MaxVersion;
        public uint Version => _info.Version;
        public uint? DefaultPkToggle => _info.Version >= 1 ? _info.DefaultPkToggle : (uint?)null;
        public ushort? LangId => _info.Version >= 1 ? _info.LangId : (ushort?)null;
    }
}

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VietTypeConfig {
    internal class Settings : INotifyPropertyChanged {
        private const string Subkey = "Software\\VietType";

        bool default_enabled = false;
        public bool DefaultEnabled {
            get {
                return default_enabled;
            }
            set {
                if (default_enabled != value) {
                    default_enabled = value;
                    OnPropertyChanged(nameof(DefaultEnabled));
                }
            }
        }

        bool oa_uy_tone1 = true;
        public bool OaUy {
            get {
                return oa_uy_tone1;
            }
            set {
                if (oa_uy_tone1 != value) {
                    oa_uy_tone1 = value;
                    OnPropertyChanged(nameof(OaUy));
                }
            }
        }

        bool accept_dd = true;
        public bool AcceptDd {
            get {
                return accept_dd;
            }
            set {
                if (accept_dd != value) {
                    accept_dd = value;
                    OnPropertyChanged(nameof(AcceptDd));
                }
            }
        }

        bool backspace_invalid = true;
        public bool BackspaceInvalid {
            get {
                return backspace_invalid;
            }
            set {
                if (backspace_invalid != value) {
                    backspace_invalid = value;
                    OnPropertyChanged(nameof(BackspaceInvalid));
                }
            }
        }

        bool backconvert_on_backspace = false;
        public bool BackconvertOnBackspace {
            get {
                return backconvert_on_backspace;
            }
            set {
                if (backconvert_on_backspace != value) {
                    backconvert_on_backspace = value;
                    OnPropertyChanged(nameof(BackconvertOnBackspace));
                }
            }
        }


        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string propName) {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        }

        private static bool? ToBool(object v) {
            if (v is int) {
                return (int)v != 0;
            } else {
                return null;
            }
        }

        public static Settings LoadSettings() {
            using (var regKey = Registry.CurrentUser.CreateSubKey(Subkey)) {
                var setting = new Settings();
                setting.DefaultEnabled = ToBool(regKey.GetValue(nameof(default_enabled))) ?? setting.DefaultEnabled;
                setting.OaUy = ToBool(regKey.GetValue(nameof(oa_uy_tone1))) ?? setting.OaUy;
                setting.AcceptDd = ToBool(regKey.GetValue(nameof(accept_dd))) ?? setting.AcceptDd;
                setting.BackspaceInvalid = ToBool(regKey.GetValue(nameof(backspace_invalid))) ?? setting.BackspaceInvalid;
                setting.BackconvertOnBackspace = ToBool(regKey.GetValue(nameof(backconvert_on_backspace))) ?? setting.BackconvertOnBackspace;
                return setting;
            }
        }

        public static void SaveSettings(Settings settings) {
            using (var regKey = Registry.CurrentUser.CreateSubKey(Subkey)) {
                regKey.SetValue(nameof(default_enabled), settings.DefaultEnabled ? 1 : 0);
                regKey.SetValue(nameof(oa_uy_tone1), settings.OaUy ? 1 : 0);
                regKey.SetValue(nameof(accept_dd), settings.AcceptDd ? 1 : 0);
                regKey.SetValue(nameof(backspace_invalid), settings.BackspaceInvalid ? 1 : 0);
                regKey.SetValue(nameof(backconvert_on_backspace), settings.BackconvertOnBackspace ? 1 : 0);
            }
        }
    }
}

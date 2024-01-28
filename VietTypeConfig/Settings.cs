// SPDX-FileCopyrightText: Copyright (c) 2022 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using Microsoft.Win32;
using System;
using System.ComponentModel;

namespace VietTypeConfig {
    internal class Settings : INotifyPropertyChanged {
        public const string Subkey = "Software\\VietType";
        private static readonly Guid CLSID_TF_ThreadMgr = Guid.Parse("{529a9e6b-6587-4f23-ab9e-9c7d683e3c50}");
        private static readonly Guid CLSID_TextService = Guid.Parse("{c0dd01a1-0deb-454b-8b42-d22ced1b4b23}");
        private static readonly Guid GUID_Profile = Guid.Parse("{8D93D10A-203B-4C5F-A122-8898EF9C56F5}");
        private const ushort TextServiceLangId = 0x42a;

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

        int optimize_multilang = 1;
        public int OptimizeMultilang {
            get {
                return optimize_multilang;
            }
            set {
                if (value >= 0 && optimize_multilang != value) {
                    optimize_multilang = value;
                    OnPropertyChanged(nameof(OptimizeMultilang));
                }
            }
        }

        bool autocorrect = false;
        public bool Autocorrect {
            get {
                return autocorrect;
            }
            set {
                if (autocorrect != value) {
                    autocorrect = value;
                    OnPropertyChanged(nameof(Autocorrect));
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

        private static int? ToInt(object v) {
            if (v is int) {
                return (int)v;
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
                setting.OptimizeMultilang = ToInt(regKey.GetValue(nameof(optimize_multilang))) ?? setting.OptimizeMultilang;
                setting.Autocorrect = ToBool(regKey.GetValue(nameof(autocorrect))) ?? setting.Autocorrect;
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
                regKey.SetValue(nameof(optimize_multilang), settings.OptimizeMultilang);
                regKey.SetValue(nameof(autocorrect), settings.Autocorrect ? 1 : 0);
            }
        }

        public static void SetDefault() {
            NativeMethods.SetInputMethodOverride(string.Format("{0:X4}:{{{1}}}{{{2}}}", TextServiceLangId, CLSID_TextService.ToString(), GUID_Profile.ToString()));
        }
    }
}

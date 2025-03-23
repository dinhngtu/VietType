// SPDX-FileCopyrightText: Copyright (c) 2022 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace VietTypeConfig2 {
    internal class AppSettings : INotifyPropertyChanged {
        private const string Subkey = "Software\\VietType";
        private static readonly Guid GUID_SystemNotifyCompartment = Guid.Parse("{B2FBD2E7-922F-4996-BE77-21085B91A8F0}");
        private static readonly Guid CLSID_TF_ThreadMgr = Guid.Parse("{529a9e6b-6587-4f23-ab9e-9c7d683e3c50}");
        private static readonly Guid CLSID_TextService = Guid.Parse("{c0dd01a1-0deb-454b-8b42-d22ced1b4b23}");
        private static readonly Guid GUID_Profile = Guid.Parse("{8D93D10A-203B-4C5F-A122-8898EF9C56F5}");
        private const ushort TextServiceLangId = 0x42a;

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

        public int typing_style = 0;
        public int TypingStyle {
            get {
                return typing_style;
            }
            set {
                if (typing_style != value) {
                    typing_style = value;
                    OnPropertyChanged(nameof(typing_style));
                }
            }
        }

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


        int show_composing_attr = 0;
        public int ShowComposingAttr {
            get {
                return show_composing_attr;
            }
            set {
                if (show_composing_attr != value) {
                    show_composing_attr = value;
                    OnPropertyChanged(nameof(ShowComposingAttr));
                }
            }
        }

        int pk_toggle = 0x100c0; // alt+~
        public int PkToggle {
            get {
                return pk_toggle;
            }
            set {
                if (pk_toggle != value) {
                    pk_toggle = value;
                    OnPropertyChanged(nameof(PkToggle));
                }
            }
        }

        public static AppSettings LoadSettings() {
            using (var regKey = Registry.CurrentUser.CreateSubKey(Subkey)) {
                var setting = new AppSettings();
                setting.TypingStyle = ToInt(regKey.GetValue(nameof(typing_style))) ?? setting.TypingStyle;
                setting.DefaultEnabled = ToBool(regKey.GetValue(nameof(default_enabled))) ?? setting.DefaultEnabled;
                setting.OaUy = ToBool(regKey.GetValue(nameof(oa_uy_tone1))) ?? setting.OaUy;
                setting.AcceptDd = ToBool(regKey.GetValue(nameof(accept_dd))) ?? setting.AcceptDd;
                setting.BackspaceInvalid = ToBool(regKey.GetValue(nameof(backspace_invalid))) ?? setting.BackspaceInvalid;
                setting.BackconvertOnBackspace = ToBool(regKey.GetValue(nameof(backconvert_on_backspace))) ?? setting.BackconvertOnBackspace;
                setting.OptimizeMultilang = ToInt(regKey.GetValue(nameof(optimize_multilang))) ?? setting.OptimizeMultilang;
                setting.Autocorrect = ToBool(regKey.GetValue(nameof(autocorrect))) ?? setting.Autocorrect;
                setting.ShowComposingAttr = ToInt(regKey.GetValue(nameof(show_composing_attr))) ?? setting.ShowComposingAttr;
                setting.PkToggle = ToInt(regKey.GetValue(nameof(pk_toggle))) ?? setting.PkToggle;
                return setting;
            }
        }

        public static void SaveSettings(AppSettings settings) {
            using (var regKey = Registry.CurrentUser.CreateSubKey(Subkey)) {
                regKey.SetValue(nameof(typing_style), settings.typing_style);
                regKey.SetValue(nameof(default_enabled), settings.DefaultEnabled ? 1 : 0);
                regKey.SetValue(nameof(oa_uy_tone1), settings.OaUy ? 1 : 0);
                regKey.SetValue(nameof(accept_dd), settings.AcceptDd ? 1 : 0);
                regKey.SetValue(nameof(backspace_invalid), settings.BackspaceInvalid ? 1 : 0);
                regKey.SetValue(nameof(backconvert_on_backspace), settings.BackconvertOnBackspace ? 1 : 0);
                regKey.SetValue(nameof(optimize_multilang), settings.OptimizeMultilang);
                regKey.SetValue(nameof(autocorrect), settings.Autocorrect ? 1 : 0);
                regKey.SetValue(nameof(show_composing_attr), settings.ShowComposingAttr);
                regKey.SetValue(nameof(pk_toggle), settings.PkToggle);

                var threadMgr = Activator.CreateInstance(Type.GetTypeFromCLSID(CLSID_TF_ThreadMgr, true)) as ITfThreadMgr;
                if (threadMgr.Activate(out uint tid) >= 0) {
                    try {
                        if (threadMgr.GetGlobalCompartment(out ITfCompartmentMgr globalMgr) >= 0 &&
                            globalMgr.GetCompartment(GUID_SystemNotifyCompartment, out ITfCompartment globalCompartment) >= 0) {
                            if (globalCompartment.GetValue(out object oldGlobal) >= 0) {
                                globalCompartment.SetValue(tid, unchecked((oldGlobal as int? ?? 0) + 1));
                            }
                        }
                    } finally {
                        threadMgr.Deactivate();
                    }
                }
            }
        }

        public static void SetDefault() {
            NativeMethods.SetInputMethodOverride(string.Format("{0:X4}:{{{1}}}{{{2}}}", TextServiceLangId, CLSID_TextService.ToString(), GUID_Profile.ToString()));
        }
    }
}

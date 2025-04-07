// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.ComponentModel;
using System.Windows.Input;

namespace VietTypeConfig2 {
    internal class MainWindowViewModel : INotifyPropertyChanged {
        public event PropertyChangedEventHandler PropertyChanged;
        readonly AppSettings _settings;

        public MainWindowViewModel() {
            _settings = AppSettings.LoadSettings();
            _activatedStatus = VietTypeRegistrar.IsProfileActivated() == VietTypeRegistrar.S_OK;
            OkCommand = new RelayCommand((_) => OnOkCommand());
            CancelCommand = new RelayCommand((_) => OnCancelCommand());
            AboutCommand = new RelayCommand((_) => OnAboutCommand());
            ToggleEnabledCommand = new RelayCommand((_) => OnToggleEnabledCommand());
        }

        void OnPropertyChanged(string propertyName) {
            var e = new PropertyChangedEventArgs(propertyName);
            PropertyChanged?.Invoke(this, e);
        }

        #region Global settings
        bool _activatedStatus;
        public bool IsEnabled => _activatedStatus;

        public string IsEnabledText {
            get {
                return IsEnabled ? "_Disable VietType" : "_Enable VietType";
            }
        }
        #endregion

        #region Engine settings
        public TypingStyles TypingStyle {
            get {
                return (TypingStyles)_settings.TypingStyle;
            }
            set {
                if (_settings.TypingStyle != (int)value && Enum.IsDefined(typeof(TypingStyles), value)) {
                    _settings.TypingStyle = (int)value;
                    OnPropertyChanged(nameof(TypingStyle));
                }
            }
        }

        public bool OaUyTone1 {
            get {
                return _settings.OaUy;
            }
            set {
                if (_settings.OaUy != value) {
                    _settings.OaUy = value;
                    OnPropertyChanged(nameof(OaUyTone1));
                }
            }
        }

        public bool AcceptDd {
            get {
                return _settings.AcceptDd;
            }
            set {
                if (_settings.AcceptDd != value) {
                    _settings.AcceptDd = value;
                    OnPropertyChanged(nameof(AcceptDd));
                }
            }
        }

        public bool BackspaceInvalid {
            get {
                return _settings.BackspaceInvalid;
            }
            set {
                if (_settings.BackspaceInvalid != value) {
                    _settings.BackspaceInvalid = value;
                    OnPropertyChanged(nameof(BackspaceInvalid));
                }
            }
        }

        public uint OptimizeMultilang {
            get {
                return (uint)_settings.OptimizeMultilang;
            }
            set {
                if (_settings.OptimizeMultilang != (int)value) {
                    _settings.OptimizeMultilang = (int)value;
                    OnPropertyChanged(nameof(OptimizeMultilang));
                }
            }
        }

        public bool Autocorrect {
            get {
                return _settings.Autocorrect;
            }
            set {
                if (_settings.Autocorrect != value) {
                    _settings.Autocorrect = value;
                    OnPropertyChanged(nameof(Autocorrect));
                }
            }
        }
        #endregion

        #region Control settings
        public bool DefaultEnabled {
            get {
                return _settings.DefaultEnabled;
            }
            set {
                if (_settings.DefaultEnabled != value) {
                    _settings.DefaultEnabled = value;
                    OnPropertyChanged(nameof(DefaultEnabled));
                }
            }
        }

        public bool BackconvertOnBackspace {
            get {
                return _settings.BackconvertOnBackspace;
            }
            set {
                if (_settings.BackconvertOnBackspace != value) {
                    _settings.BackconvertOnBackspace = value;
                    OnPropertyChanged(nameof(BackconvertOnBackspace));
                }
            }
        }

        public ToggleModifiers ToggleModifiers {
            get {
                return (ToggleModifiers)((_settings.PkToggle >> 16) & 0xffff);
            }
            set {
                if (ToggleModifiers != value && Enum.IsDefined(typeof(ToggleModifiers), value)) {
                    _settings.PkToggle = (((int)value & 0xffff) << 16) | (_settings.PkToggle & 0xffff);
                    OnPropertyChanged(nameof(ToggleModifiers));
                }
            }
        }

        public ToggleKeys ToggleKeys {
            get {
                return (ToggleKeys)(_settings.PkToggle & 0xffff);
            }
            set {
                if (ToggleKeys != value && Enum.IsDefined(typeof(ToggleKeys), value)) {
                    _settings.PkToggle = ((int)value & 0xffff) | (_settings.PkToggle & -0x10000);
                    OnPropertyChanged(nameof(ToggleKeys));
                }
            }
        }

        public ComposingAttributes ShowComposingAttr {
            get {
                return (ComposingAttributes)_settings.ShowComposingAttr;
            }
            set {
                if (_settings.ShowComposingAttr != (int)value && Enum.IsDefined(typeof(ComposingAttributes), value)) {
                    _settings.ShowComposingAttr = (int)value;
                    OnPropertyChanged(nameof(ShowComposingAttr));
                }
            }
        }
        #endregion

        #region Commands
        public event EventHandler RequestClose;

        void OnOkCommand() {
            AppSettings.SaveSettings(_settings);
            RequestClose?.Invoke(this, EventArgs.Empty);
        }
        public ICommand OkCommand { get; }

        void OnCancelCommand() {
            RequestClose?.Invoke(this, EventArgs.Empty);
        }
        public ICommand CancelCommand { get; }

        public event EventHandler RequestAbout;
        void OnAboutCommand() {
            RequestAbout?.Invoke(this, EventArgs.Empty);
        }
        public ICommand AboutCommand { get; }

        public class ToggleEventArgs : EventArgs {
            readonly int _error;
            readonly bool _enabled;
            public ToggleEventArgs(int error, bool enabled) {
                _error = error;
                _enabled = enabled;
            }
            public int ErrorCode => _error;
            public bool IsEnabled => _enabled;
        }

        public event EventHandler<ToggleEventArgs> ToggleEnabledCompleted;
        void OnToggleEnabledCommand() {
            var result = _activatedStatus ? VietTypeRegistrar.DeactivateProfiles() : VietTypeRegistrar.ActivateProfiles();
            ToggleEnabledCompleted?.Invoke(this, new ToggleEventArgs(result, !_activatedStatus));
            _activatedStatus = VietTypeRegistrar.IsProfileActivated() == VietTypeRegistrar.S_OK;
            OnPropertyChanged(nameof(IsEnabled));
            OnPropertyChanged(nameof(IsEnabledText));
        }
        public ICommand ToggleEnabledCommand { get; }
        #endregion
    }
}

// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.ComponentModel;
using System.Globalization;
using System.Resources;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Windows;

namespace VietTypeConfig2 {
    public class LocalizationManager : INotifyPropertyChanged {
        private static readonly Lazy<LocalizationManager> _instance =
            new Lazy<LocalizationManager>(() => new LocalizationManager());

        public static LocalizationManager Instance => _instance.Value;

        private readonly ResourceManager _resourceManager = LocalizedResources.Localized.ResourceManager;
        private CultureInfo _currentCulture = CultureInfo.CurrentUICulture;

        private LocalizationManager() { }

        public event PropertyChangedEventHandler PropertyChanged;

        public CultureInfo CurrentCulture {
            get {
                return _currentCulture;
            }
            set {
                if (!Equals(_currentCulture, value)) {
                    _currentCulture = value;
                    Thread.CurrentThread.CurrentUICulture = value;
                    OnPropertyChanged(nameof(CurrentCulture));
                    // Crucial: Notify that the result of the indexer has changed
                    // for ALL bindings. The string "Item[]" is the conventional name for indexers.
                    OnPropertyChanged("Item[]");
                }
            }
        }

        public string this[string key] {
            get {
                string value = _resourceManager.GetString(key, _currentCulture);
                return value ?? $"[{key}]";
            }
        }

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            // Ensure the event is raised on the UI thread if changed from background thread
            if (Application.Current?.Dispatcher != null && !Application.Current.Dispatcher.CheckAccess()) {
                Application.Current.Dispatcher.Invoke(() => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName)));
            } else {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}

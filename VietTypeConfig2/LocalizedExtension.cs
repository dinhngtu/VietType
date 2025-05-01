// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Windows;
using System.Windows.Data;
using System.Windows.Markup;

namespace VietTypeConfig2 {
    public class LocalizedExtension : MarkupExtension {
        public string Key { get; set; }

        public LocalizedExtension() { }

        public LocalizedExtension(string key) {
            Key = key;
        }

        public override object ProvideValue(IServiceProvider serviceProvider) {
            if (string.IsNullOrEmpty(Key)) {
                return DependencyProperty.UnsetValue;
            }

            var binding = new Binding($"[{Key}]") {
                Source = LocalizationManager.Instance,
                Mode = BindingMode.OneWay,
            };

            // IMPORTANT: Return the result of the binding expression, not the binding object itself (usually).
            // For most scenarios (like setting TextBlock.Text), you need the binding evaluated.
            // However, if the target property itself IS a Binding (less common), you'd return 'binding'.
            // For standard DP targets, use binding.ProvideValue.
            try {
                // Let the binding infrastructure handle the value resolution
                return binding.ProvideValue(serviceProvider);
            } catch {
                return $"[{Key}]"; // Fallback on error
            }
        }
    }
}

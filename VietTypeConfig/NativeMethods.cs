// SPDX-FileCopyrightText: Copyright (c) 2022 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Runtime.InteropServices;

namespace VietTypeConfig {
    internal static class VietTypeRegistrar {
        public const int S_OK = 0;
        public const int S_FALSE = 1;

        [DllImport("VietTypeATL32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ActivateProfiles();

        [DllImport("VietTypeATL32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int DeactivateProfiles();

        [DllImport("VietTypeATL32.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int IsProfileActivated();
    }

    internal static class NativeMethods {
        [DllImport("bcp47langs.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern int SetInputMethodOverride(string TipString);
    }
}

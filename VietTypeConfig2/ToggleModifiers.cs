// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

namespace VietTypeConfig2 {
    internal enum ToggleModifiers {
        None = 0,
        Alt = 0x1,
        Control = 0x2,
        ControlAlt = 0x3,
        Shift = 0x4,
        AltShift = 0x5,
        ControlShift = 0x6,
        ControlAltShift = 0x7,
        RAlt = 0x8,
        RControl = 0x10,
        RShift = 0x20,
        LAlt = 0x40,
        LControl = 0x80,
        LShift = 0x100,
        Any = 0x400,
    }
}

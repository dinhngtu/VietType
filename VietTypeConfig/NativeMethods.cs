// SPDX-License-Identifier: GPL-3.0-only

using System;
using System.Runtime.InteropServices;

namespace VietTypeConfig {
    [ComImport]
    [Guid("aa80e801-2021-11d2-93e0-0060b067b86e")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface ITfThreadMgr {
        int Activate(out uint tid);
        int Deactivate();
        int CreateDocumentMgr(out object ppdim);
        int EnumDocumentMgrs(out object ppEnum);
        int GetFocus(out object ppdimFocus);
        int SetFocus(out object pdimFocus);
        int AssociateFocus(IntPtr hwnd, ref object pdimNew, out object ppdimPrev);
        int IsThreadFocus(out bool pfThreadFocus);
        int GetFunctionProvider(ref Guid clsid, out object ppFuncProv);
        int EnumFunctionProviders(out object ppEnum);
        int GetGlobalCompartment(out ITfCompartmentMgr ppCompMgr);
    }

    [ComImport]
    [Guid("7dcf57ac-18ad-438b-824d-979bffb74b7c")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface ITfCompartmentMgr {
        int GetCompartment(ref Guid rguid, out ITfCompartment compartment);
        int ClearCompartment(uint tid, ref Guid rguid);
        int EnumCompartments(out object ppEnum);
    }

    [ComImport]
    [Guid("bb08f7a9-607a-4384-8623-056892b64371")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface ITfCompartment {
        int SetValue(uint tid, ref object value);
        int GetValue(out object value);
    }

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
}

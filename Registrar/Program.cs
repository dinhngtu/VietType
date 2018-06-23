// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace ConsoleApp1 {
    class Program {
        [DllImport("VietTypeATL.dll", CallingConvention = CallingConvention.StdCall)]
        static extern uint RegisterTextServiceProfiles(ulong handle);
        [DllImport("VietTypeATL.dll", CallingConvention = CallingConvention.StdCall)]
        static extern uint UnregisterTextServiceProfiles(ulong handle);
        [DllImport("VietTypeATL.dll", CallingConvention = CallingConvention.StdCall)]
        static extern uint RegisterTextServiceCategories(ulong handle);
        [DllImport("VietTypeATL.dll", CallingConvention = CallingConvention.StdCall)]
        static extern uint UnregisterTextServiceCategories(ulong handle);

        [STAThread]
        static void Main(string[] args) {
            if (args.Length == 2) {
                if (args[0] == "register") {
                    Register(args[1]);
                } else if (args[0] == "unregister") {
                    Unregister(args[1]);
                }
            } else {
                Console.WriteLine("registrar.exe register|unregister categories|profiles");
            }
        }

        private static void Register(string regtype) {
            if (regtype == "categories") {
                Console.WriteLine(RegisterTextServiceCategories(0));
            } else if (regtype == "profiles") {
                Console.WriteLine(RegisterTextServiceProfiles(0));
            } else {
                Console.WriteLine("registrar.exe register|unregister categories|profiles");
            }
        }

        private static void Unregister(string regtype) {
            if (regtype == "categories") {
                Console.WriteLine(UnregisterTextServiceCategories(0));
            } else if (regtype == "profiles") {
                Console.WriteLine(UnregisterTextServiceProfiles(0));
            } else {
                Console.WriteLine("registrar.exe register|unregister categories|profiles");
            }
        }
    }
}

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

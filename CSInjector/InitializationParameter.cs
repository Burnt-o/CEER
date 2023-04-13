using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CSInjector
{
    // Important - the injected DLL must have a definition of this same struct that has the exact same order and size of struct members
    // Any types that are stored as pointers will not work either, so strings are out


    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct InitializationParameter
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = PInvoke.MAX_PATH)]
        public string InjectorPath;
    }
}



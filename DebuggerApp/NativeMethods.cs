using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace DebuggerApp
{
    public partial class NativeMethods
    {
        /// Return Type: void
        ///hProcess: HANDLE->void*
        ///addr: byte*
        ///size: size_t->unsigned int
        ///buf: byte*
        [DllImport("Udis86Wrapper.dll", EntryPoint = "Disassemble", CallingConvention = CallingConvention.StdCall)]
        public static extern void Disassemble(IntPtr addr, uint size, IntPtr buf);

    }
}

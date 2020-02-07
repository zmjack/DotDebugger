using NMarshal;
using NStandard;
using System;
using System.Diagnostics;

namespace DebuggerApp
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("DebuggerApp...");

            var process = Process.Start(new ProcessStartInfo
            {
                FileName = "CppConsoleApp.exe",
                UseShellExecute = true,
            });

            var memory = new NWin32.MemoryAccessor((uint)process.Id);
            var buffer = new AutoIntPtr<byte[]>(88).Then(x => x.Value = memory.Buffer(0x4113A7, 88));

            NativeMethods.Disassemble(new IntPtr(0x4113A7), 88, buffer);
        }
    }
}

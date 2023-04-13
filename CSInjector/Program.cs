using System.Runtime.InteropServices;
using System.Diagnostics;
using static CSInjector.PInvoke;
using System.Text;
using System.Threading;
using System.Collections;
using System;

namespace CSInjector
{

    internal class Program
    {
        
        static readonly string[] targetProcessNames = { "MCC-Win64-Shipping", "MCC-Win64-Shipping-Winstore" }; // MCC has two different process names depending whether user playing on Steam or Winstore
        static readonly string dllToInjectName = "CERandomiser";
        static readonly string dllToInjectPath = AppDomain.CurrentDomain.BaseDirectory.ToString() + dllToInjectName + ".dll";


        static void Main(string[] args) // args are unused
        {
            Console.WriteLine("DLL Injector running");
            try
            {
                // Search for our target process
                Process? targetProcess = FindTargetProcess(targetProcessNames);
                if (targetProcess == null) { Console.WriteLine("Waiting for MCC to open"); }
                while (targetProcess == null)
                {
                    Thread.Sleep(1000);
                    targetProcess = FindTargetProcess(targetProcessNames);
                }

                FileInfo dllToInject = new FileInfo(dllToInjectPath);
                if (!dllToInject.Exists) throw new InvalidOperationException(string.Format("DLL file {0}.dll missing!", dllToInjectName));


                // Check if the dll is already loaded in the process before we inject. In which case we want to make sure it finishes execution by calling shutdown on it
                ProcessModule? dllAlreadyInProcess = FindTargetModule(targetProcess, dllToInjectName);
                if (dllAlreadyInProcess != null) CallExternalShutdown(GetProcAddressEx(targetProcess, dllAlreadyInProcess, "Shutdown"), targetProcess, dllAlreadyInProcess);

                // Inject!
                dllAlreadyInProcess = InjectDLL(targetProcess, dllToInjectPath, dllToInjectName);
                if (dllAlreadyInProcess == null) throw new InvalidOperationException("Failed to get module Handle after injecting.");


                // Set up the initialization param we're going to pass to the injected DLL
                InitializationParameter initParam = new();
                initParam.InjectorPath = AppDomain.CurrentDomain.BaseDirectory.ToString();

                CallExternalStartup(initParam, GetProcAddressEx(targetProcess, dllAlreadyInProcess, "Startup"), targetProcess, dllAlreadyInProcess);


                // No errors so we'll let the console window close
                Console.WriteLine("Succesfully injected {0}!", dllToInjectName);
                #if DEBUG
                Console.Read();// Unless we're debugging, then we want to see the output anyway
                #endif

            }
            catch (Exception ex)
            {
                // If exception thrown we'll keep the console window open so user can read the error message
                Console.WriteLine("Injection failed! Error: {0}", ex);
                Console.Read();
            }


        }


        static ProcessModule? InjectDLL(Process targetProcess, string dllPath, string dllName)
        {
            // All right, time to inject
            // First get the address of our own Kernel32's loadLibraryA (it will be the same in the target process because Kernel32 is loaded in the same virtual memory in all processes)
            IntPtr loadLibraryAddr = PInvoke.GetProcAddress(PInvoke.GetModuleHandle("kernel32.dll"), "LoadLibraryA");
            if (loadLibraryAddr == IntPtr.Zero) throw new InvalidOperationException("Could not get address of LoadLibraryA, error code: " + PInvoke.GetLastError());


            // Allocate some memory on the target process, enough to store the filepath of the DLL
            byte[] dllPathStringBytes = Encoding.Default.GetBytes(dllPath);
            IntPtr allocatedMemoryAddress = PInvoke.VirtualAllocEx(targetProcess.Handle, IntPtr.Zero, dllPathStringBytes.Length, PInvoke.ALLOC_FLAGS.MEM_COMMIT | PInvoke.ALLOC_FLAGS.MEM_RESERVE, PInvoke.ALLOC_FLAGS.PAGE_READWRITE);

            Console.WriteLine("Allocated memory in {0} at {1}", targetProcess.ProcessName, allocatedMemoryAddress.ToString("X"));
            // We'll want to free allocatedMemoryAddress if anything goes wrong, so wrap in try-finally
            try
            {
                // Use WriteProcessMemory to write the dllPath string to that allocated path
                // Note we need to modify then restore the page protection as we do this
                PInvoke.VirtualProtectEx(targetProcess.Handle, allocatedMemoryAddress, dllPathStringBytes.Length, PInvoke.PAGE_READWRITE, out uint lpflOldProtect);
                PInvoke.WriteProcessMemory(targetProcess.Handle, allocatedMemoryAddress, dllPathStringBytes, dllPathStringBytes.Length, out int bytesWritten);
                PInvoke.VirtualProtectEx(targetProcess.Handle, allocatedMemoryAddress, dllPathStringBytes.Length, lpflOldProtect, out _);

                if (bytesWritten != dllPathStringBytes.Length) throw new InvalidOperationException("Couldn't write DLLpath to process allocated memory");

                // Woo, now let's create a thread inside the target process that will call LoadLibraryA, with allocMemAddress (our DLL path string) as parameter
                IntPtr threadHandle = PInvoke.CreateRemoteThread(targetProcess.Handle, IntPtr.Zero, 0, loadLibraryAddr, allocatedMemoryAddress, 0, IntPtr.Zero);
                if (threadHandle == IntPtr.Zero) throw new InvalidOperationException("Couldn't create remote thread in target process to call LoadLibraryA");

                // We'll want to free our threadHandle if things go wrong so wrap in try-finally
                try
                {
                    // Wait for the thread to finish executing (or some timeout)
                    uint waitResult = PInvoke.WaitForSingleObject(threadHandle, 3000);

                    // Check if the thread completed succesfully
                    if (waitResult == 0x00000080) // WAIT_ABANDONED
                    {
                        throw new InvalidOperationException("Remote thread failed unexpectedly");
                    }
                    else if (waitResult == 0x00000102) // WAIT_TIMEOUT
                    {
                        throw new InvalidOperationException("Remote thread timed out");
                    }

                    // Looks like the thread completed, let's get it's exit code
                    // A successful LoadLibraryA call will return a 32bit truncated handle to the loaded DLL
                    PInvoke.GetExitCodeThread(threadHandle, out uint exitCode);

                    // A return of 0 means a failure
                    if (exitCode == 0)
                    {
                        throw new InvalidOperationException("LoadLibrary call failed, error code: " + PInvoke.GetLastError());
                    }

                    // Okay, if we got here then the LoadLibrary call was a success!
                    // Just need to get a handle to the loaded Library to return it
                    // While LoadLibraryA's exit code contains a handle to the loaded Library, unfortunately it only returns 32 bits. On 64 bit this means the handle is cut in half, so is pretty much useless.
                    Console.WriteLine("Successfully injected {0}.dll into {1}.exe", dllName, targetProcess.ProcessName);
                    return FindTargetModule(targetProcess, dllName);

                }
                finally
                {
                    // Free our created remote thread
                    PInvoke.CloseHandle(threadHandle);
                }

            }
            finally
            {
                // Free our allocated memory for the dll path string
                PInvoke.VirtualFreeEx(targetProcess.Handle, allocatedMemoryAddress, dllPathStringBytes.Length, PInvoke.AllocationType.Release);
            }

        }

        static Process? FindTargetProcess(string[] targets)
        {
            // Get all processes currently running
            Process[] AllProcesses = Process.GetProcesses();

            // Return the process that matches one of our targetProcessNames, or null if it's missing
            return AllProcesses.FirstOrDefault(p => targets.Contains(p.ProcessName));
        }

        static ProcessModule? FindTargetModule(Process process, string targetModuleName)
        {
            // Need to update process info object
            Process updatedProcess = Process.GetProcessById(process.Id);
            foreach (ProcessModule mod in updatedProcess.Modules)
            {
                //Console.WriteLine("target: {0}, check: {1}", targetModuleName, mod.ModuleName);
                if (mod.ModuleName == targetModuleName + ".dll")
                {
                    Console.WriteLine("Found target module in {0}!", process.ProcessName);
                    return mod;
                }

            }
            return null;
        }





        static void CallExternalShutdown(IntPtr? ShutdownAddress, Process process, ProcessModule module)
        {
            Console.WriteLine("Calling Shutdown on old injected dll");
            if (!ShutdownAddress.HasValue) throw new ArgumentNullException(nameof(ShutdownAddress));

            Console.WriteLine("Calling Shutdown at address: " + ShutdownAddress.Value.ToString("X"));

            // Luckily Shutdown takes no parameters so we don't need to allocate any memory or anything
            PInvoke.CreateRemoteThread(process.Handle, IntPtr.Zero, 0, ShutdownAddress.Value, IntPtr.Zero, 0, IntPtr.Zero);
        }

        static void CallExternalStartup(InitializationParameter initParam, IntPtr? StartupAddress, Process targetProcess, ProcessModule module)
        {
            if (!StartupAddress.HasValue) throw new ArgumentNullException(nameof(StartupAddress));

            // Now need to alloc memory in the target process and put our param in there
            int allocatedSize = Marshal.SizeOf(initParam);
            Console.WriteLine("initParam size: {0}", allocatedSize);
            IntPtr allocatedMemoryAddress = PInvoke.VirtualAllocEx(targetProcess.Handle, IntPtr.Zero, allocatedSize, PInvoke.ALLOC_FLAGS.MEM_COMMIT | PInvoke.ALLOC_FLAGS.MEM_RESERVE, PInvoke.ALLOC_FLAGS.PAGE_READWRITE);

            //try-finally so we free the allocated memory 
            try
            {
                // Converting the initParam to a byte array is a bit fun
                var initParamBytes = StructToByteArray(initParam);

                // Use WriteProcessMemory to write the dllPath string to that allocated path
                // Note we need to modify then restore the page protection as we do this
                PInvoke.VirtualProtectEx(targetProcess.Handle, allocatedMemoryAddress, allocatedSize, PInvoke.PAGE_READWRITE, out uint lpflOldProtect);
                PInvoke.WriteProcessMemory(targetProcess.Handle, allocatedMemoryAddress, initParamBytes, allocatedSize, out int bytesWritten);
                PInvoke.VirtualProtectEx(targetProcess.Handle, allocatedMemoryAddress, allocatedSize, lpflOldProtect, out _);

                if (bytesWritten != allocatedSize) throw new InvalidOperationException("Couldn't write DLLpath to process allocated memory");

                Console.WriteLine("Calling Startup at address: {0}", StartupAddress.Value.ToString("X"));

                // Call Startup with a pointer to our initParam
                PInvoke.CreateRemoteThread(targetProcess.Handle, IntPtr.Zero, 0, StartupAddress.Value, allocatedMemoryAddress, 0, IntPtr.Zero);


            }
            finally
            {
                // Free our allocated memory for the initparam
                PInvoke.VirtualFreeEx(targetProcess.Handle, allocatedMemoryAddress, allocatedSize, PInvoke.AllocationType.Release);
            }


        }

        static byte[] StructToByteArray(object structObject)
        {
            byte[] byteArray = new byte[Marshal.SizeOf(structObject)];
            var ptr = Marshal.AllocHGlobal(byteArray.Length);
            // Copy object byte-to-byte to unmanaged memory.
            Marshal.StructureToPtr(structObject, ptr, false);
            // Copy data from unmanaged memory to managed buffer.
            Marshal.Copy(ptr, byteArray, 0, byteArray.Length);
            // Release unmanaged memory.
            Marshal.FreeHGlobal(ptr);
            return byteArray;
        }

        static object ByteArrayToStruct(byte[] bytearray, Type objectType)
        {
            var ptr = Marshal.AllocHGlobal(bytearray.Length);
            Marshal.Copy(bytearray, 0, ptr, bytearray.Length);
            var my_object = Marshal.PtrToStructure(ptr, objectType);
            Marshal.FreeHGlobal(ptr);
            return my_object;
        }

        static IntPtr? GetProcAddressEx(Process targetProcess, ProcessModule targetModule, string functionName)
        {
            Console.WriteLine("GetProcAddressEx called, searching for {0}", functionName);
            // Adapted from https://stackoverflow.com/a/59537162
            // yes this would be infinitely easier in cpp

            IMAGE_DOS_HEADER image_dos_header = new();
            byte[] buffer = new byte[Marshal.SizeOf(image_dos_header)];
            if (!PInvoke.ReadProcessMemory(targetProcess.Handle, targetModule.BaseAddress, buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 1");
            image_dos_header = (IMAGE_DOS_HEADER)ByteArrayToStruct(buffer, typeof(IMAGE_DOS_HEADER));
            if (!image_dos_header.isValid) throw new InvalidOperationException("GetProcAddressEx failed: 2");


            IMAGE_NT_HEADERS64 image_nt_headers64 = new();
            buffer = new byte[Marshal.SizeOf(image_nt_headers64)];
            if (!PInvoke.ReadProcessMemory(targetProcess.Handle, IntPtr.Add(targetModule.BaseAddress, image_dos_header.e_lfanew), buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 3");
            image_nt_headers64 = (IMAGE_NT_HEADERS64)ByteArrayToStruct(buffer, typeof(IMAGE_NT_HEADERS64));
            if (image_nt_headers64.Signature != IMAGE_NT_SIGNATURE) throw new InvalidOperationException("GetProcAddressEx failed: 4");

            uint img_exp_dir_rva = image_nt_headers64.OptionalHeader.ExportTable.VirtualAddress;
            IMAGE_EXPORT_DIRECTORY image_export_directory = new();
            buffer = new byte[Marshal.SizeOf(image_export_directory)];
            if (!PInvoke.ReadProcessMemory(targetProcess.Handle, IntPtr.Add(targetModule.BaseAddress, (int)img_exp_dir_rva), buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 5");
            image_export_directory = (IMAGE_EXPORT_DIRECTORY)ByteArrayToStruct(buffer, typeof(IMAGE_EXPORT_DIRECTORY));

            IntPtr EAT = IntPtr.Add(targetModule.BaseAddress, (int)image_export_directory.AddressOfFunctions);
            IntPtr ENT = IntPtr.Add(targetModule.BaseAddress, (int)image_export_directory.AddressOfNames);
            IntPtr EOT = IntPtr.Add(targetModule.BaseAddress, (int)image_export_directory.AddressOfNameOrdinals);



            for (int i = 0; i < image_export_directory.NumberOfNames; i++)
            {
                int ptrStride = 4;

                buffer = new byte[ptrStride];
                if (!PInvoke.ReadProcessMemory(targetProcess.Handle, IntPtr.Add(ENT, (i * ptrStride)), buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 6");
                uint tempRvaString = BitConverter.ToUInt32(buffer, 0);

                buffer = new byte[functionName.Length + 1];
                if (!PInvoke.ReadProcessMemory(targetProcess.Handle, IntPtr.Add(targetModule.BaseAddress, (int)tempRvaString), buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 7");


                    string readName = Encoding.UTF8.GetString(buffer, 0, buffer.Length);
                    Console.WriteLine("read name: {0}", readName);

                if (readName.Trim('\0') == functionName) // Gotta remove the null char from the read name. TODO: make this ignore case
                {
                    Console.WriteLine("Match!");

                    buffer = new byte[2];
                    if (!PInvoke.ReadProcessMemory(targetProcess.Handle, IntPtr.Add(EOT, i * 2), buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 8");

                    ushort ordinal = BitConverter.ToUInt16(buffer, 0);

                    buffer = new byte[ptrStride];
                    if (!PInvoke.ReadProcessMemory(targetProcess.Handle, IntPtr.Add(EAT, ordinal * ptrStride), buffer, buffer.Length, out _)) throw new InvalidOperationException("GetProcAddressEx failed: 8");
                    uint temp_rva_func = BitConverter.ToUInt32(buffer, 0);

                    Console.WriteLine("temp_rva_func: " + temp_rva_func);
                    IntPtr finalAddy = IntPtr.Add(targetModule.BaseAddress, (int)temp_rva_func);
                    Console.WriteLine("final address: " + finalAddy.ToString("X"));
                    return finalAddy;

                }

            }

            return null;

        }


    }
}
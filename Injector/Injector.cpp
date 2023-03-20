// https://github.com/saeedirha/DLL-Injector/blob/master/DLL_Injector/Source.cpp/


// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <Shlwapi.h>
#include <locale>
#include <codecvt>
#include <ctype.h>

#pragma comment( lib, "shlwapi.lib")

using namespace std;

#define print(format, ...) fprintf (stderr, format, __VA_ARGS__)


bool InjectDLL(const int& pid, const string& DLL_Path);
int getProcID(const string& p_name);

int getProcID(const string& targetProcName)
{
    // PROCESSENTRY32 is used to open and get information about a running process..
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    // We use a th32snapprocess to iterate through all running processes.
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    // Success check oon the snapshot tool.
    if (!hSnap) {
        throw "Snapshot tool failed to open";
    }

    // If a first process exist (there are running processes), iterate through
    // all running processes.
    std::wstring string_to_convert;
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    DWORD ProcID = NULL;
    if (Process32First(hSnap, &entry)) {
        do
        {
            string entryA = converter.to_bytes(entry.szExeFile);
            // If the current process entry is the target process, store its ID.
            if (!strcmp(entryA.c_str(), targetProcName.c_str()))
            {
                ProcID = entry.th32ProcessID;
            }
        } while (Process32Next(hSnap, &entry) && !ProcID);        // Move on to the next running process.
    }
    else {
        // If there was no first process, notify the user.
        throw "No running processes found";
    }

    return ProcID;
}

bool InjectDLL(const int& pid, const string& DLL_Path)
{
    long dll_size = DLL_Path.length() + 1;
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (hProc == NULL)
    {
        cerr << "[!]Fail to open target process!" << endl;
        return false;
    }
    cout << "[+]Opening Target Process..." << endl;

    LPVOID MyAlloc = VirtualAllocEx(hProc, NULL, dll_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (MyAlloc == NULL)
    {
        cerr << "[!]Fail to allocate memory in Target Process." << endl;
        return false;
    }

    cout << "[+]Allocating memory in Target Process." << endl;
    int IsWriteOK = WriteProcessMemory(hProc, MyAlloc, DLL_Path.c_str(), dll_size, 0);
    if (IsWriteOK == 0)
    {
        cerr << "[!]Fail to write in Target Process memory." << endl;
        return false;
    }
    cout << "[+]Creating Remote Thread in Target Process" << endl;

    DWORD dWord;
    LPTHREAD_START_ROUTINE addrLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibraryA("kernel32"), "LoadLibraryA");
    HANDLE ThreadReturn = CreateRemoteThread(hProc, NULL, 0, addrLoadLibrary, MyAlloc, 0, &dWord);
    if (ThreadReturn == NULL)
    {
        cerr << "[!]Fail to create Remote Thread" << endl;
        return false;
    }

    if ((hProc != NULL) && (MyAlloc != NULL) && (IsWriteOK != ERROR_INVALID_HANDLE) && (ThreadReturn != NULL))
    {
        cout << "[+]DLL Successfully Injected :)" << endl; 
        return true;
    }

    return false;
}


std::string ExePath() {
    CHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos); 
}

int main()
{
    std::string steamProcName = "MCC-Win64-Shipping.exe";
    std::string dllpath = ExePath() + "\\CERandomiser.dll";

    std::string userInput; // I don't actually care, just want to hold the window open if there's a failure
    //TODO alloc console

    if (PathFileExistsA(dllpath.c_str()) == FALSE)
    {
        print("DLL File does NOT exist!");
        print(dllpath.c_str());
        std::cin >> userInput;
        return EXIT_FAILURE;
    }

    int procId = 0;
    procId = getProcID(steamProcName);

    bool success = false;

    if (procId == NULL)
    {
        print("Process Not found (0x%lX)\n", GetLastError());
    }
    else
    {
        success = InjectDLL(procId, dllpath);
    }


    if (success)
        return EXIT_SUCCESS;
    else
        std::cin>>userInput;
        return EXIT_FAILURE;

}


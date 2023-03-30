// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "windows_utilities.h"
#include "global_kill.h"
#include "ModuleCache.h"
#include "ModuleHookManager.h"
#include "D3D11Hook.h"
#include "ImGuiManager.h"
#include "CEERGUI.h"



void init_logging()
{
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("log.txt", 10000000, 3);
    plog::init<1>(plog::info, &fileAppender);


    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);


    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init<2>(plog::verbose, &consoleAppender);

    plog::init(plog::verbose).addAppender(plog::get<1>()).addAppender(plog::get<2>());


}

void stop_logging()
{
    FreeConsole();
}



/* GENERAL TODO:: 
Create a PointerManager service that stores all the pointers per game version / looks it up from the intertubes
Add OptionState stuff
BOY does the external window not interact good with RTSS
*/ 




// Main Execution Loop
void RealMain(HMODULE dllHandle) {

    init_logging();


    PLOG_INFO << "Randomizer initializing";
    // instantiate the singletons
    try
    {
        ModuleCache::initialize(); // First so ModuleOffset pointers can resolve
        ModuleHookManager::initialize();

        // Set up rendering and GUI
        D3D11Hook::initialize();
        ImGuiManager::initialize(D3D11Hook::get());
        CEERGUI::initialize();



    }
    catch (expected_exception& ex)
    {
        PLOG_FATAL << "Failed initializing singletons: " << ex.what();
        global_kill::kill_me();
    }



    // We live in this loop 99% of the time
    while (!global_kill::is_kill_set()) {
       

        if (GetKeyState(0x23) & 0x8000) // 'End' key
        {
            PLOG_INFO << "Killing internal dll";
            global_kill::kill_me();
        }


        if (GetKeyState(0x24) & 0x8000) // 'End' key
        {
            PLOG_INFO << "Killing hooks";
            ModuleHookManager::destroy();   PLOG_DEBUG << "ModuleHookManager destroyed";

        }
       
    }

    // Unattach hooks and release any manually managed resources
    // Destroy singletons (order matters)
    PLOG_DEBUG << "Unattaching hooks and destroying Singletons";
    CEERGUI::destroy();             PLOG_DEBUG << "CEERGUI destroyed";
    ImGuiManager::release();        PLOG_DEBUG << "ImGuiManager destroyed";
    D3D11Hook::release();           PLOG_DEBUG << "D3D11Hook destroyed";



    ModuleHookManager::destroy();   PLOG_DEBUG << "ModuleHookManager destroyed";

    PLOG_DEBUG << "Shutting down logging";
    stop_logging();
}


// This thread is created by the dll when loaded into the process, see RealMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) {
    
    RealMain(hDLL);

    Sleep(200);
    FreeLibraryAndExitThread(hDLL, NULL);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    DWORD dwThreadID;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, &dwThreadID);
    }

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;

}


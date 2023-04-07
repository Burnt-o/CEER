// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "windows_utilities.h"
#include "global_kill.h"
#include "ModuleCache.h"

#include "D3D11Hook.h"
#include "ImGuiManager.h"
#include "OptionsGUI.h"
#include "OptionsState.h"
#include "MirrorMode.h"
#include "LevelLoadHook.h"
#include "EnemyRandomiser.h"
#include "PointerManager.h"

#include "curl\curl.h"




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
IAT pointer
IAT hook
Create a PointerManager service that stores all the pointers per game version / looks it up from the intertubes
Add OptionState stuff

*/ 



// Main Execution Loop
void RealMain(HMODULE dllHandle) 
{
    init_logging();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    PLOG_INFO << "Randomizer initializing";
    try
    {
        auto ptr = std::make_unique<PointerManager>();
        ModuleCache::initialize();
        auto mhm = std::make_unique<ModuleHookManager>();
        auto d3d = std::make_unique<D3D11Hook>();
        auto imm = std::make_unique<ImGuiManager>(d3d.get()->presentHookEvent);
        auto optGUI = std::make_unique<OptionsGUI>(imm.get()->ImGuiRenderCallback);

        auto lvl = std::make_unique<LevelLoadHook>();

        // TODO: we should make the public events private and only allow public access by ref
        auto nme = std::make_unique<EnemyRandomiser>(OptionsState::EnemyRandomiserEnabled.valueChangedEvent, lvl->levelLoadEvent);


        // We live in this loop 99% of the time
        while (!global_kill::is_kill_set()) {


            if (GetKeyState(0x23) & 0x8000) // 'End' key
            {
                PLOG_INFO << "Killing internal dll";
                global_kill::kill_me();
            }


            //if (!mirror && GetKeyState(0x24) & 0x8000) // 'Home' key
            //{
            //    MirrorMode::initialize();
            //    mirror = true;
            //}

        }


    }
    catch (expected_exception& ex)
    {
        PLOG_FATAL << "Failed initializing: " << ex.what();
        std::cout << "Enter any command to shutdown CEER";
        global_kill::kill_me();
        std::string dontcare;
        std::cin >> dontcare;
    }

    // Auto managed resources have fallen out of scope
    std::cout << "Shutting down";

    bool mirror = false;




    if (mirror)
    {
        MirrorMode::destroy();
    }

    curl_global_cleanup();

    PLOG_DEBUG << "Shutting down logging";
    stop_logging();
}


// This thread is created by the dll when loaded into the process, see RealMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) 
{
    
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


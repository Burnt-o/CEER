// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "WindowsUtilities.h"
#include "GlobalKill.h"
#include "ModuleCache.h"

#include "D3D11Hook.h"
#include "ImGuiManager.h"
#include "OptionsGUI.h"
#include "OptionsState.h"
#include "MirrorMode.h"
#include "LevelLoadHook.h"
#include "EnemyRandomiser.h"
#include "PointerManager.h"
#include "MapReader.h"

#include "curl\curl.h"
#include "InitParameter.h"



void init_logging()
{
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("log.txt", 10000000, 3);
    plog::init<1>(plog::info, &fileAppender);


    DWORD attachError = 0;
    if (AttachConsole(GetCurrentProcessId()) == 0)
    {
        attachError = GetLastError();
        AllocConsole();
    }

    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);


    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init<2>(plog::verbose, &consoleAppender);

    plog::init(plog::verbose).addAppender(plog::get<1>()).addAppender(plog::get<2>());

    if (attachError != 0) PLOG_INFO << "Creating new console: " << attachError;

}

void stop_logging()
{
    fclose(stdout);
    fclose(stdin);
    fclose(stderr);
    FreeConsole();
}



/* GENERAL TODO:: 

*/ 



// Main Execution Loop
void RealMain(HMODULE dllHandle) 
{


    //acquire_global_unhandled_exception_handler();


    init_logging();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    PLOG_INFO << "Randomizer initializing";


    // wait for init parameters from the injector
    auto startTime = GetTickCount64();
    constexpr ULONGLONG timeoutMilliseconds = 10 * 1000;
    while (g_ourInitParameters == nullptr)
    {
        // Escape in case injector fails to call the Initialize function
        if (GlobalKill::isKillSet() || GetTickCount64() - startTime > timeoutMilliseconds)
        {
            return;
        }
        Sleep(50);
    }

    PLOG_DEBUG << "initParameter.injectorPath: " << g_ourInitParameters->injectorPath;




    try
    {
        ModuleCache::initialize();
        auto mhm = std::make_unique<ModuleHookManager>();
        auto d3d = std::make_unique<D3D11Hook>();
        auto imm = std::make_unique<ImGuiManager>(d3d.get()->presentHookEvent);
        auto optGUI = std::make_unique<OptionsGUI>(imm.get()->ImGuiRenderCallback);

        auto exp = std::make_unique<RuntimeExceptionHandler>(imm.get()->ImGuiRenderCallback);

        auto ptr = std::make_unique<PointerManager>(); // must be after moduleCache but before anything that uses it in it's constructor

        auto lvl = std::make_unique<LevelLoadHook>();

        auto map = std::make_unique<MapReader>(lvl->levelLoadEvent);
        // TODO: we should make the public events private and only allow public access by ref
        auto nme = std::make_unique<EnemyRandomiser>(OptionsState::MasterToggle.valueChangedEvent, lvl->levelLoadEvent, map.get());


        //CEERRuntimeException ex("test");
        //RuntimeExceptionHandler::handle(ex);


        // We live in this loop 99% of the time
        while (!GlobalKill::isKillSet()) {


            if (GetKeyState(0x23) & 0x8000) // 'End' key
            {
                PLOG_INFO << "Killing internal dll";
                GlobalKill::killMe();
            }

            if (GetKeyState(0x21) & 0x8000) // 'Page Up' key
            {
                ImGuiManager::debugInput();
            }


        }


    }
    catch (InitException& ex)
    {
        PLOG_FATAL << "Failed initializing: " << ex.what();
        std::cout << "Enter any command to shutdown CEER";
        GlobalKill::killMe();
        std::string dontcare;
        std::cin >> dontcare;
    }

    // Auto managed resources have fallen out of scope
    std::cout << "Shutting down";




    curl_global_cleanup();
    //release_global_unhandled_exception_handler();
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


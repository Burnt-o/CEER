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
#include "Logging.h"
#include "MessagesGUI.h"
#include "OptionSerialisation.h"
#include "TextureRandomiser.h"
#include "SoundRandomiser.h"
#include "CEERStateLogger.h"


/* GENERAL TODO:: 


look into cursed halo again compatibility
look into rule combination logic (enemy rando)
finish playthru of game (check nipple grunt)


-- known issues (won't fix)
energy swords disappear if elite forced out of vehicle - the issue is tied to the "detonates when dropped" flag.
    happens to frg too
    would have to follow breakpoints on what checks the detonate flag and what is setting it off etc.. ceebs
    - might not bother fixing

the game sometimes freezes if you shutdown CEER while on the first part of the main menu after booting MCC.
    - won't fix
*/ 



// Main Execution Loop
void RealMain(HMODULE dllHandle) 
{


    acquire_global_unhandled_exception_handler();

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


    Logging::initLogging();
    Logging::SetConsoleLoggingLevel(plog::verbose);
    Logging::SetFileLoggingLevel(plog::verbose);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    PLOG_INFO << "Randomizer initializing";

    PLOG_DEBUG << "initParameter.injectorPath: " << g_ourInitParameters->injectorPath;



    try
    {
        // setup default rules
        OptionsState::currentRandomiserRules.emplace_back(new RandomiseXintoY());
        OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierBeforeRando());


        ModuleCache::initialize();
        auto mhm = std::make_unique<ModuleHookManager>();
        auto d3d = std::make_unique<D3D11Hook>();
        auto imm = std::make_unique<ImGuiManager>(d3d.get()->presentHookEvent);
        auto optGUI = std::make_unique<OptionsGUI>(imm.get()->ImGuiRenderCallback, d3d.get()->resizeBuffersHookEvent);
        auto mesGUI = std::make_unique<MessagesGUI>(imm.get()->ImGuiRenderCallback);

        auto exp = std::make_unique<RuntimeExceptionHandler>(imm.get()->ImGuiRenderCallback);

        auto ptr = std::make_unique<PointerManager>(); // must be after moduleCache but before anything that uses it in it's constructor

        auto lvl = std::make_unique<LevelLoadHook>();

        auto map = std::make_unique<MapReader>();
        // TODO: we should make the public events private and only allow public access by ref
        auto nme = std::make_unique<EnemyRandomiser>(lvl->levelLoadEvent, map.get());
        auto tex = std::make_unique<TextureRandomiser>(lvl->levelLoadEvent, map.get());
        auto snd = std::make_unique<SoundRandomiser>(lvl->levelLoadEvent, map.get());
        auto csl = std::make_unique<CEERStateLogger>(lvl->levelLoadEvent);

        OptionSerialisation::deserialiseFromFile();

        PLOG_INFO << "All services succesfully initialized! Entering main loop";
        Sleep(100);
        // Shutdown the console on successful init, at least in release mode.
        // If an initialization error occurs before this point, console will be left up so user can look at it.
        MessagesGUI::addMessage("CEER successfully initialised!");






#ifndef CEER_DEBUG
        Logging::closeConsole();
        Logging::SetConsoleLoggingLevel(plog::info);
        Logging::SetFileLoggingLevel(plog::info);
#else

#endif // !CEER_DEBUG


        // We live in this loop 99% of the time
        while (!GlobalKill::isKillSet()) {

#ifdef CEER_DEBUG
            if (GetKeyState(0x23) & 0x8000) // 'End' key
            {
                PLOG_INFO << "Killing internal dll";
                GlobalKill::killMe();
            }

            if (GetKeyState(0x21) & 0x8000) // 'Page Up' key
            {
                ImGuiManager::debugInput();
            }
#endif // !CEER_DEBUG
            Sleep(10);

        }

        // Closing down, save options to config file
        OptionSerialisation::serialiseToFile();

    }
    catch (InitException& ex)
    {
        PLOG_FATAL << "Failed initializing: " << ex.what();
        PLOG_FATAL << "Please send Burnt the log file located at: " << std::endl << Logging::GetLogFileDestination();
        std::cout << "Enter any command to shutdown CEER";
        GlobalKill::killMe();
        std::string dontcare;
        std::cin >> dontcare;
    }

    // Auto managed resources have fallen out of scope
    std::cout << "CEER singletons succesfully shut down";

    curl_global_cleanup();
    release_global_unhandled_exception_handler();

#ifdef CEER_DEBUG
    while (!GetKeyState(0x23) & 0x8000) // 'End' key
    {
        Sleep(100);
    }
#endif // !CEER_DEBUG
    PLOG_DEBUG << "Closing console";
    Logging::closeConsole();


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


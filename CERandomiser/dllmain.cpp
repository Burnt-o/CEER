// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "windows_utilities.h"
#include "global_kill.h"
#include "dll_cache.h"
#include "hook_manager.h"
#include "config_window.h"



SafetyHookInline hook0, hook1, hook2, hook3;

SafetyHookMid hookCE;

__declspec(noinline) void say_hi(const std::string& name) {
    std::cout << "hello " << name << std::endl;
}

void hook0_fn(const std::string& name) {
    hook0.call<void, const std::string&>(name + " and bob");
}

void hook1_fn(const std::string& name) {
    hook1.call<void, const std::string&>(name + " and alice");
}

void hook2_fn(const std::string& name) {
    hook2.call<void, const std::string&>(name + " and eve");
}

void hook3_fn(const std::string& name) {
    hook3.call<void, const std::string&>(name + " and carol");
}




void init()
{
    std::cout << "init " << std::endl;
    {
    auto builder = SafetyHookFactory::acquire();
    hook0 = builder.create_inline((void*)say_hi, (void*)hook0_fn);
    hook1 = builder.create_inline((void*)say_hi, (void*)hook1_fn);
    hook2 = builder.create_inline((void*)say_hi, (void*)hook2_fn);
    hook3 = builder.create_inline((void*)say_hi, (void*)hook3_fn);

    uintptr_t CEcodetarget = 0x7FF8A08D40B5;

    hookCE = builder.create_mid((void*)CEcodetarget, [](safetyhook::Context& ctx)
        { 
            std::cout << "Hello from ce enemy spawn mid-function hook." << std::endl;
            ctx.rcx = 0;
        });
    }
    say_hi("world");
    std::cout << "done " << std::endl;
}


void init_logging()
{


    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("log.txt", 10000000, 3);
    plog::init<1>(plog::info, &fileAppender);

//#ifdef _DEBUG 
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
//endif

    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init<2>(plog::verbose, &consoleAppender);

    plog::init(plog::verbose).addAppender(plog::get<1>()).addAppender(plog::get<2>());




}

void stop_logging()
{
    
//#ifdef _DEBUG 
    FreeConsole();
//#endif

}

bool verbose = false;

// Main Execution Loop
void RealMain() {
    //acquire_global_unhandled_exception_handler();
    init_logging();
    PLOG_INFO << "Randomizer initializing";

    dll_cache::initialize();
    config_window::initialize();


    //auto interop = std::make_unique<gui_interop>();
    auto hks = std::make_unique <hook_manager> ();
    hks->attach_all();

     //Most of the cool stuff happens in other threads.
     //This loop is just to keep stuff alive.
   while (!global_kill::is_kill_set()) {
       if (GetKeyState(0x23) & 0x8000) // 'End' key
       {
           PLOG_INFO << "Killing internal dll";
           global_kill::kill_me();
       }

       if (GetKeyState(0x24) & 0x8000) // 'Home' key
       {
           PLOG_INFO << "enabling halo 1 enemy randomizer";
           hks->EnableEnemyRandomizer();
       }


        Sleep(100);
    }

     // shutdown

    hks->detach_all();

    //stop_logging();
    //release_global_unhandled_exception_handler();
}


// This thread is created by the dll when loaded into the process, see RealMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) {
    RealMain();

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


    FILE* pCout = nullptr;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        AllocConsole();
        freopen_s(&pCout, "conout$", "w", stdout);
        init();

        break;
    case DLL_THREAD_ATTACH:

        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


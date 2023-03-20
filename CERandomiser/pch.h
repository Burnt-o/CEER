// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H



// Windows 
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <Psapi.h>
#include <winternl.h>
#include <Dbghelp.h>






// Standard library
#include <iostream>
//#include <memory>
#include <vector>
#include <cstdint>
#include <optional>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <thread>
//#include <format>
#include <string>
#include <unordered_map>









// Libraries
// hooking
#include <SafetyHook.hpp>

// logging
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Initializers/ConsoleInitializer.h>

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
//#include "imgui/imgui_stdlib.h"





// Utilities
#include "expected_exception.h"



#endif //PCH_H

#pragma once
#include "InitParameter.h"



extern "C" __declspec(dllexport) void Shutdown();

extern "C" __declspec(dllexport) void Startup(InitParameter);

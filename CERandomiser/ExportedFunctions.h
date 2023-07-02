#pragma once
#include "InitParameter.h"



extern "C" __declspec(dllexport) BOOL Shutdown();

extern "C" __declspec(dllexport) BOOL Startup(InitParameter);

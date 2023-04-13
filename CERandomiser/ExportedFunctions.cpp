#include "pch.h"
#include "ExportedFunctions.h"
#include "GlobalKill.h"

extern "C" __declspec(dllexport) void Shutdown()
{
	GlobalKill::killMe();
}

extern "C" __declspec(dllexport) void Startup(InitParameter initParam)
{
	// Make a copy of the initParam
	// The default copy constructor just clones all members which is good enough for us
	g_ourInitParameters = new InitParameter(initParam);
}

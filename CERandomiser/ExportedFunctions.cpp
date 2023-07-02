#include "pch.h"
#include "ExportedFunctions.h"
#include "GlobalKill.h"

extern "C" __declspec(dllexport) BOOL Shutdown()
{
	PLOG_INFO << "Shutdown called from external";
	GlobalKill::killMe();
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL Startup(InitParameter initParam)
{
	// Make a copy of the initParam
	// The default copy constructor just clones all members which is good enough for us
	g_ourInitParameters = new InitParameter(initParam);
	return TRUE;
}

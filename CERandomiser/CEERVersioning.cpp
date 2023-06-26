#include "pch.h"
#include "CEERVersioning.h"
#include "InitParameter.h"
#pragma comment(lib, "version.lib")

bool CEERVersioning::currentVersionCached = false;
VersionInfo CEERVersioning::currentVersion;
bool CEERVersioning::latestVersionCached = false;
VersionInfo CEERVersioning::latestVersion;

// Looks up own dll path to access productversioninfo of file
VersionInfo CEERVersioning::GetCurrentVersion()
{
	if (currentVersionCached) return currentVersion;

	std::string dllPath = g_ourInitParameters->injectorPath;
	dllPath += "CERandomiser.dll";

	currentVersionCached = true;

	return getFileVersion(dllPath.c_str());
}

// reads from pointerData
VersionInfo CEERVersioning::GetLatestVersion()
{
	if (latestVersionCached) return latestVersion;

	throw InitException("Could not read latest CEER version from pointer data");
}

void CEERVersioning::SetLatestVersion(VersionInfo verInfo)
{
	if (latestVersionCached)
	{
		PLOG_ERROR << "Attempting to set latest version but it was already set";
		return;
	}

	latestVersion = verInfo;
	latestVersionCached = true;
}
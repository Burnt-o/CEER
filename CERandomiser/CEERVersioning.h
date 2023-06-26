#pragma once




class CEERVersioning
{
private:
	static bool currentVersionCached;
	static VersionInfo currentVersion;
	static bool latestVersionCached;
	static VersionInfo latestVersion;
public:
	static VersionInfo GetCurrentVersion();
	static VersionInfo GetLatestVersion(); 
	static void SetLatestVersion(VersionInfo verInfo); // called by pointer manager

};


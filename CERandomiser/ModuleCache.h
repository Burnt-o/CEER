#pragma once


/// <summary>
/// The ModuleCache singleton contains the current status of ALL loaded DLLs
/// With hooks to LoadLibrary/FreeLibrary (applied in HookManager) this should always be up to date.
/// Adapted from scales' MCCTAS https://github.com/Scaless/HaloTAS/blob/master/HaloTAS/libhalotas/dll_cache.h
/// </summary>
class ModuleCache
{
private:
	// DLL Name, Base Address
	std::unordered_map<std::wstring, MODULEINFO> mCache;

	static ModuleCache& get() {
		static ModuleCache instance;
		return instance;
	}

	// constructor hidden
	ModuleCache() = default;
	~ModuleCache()
	{
		PLOG_VERBOSE << "ModuleCache destroyed";
	}



public:

	// Wipes the cache and gets the current status of ALL modules
	// This is usually very expensive (10s of milliseconds) so don't call this often
	static void initialize();

	// Returns true if successfully added to cache
	static bool addModuleToCache(const std::wstring& moduleName, HMODULE moduleHandle);
	// Returns true if succssfully removed from cache
	static bool removeModuleFromCache(const std::wstring& moduleName);

	static std::optional<HMODULE> getModuleHandle(const std::wstring& moduleName);
	static std::optional<MODULEINFO> getModuleInfo(const std::wstring& moduleName);
	static bool isModuleInCache(const std::wstring& moduleName);


	// no copying
	ModuleCache(ModuleCache& other) = delete;
	ModuleCache& operator=(const ModuleCache&) = delete;

	static void destroy()
	{
		get().~ModuleCache();
	}
};


// I swear to god "cache" is not a real word
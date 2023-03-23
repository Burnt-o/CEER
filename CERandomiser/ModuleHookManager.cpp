#include "pch.h"
#include "ModuleHookManager.h"
#include "windows_utilities.h"

#include "ModuleCache.h"

#include "enemy_randomizer.h"




ModuleHookManager::ModuleHookManager()
{
	// We'll make global hooks here that track dll loading/unloading
	PLOG_INFO << "Hooking module load/unload";
	auto builder = SafetyHookFactory::acquire();

	mHook_LoadLibraryA = builder.create_inline(&LoadLibraryA, &newLoadLibraryA);
	mHook_LoadLibraryW = builder.create_inline(&LoadLibraryW, &newLoadLibraryW);
	mHook_LoadLibraryExA = builder.create_inline(&LoadLibraryExA, &newLoadLibraryExA);
	mHook_LoadLibraryExW = builder.create_inline(&LoadLibraryExW, &newLoadLibraryExW);
	mHook_FreeLibrary = builder.create_inline(&FreeLibrary, &newFreeLibrary);

	mModuleHooksMap.reserve(6); // probably only the 6 game dll's that we might care about ever

}



// This only called on global_kill
void ModuleHookManager::detachAllHooks() {
	// Get all hook elements and detach them
	for (auto& [moduleName, hookVector] : mModuleHooksMap)
	{
		for (auto& hook : hookVector)
		{
			hook->detach();
		}
	}

}


// Called by newFreeLibrary, unhooks any hooks tied to the unloading module
void ModuleHookManager::preModuleUnload_UpdateHooks(std::wstring_view libFilename) 
{

	// Get a ref to the module-hooks map
	const std::unordered_map<std::wstring, std::vector<std::shared_ptr<ModuleHookBase>>>& moduleHooksMap = ModuleHookManager::get().mModuleHooksMap;

	// Is the currently unloading library in our map?
	auto it = moduleHooksMap.find(libFilename.data());

	if (it == moduleHooksMap.end()) return; // Module isn't in our map, we're done here

	PLOG_DEBUG << "pre_lib_unload_hooks_patches detaching hooks for: " << libFilename;
	// Iterate thru the vector of hooks associated with this module and detach them all
	for (auto& hook : it->second)
	{
			hook->detach();
	}
}

// Called by newLoadLibraries, attaches any hooks (that want to be enabled) associated with the loading module
void ModuleHookManager::postModuleLoad_UpdateHooks(std::wstring_view libPath) 
{
	// Need to get the name of the module from the libPath arguement
	std::filesystem::path path = libPath;
	auto libFilename = path.filename().generic_wstring();

	// Get a ref to the module-hooks map
	const std::unordered_map<std::wstring, std::vector<std::shared_ptr<ModuleHookBase>>>& moduleHooksMap = ModuleHookManager::get().mModuleHooksMap;

	// Is the currently loading module in our map?
	auto it = moduleHooksMap.find(libFilename.data());

	if (it == moduleHooksMap.end()) return; // Module isn't in our map, we're done here

	PLOG_DEBUG << "post_lib_load_hooks_patches attaching hooks for: " << libFilename;
	// Iterate thru the vector of hooks associated with this module and attach all the wants that have their wantToBeAttached flag set
	for (auto& hook : it->second)
	{
		if (hook->getWantsToBeAttached())
		{
			hook->attach();
		}
		else // It wants to be disabled
		{
			hook->detach();
		}
	}
}


// the hook-redirected functions
HMODULE ModuleHookManager::newLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = ModuleHookManager::get().mHook_LoadLibraryA.call< HMODULE, LPCSTR > (lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << "LoadLibraryA: " << wLibFileName;
	ModuleCache::addModuleToCache(wLibFileName, result);
	postModuleLoad_UpdateHooks(wLibFileName);

	return result;
}

HMODULE ModuleHookManager::newLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = ModuleHookManager::get().mHook_LoadLibraryW.call<HMODULE, LPCWSTR>(lpLibFileName);

	PLOG_DEBUG << L"LoadLibraryW: " << lpLibFileName;
	ModuleCache::addModuleToCache(lpLibFileName, result);
	postModuleLoad_UpdateHooks(lpLibFileName);

	return result;
}

HMODULE ModuleHookManager::newLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = ModuleHookManager::get().mHook_LoadLibraryExA.call<HMODULE, LPCSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << L"LoadLibraryExA: " << wLibFileName;
	ModuleCache::addModuleToCache(wLibFileName, result);
	postModuleLoad_UpdateHooks(wLibFileName);

	return result;
}

HMODULE ModuleHookManager::newLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = ModuleHookManager::get().mHook_LoadLibraryExW.call<HMODULE, LPCWSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	PLOG_DEBUG << L"LoadLibraryExW: " << lpLibFileName;
	ModuleCache::addModuleToCache(lpLibFileName, result);
	postModuleLoad_UpdateHooks(lpLibFileName);

	return result;
}

BOOL ModuleHookManager::newFreeLibrary(HMODULE hLibModule) {
	PLOG_DEBUG << L"FreeLibrary";
	

	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));
	PLOG_DEBUG << L"FreeLibrary: " << moduleFilePath;

	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

	preModuleUnload_UpdateHooks(filename);
	ModuleCache::removeModuleFromCache(filename);
	return ModuleHookManager::get().mHook_FreeLibrary.call<BOOL, HMODULE>(hLibModule);
}


#include "pch.h"
#include "ModuleHookManager.h"
#include "WindowsUtilities.h"

#include "ModuleCache.h"


ModuleHookManager* ModuleHookManager::instance = nullptr;


ModuleHookManager::ModuleHookManager()
{
	if (instance != nullptr)
	{ 
		throw InitException("Cannot have more than one ModuleHookManager");
	}
	instance = this;

	// We'll make global hooks here that track dll loading/unloading
	PLOG_INFO << "Hooking module load/unload";

	mHook_LoadLibraryA = safetyhook::create_inline(&LoadLibraryA, &newLoadLibraryA);
	mHook_LoadLibraryW = safetyhook::create_inline(&LoadLibraryW, &newLoadLibraryW);
	mHook_LoadLibraryExA = safetyhook::create_inline(&LoadLibraryExA, &newLoadLibraryExA);
	mHook_LoadLibraryExW = safetyhook::create_inline(&LoadLibraryExW, &newLoadLibraryExW);
	mHook_FreeLibrary = safetyhook::create_inline(&FreeLibrary, &newFreeLibrary);

	mModuleHooksMap.reserve(6); // probably only the 6 game dll's that we might care about ever
	PLOG_VERBOSE << "Hooking module load/unload done";
}

ModuleHookManager::~ModuleHookManager()
{
	PLOG_VERBOSE << "ModuleHookManager destructor called";

	// We must detach the module-relative hooks before unhooking the library load/unload functions
	// otherwise we could have a stale reference issue
	detachAllHooks();

	instance->mHook_LoadLibraryA.reset();
	instance->mHook_LoadLibraryW.reset();
	instance->mHook_LoadLibraryExA.reset();
	instance->mHook_LoadLibraryExW.reset();
	instance->mHook_FreeLibrary.reset();

	instance = nullptr;
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
	const auto& moduleHooksMap = instance->mModuleHooksMap;

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
	const auto& moduleHooksMap = instance->mModuleHooksMap;

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



// For debugging/logging the below functions, logs only if the affected module is one of the ones we care about
#ifdef CEER_DEBUG

std::set<std::wstring> gameDLLNames{ L"halo1.dll", L"halo2.dll", L"halo3.dll", L"halo3odst.dll", L"haloreach.dll", L"halo4.dll" };

#define printGameDLL(callingfunc, dllname) if (gameDLLNames.contains(dllname)) \
						{	\
							PLOG_INFO << callingfunc << dllname;	\
						}	\

#endif // CEER_DEBUG



// the hook-redirected functions
HMODULE ModuleHookManager::newLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = instance->mHook_LoadLibraryA.call< HMODULE, LPCSTR > (lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);

#ifdef CEER_DEBUG
	printGameDLL(L"LoadLibraryA: ", wLibFileName)
#endif

	ModuleCache::addModuleToCache(wLibFileName, result);
	postModuleLoad_UpdateHooks(wLibFileName);

	return result;
}

HMODULE ModuleHookManager::newLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = instance->mHook_LoadLibraryW.call<HMODULE, LPCWSTR>(lpLibFileName);

#ifdef CEER_DEBUG
	printGameDLL(L"LoadLibraryW: ", lpLibFileName)
#endif

	ModuleCache::addModuleToCache(lpLibFileName, result);
	postModuleLoad_UpdateHooks(lpLibFileName);

	return result;
}

HMODULE ModuleHookManager::newLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = instance->mHook_LoadLibraryExA.call<HMODULE, LPCSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);

#ifdef CEER_DEBUG
	printGameDLL(L"LoadLibraryExA: ", wLibFileName)
#endif
;
	ModuleCache::addModuleToCache(wLibFileName, result);
	postModuleLoad_UpdateHooks(wLibFileName);

	return result;
}

HMODULE ModuleHookManager::newLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = instance->mHook_LoadLibraryExW.call<HMODULE, LPCWSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

#ifdef CEER_DEBUG
	printGameDLL(L"LoadLibraryExW: ", lpLibFileName)
#endif
	ModuleCache::addModuleToCache(lpLibFileName, result);
	postModuleLoad_UpdateHooks(lpLibFileName);

	return result;
}

BOOL ModuleHookManager::newFreeLibrary(HMODULE hLibModule) {

	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));



	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

#ifdef CEER_DEBUG
	printGameDLL(L"FreeLibrary: ", filename)
#endif

	preModuleUnload_UpdateHooks(filename);
	ModuleCache::removeModuleFromCache(filename);
	return instance->mHook_FreeLibrary.call<BOOL, HMODULE>(hLibModule);
}


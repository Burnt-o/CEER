#include "pch.h"
#include "HookManager.h"
#include "windows_utilities.h"

#include "ModuleCache.h"

#include "enemy_randomizer.h"




HookManager::HookManager()
{
	PLOG_INFO << "hook_manager initializing";
	//MultilevelPointer* pCEEnemyHook1 = MultilevelPointer::make(std::wstring(L"halo1.dll"), { 0xC540B5 });

	// We'll make global hooks here that track dll loading/unloading
	PLOG_INFO << "Creating hooks";
	auto mGlobalHook_LoadLibraryA = std::make_shared<InlineHook>(L"hkLoadLibraryA", MultilevelPointer::make(&LoadLibraryA), newLoadLibraryA, true);
	auto mGlobalHook_LoadLibraryW = std::make_shared<InlineHook>(L"hkLoadLibraryW", MultilevelPointer::make(&LoadLibraryW), newLoadLibraryW, true);
	auto mGlobalHook_LoadLibraryExA = std::make_shared<InlineHook>(L"hkLoadLibraryExA", MultilevelPointer::make(&LoadLibraryExA), newLoadLibraryExA, true);
	auto mGlobalHook_LoadLibraryExW = std::make_shared<InlineHook>(L"hkLoadLibraryExW", MultilevelPointer::make(&LoadLibraryExW), newLoadLibraryExW, true);
	auto mGlobalHook_FreeLibrary = std::make_shared<InlineHook>(L"hkFreeLibrary", MultilevelPointer::make(&FreeLibrary), newFreeLibrary, true);

	mAllHooks.reserve(20);

	// Add to our list. 
	mAllHooks.emplace_back(mGlobalHook_LoadLibraryA);
	mAllHooks.emplace_back(mGlobalHook_LoadLibraryW);
	mAllHooks.emplace_back(mGlobalHook_LoadLibraryExA);
	mAllHooks.emplace_back(mGlobalHook_LoadLibraryExW);
	mAllHooks.emplace_back(mGlobalHook_FreeLibrary);

	// Attach them
	mGlobalHook_LoadLibraryA.get()->attach();
	mGlobalHook_LoadLibraryW.get()->attach();
	mGlobalHook_LoadLibraryExA.get()->attach();
	mGlobalHook_LoadLibraryExW.get()->attach();
	mGlobalHook_FreeLibrary.get()->attach();
}



//this only called on global_kill
void HookManager::detachAllHooks() {
	for (std::shared_ptr<Hook> hook : mAllHooks) {
		hook->detach();
	}

}


// called by newFreeLibrary, unhooks any hooks tied to the unloading module
void HookManager::preModuleUnload_UpdateHooks(std::wstring_view libFilename) {
	PLOG_DEBUG << "pre_lib_unload_hooks_patches iterating, unloaded module: " << libFilename;
	for (std::shared_ptr<Hook> hook : HookManager::get().mAllHooks) {
		if (hook->getAssociatedModule() == libFilename) {
			hook->detach();
		}
	}
}

// called by newLoadLibraries, attaches any hooks (that want to be enabled) associated with the loading module
void HookManager::postModuleLoad_UpdateHooks(std::wstring_view libPath) {
	PLOG_DEBUG << "post_lib_load_hooks_patches iterating";
	std::filesystem::path path = libPath;
	auto filename = path.filename().generic_wstring();

	for (std::shared_ptr<Hook> hook : HookManager::get().mAllHooks) {
		if (hook->get_WantsToBeEnabled())
		{
			if (hook->getAssociatedModule() == filename)
			{
				hook->attach();
			}
		}
		else
		{
			hook->detach();
		}

	}
}


// the hook-redirected functions
HMODULE HookManager::newLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = HookManager::get().mHook_LoadLibraryA.get()->getInlineHook().call<HMODULE, LPCSTR>(lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << "LoadLibraryA: " << wLibFileName;
	ModuleCache::addModuleToCache(wLibFileName, result);
	postModuleLoad_UpdateHooks(wLibFileName);

	return result;
}

HMODULE HookManager::newLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = HookManager::get().mHook_LoadLibraryW.get()->getInlineHook().call<HMODULE, LPCWSTR>(lpLibFileName);

	PLOG_DEBUG << L"LoadLibraryW: " << lpLibFileName;
	ModuleCache::addModuleToCache(lpLibFileName, result);
	postModuleLoad_UpdateHooks(lpLibFileName);

	return result;
}

HMODULE HookManager::newLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = HookManager::get().mHook_LoadLibraryExA->getInlineHook().call<HMODULE, LPCSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << L"LoadLibraryExA: " << wLibFileName;
	ModuleCache::addModuleToCache(wLibFileName, result);
	postModuleLoad_UpdateHooks(wLibFileName);

	return result;
}

HMODULE HookManager::newLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = HookManager::get().mHook_LoadLibraryExW->getInlineHook().call<HMODULE, LPCWSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	PLOG_DEBUG << L"LoadLibraryExW: " << lpLibFileName;
	ModuleCache::addModuleToCache(lpLibFileName, result);
	postModuleLoad_UpdateHooks(lpLibFileName);

	return result;
}

BOOL HookManager::newFreeLibrary(HMODULE hLibModule) {
	PLOG_DEBUG << L"FreeLibrary";
	


	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));
	PLOG_DEBUG << L"FreeLibrary: " << moduleFilePath;

	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

	preModuleUnload_UpdateHooks(filename);
	ModuleCache::removeModuleFromCache(filename);
	return HookManager::get().mHook_FreeLibrary->getInlineHook().call<BOOL, HMODULE>(hLibModule);
}


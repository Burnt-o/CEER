#include "pch.h"
#include "hook_manager.h"
#include "windows_utilities.h"
//#include "patch.h"
#include "dll_cache.h"
#include "hook.h"
#include "enemy_randomizer.h"












hook_manager::hook_manager()
{
	PLOG_INFO << "hook_manager initializing";
	//MultilevelPointer* pCEEnemyHook1 = MultilevelPointer::make(std::wstring(L"halo1.dll"), { 0xC540B5 });

	// We'll make global hooks here that track dll loading/unloading
	PLOG_INFO << "Creating hooks";
	std::shared_ptr<InlineHook> mGlobalHook_LoadLibraryA = std::make_shared<InlineHook>(L"hkLoadLibraryA", MultilevelPointer::make(&LoadLibraryA), newLoadLibraryA, true);
	std::shared_ptr<InlineHook> mGlobalHook_LoadLibraryW = std::make_shared<InlineHook>(L"hkLoadLibraryW", MultilevelPointer::make(&LoadLibraryW), newLoadLibraryW, true);
	std::shared_ptr<InlineHook> mGlobalHook_LoadLibraryExA = std::make_shared<InlineHook>(L"hkLoadLibraryExA", MultilevelPointer::make(&LoadLibraryExA), newLoadLibraryExA, true);
	std::shared_ptr<InlineHook> mGlobalHook_LoadLibraryExW = std::make_shared<InlineHook>(L"hkLoadLibraryExW", MultilevelPointer::make(&LoadLibraryExW), newLoadLibraryExW, true);
	std::shared_ptr<InlineHook> mGlobalHook_FreeLibrary = std::make_shared<InlineHook>(L"hkFreeLibrary", MultilevelPointer::make(&FreeLibrary), newFreeLibrary, true);

	mAllHooks.reserve(20);

	// Add to our list. 
	mAllHooks.push_back(mGlobalHook_LoadLibraryA);
	mAllHooks.push_back(mGlobalHook_LoadLibraryW);
	mAllHooks.push_back(mGlobalHook_LoadLibraryExA);
	mAllHooks.push_back(mGlobalHook_LoadLibraryExW);
	mAllHooks.push_back(mGlobalHook_FreeLibrary);

	// Attach them
	mGlobalHook_LoadLibraryA.get()->attach();
	mGlobalHook_LoadLibraryA.get()->attach();
	mGlobalHook_LoadLibraryExA.get()->attach();
	mGlobalHook_LoadLibraryExW.get()->attach();
	mGlobalHook_FreeLibrary.get()->attach();
}








//this only called on global_kill
void hook_manager::detach_all() {
	for (std::shared_ptr<Hook> hook : mAllHooks) {
		hook->detach();
	}

}



// called by newLoadLibraries, attaches any hooks (that want to be enabled) associated with the loading module
void hook_manager::post_lib_load_hooks_patches(std::wstring_view libPath) {
	PLOG_DEBUG << "post_lib_load_hooks_patches iterating";
	std::filesystem::path path = libPath;
	auto filename = path.filename().generic_wstring();

	for (std::shared_ptr<Hook> hook : mAllHooks) {
		if (hook->get_WantsToBeEnabled())
		{
			if (hook->associated_module() == filename)
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

// called by newFreeLibrary, unhooks any hooks tied to the unloading module
void hook_manager::pre_lib_unload_hooks_patches(std::wstring_view libFilename) {
	PLOG_DEBUG << "pre_lib_unload_hooks_patches iterating, unloaded module: " << libFilename;
	for (std::shared_ptr<Hook> hook : mAllHooks) {
		if (hook->associated_module() == libFilename) {
			hook->detach();
		}
	}
}

HMODULE hook_manager::newLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = mHook_LoadLibraryA.get()->getInlineHook().call<HMODULE, LPCSTR>(lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << "LoadLibraryA: " << wLibFileName;
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hook_manager::newLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = mHook_LoadLibraryW.get()->getInlineHook().call<HMODULE, LPCWSTR>(lpLibFileName);

	PLOG_DEBUG << L"LoadLibraryW: " << lpLibFileName;
	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

HMODULE hook_manager::newLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = mHook_LoadLibraryExA->getInlineHook().call<HMODULE, LPCSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << L"LoadLibraryExA: " << wLibFileName;
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hook_manager::newLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = mHook_LoadLibraryExW->getInlineHook().call<HMODULE, LPCWSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	PLOG_DEBUG << L"LoadLibraryExW: " << lpLibFileName;
	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

BOOL hook_manager::newFreeLibrary(HMODULE hLibModule) {
	PLOG_DEBUG << L"FreeLibrary";
	


	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));
	PLOG_DEBUG << L"FreeLibrary: " << moduleFilePath;

	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

	pre_lib_unload_hooks_patches(filename);
	dll_cache::remove_from_cache(filename);
	return mHook_FreeLibrary->getInlineHook().call<BOOL, HMODULE>(hLibModule); 
}


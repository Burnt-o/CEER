#include "pch.h"
#include "hook_manager.h"
#include "windows_utilities.h"
//#include "patch.h"
#include "dll_cache.h"
#include "hook.h"
#include "enemy_randomizer.h"

std::vector<IHook*> gAllHooks;


// For keeping track fo dll loading and unloading
typedef HMODULE(*LoadLibraryA_t)(LPCSTR lpLibFileName);
HMODULE hkLoadLibraryA(LPCSTR lpLibFileName);


typedef HMODULE(*LoadLibraryW_t)(LPCWSTR lpLibFileName);
HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName);


typedef HMODULE(*LoadLibraryExA_t)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);


typedef HMODULE(*LoadLibraryExW_t)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);


typedef BOOL(*FreeLibrary_t)(HMODULE hLibModule);
BOOL hkFreeLibrary(HMODULE hLibModule);

void hookEnemySpawn(SafetyHookContext& ctx) {
	PLOG_VERBOSE << "Enemy spawning, actv index: " << ctx.rcx;
	ctx.rcx = 3;


}

inline_hook* GlobalHook_LoadLibraryA;
inline_hook* GlobalHook_LoadLibraryW;
inline_hook* GlobalHook_LoadLibraryExA;
inline_hook* GlobalHook_LoadLibraryExW;
inline_hook* GlobalHook_FreeLibrary;

//mid_hook* CEEnemyHook1;

MultilevelPointer* pCEEnemyHook1 = MultilevelPointer::make(std::wstring(L"halo1.dll"), { 0xC540B5 });
MultilevelPointer* pLoadLibraryA = MultilevelPointer::make(&LoadLibraryA);
MultilevelPointer* pLoadLibraryW = MultilevelPointer::make(&LoadLibraryW);
MultilevelPointer* pLoadLibraryExA = MultilevelPointer::make(&LoadLibraryExA);
MultilevelPointer* pLoadLibraryExW = MultilevelPointer::make(&LoadLibraryExW);
MultilevelPointer* pFreeLibrary = MultilevelPointer::make(&FreeLibrary);

hook_manager::hook_manager()
{

	PLOG_INFO << "hook_manager initializing";

	// need to go on the heap
	GlobalHook_LoadLibraryA = new inline_hook(L"hkLoadLibraryA", pLoadLibraryA, hkLoadLibraryA, true);
	GlobalHook_LoadLibraryW = new inline_hook(L"hkLoadLibraryW", pLoadLibraryW, hkLoadLibraryW, true);
	GlobalHook_LoadLibraryExA = new inline_hook(L"hkLoadLibraryExA", pLoadLibraryExA, hkLoadLibraryExA, true);
	GlobalHook_LoadLibraryExW = new inline_hook(L"hkLoadLibraryExW", pLoadLibraryExW, hkLoadLibraryExW, true);
	GlobalHook_FreeLibrary = new inline_hook(L"hkFreeLibrary", pFreeLibrary, hkFreeLibrary, true);





	gAllHooks.reserve(20);

	// do I need to cast these to IHook* ? 
	gAllHooks.push_back((IHook*)GlobalHook_LoadLibraryA);
	gAllHooks.push_back((IHook*)GlobalHook_LoadLibraryW);
	gAllHooks.push_back((IHook*)GlobalHook_LoadLibraryExA);
	gAllHooks.push_back((IHook*)GlobalHook_LoadLibraryExW);
	gAllHooks.push_back((IHook*)GlobalHook_FreeLibrary);
	gAllHooks.push_back((IHook*)mh_Hook_SpawnActvByIndex);
	gAllHooks.push_back((IHook*)mh_Hook_EvaluateEncounter);



}
hook_manager::~hook_manager()
{

}

void hook_manager::EnableEnemyRandomizer()
{
	enemy_randomizer_manager::er_set_hooks(true);
}




void hook_manager::attach_all() {
	PLOG_INFO << "attaching all hooks (that want to be enabled)";
	PLOG_DEBUG << "total hooks: " << gAllHooks.size();
	for (IHook* hook : gAllHooks) {
		if (hook->get_WantsToBeEnabled())
		{
			hook->attach();
		}
	}

}

//this only called on global_kill
void hook_manager::detach_all() {
	for (IHook* hook : gAllHooks) {
		hook->detach();
	}

}




void post_lib_load_hooks_patches(std::wstring_view libPath) {
	PLOG_DEBUG << "post_lib_load_hooks_patches iterating";
	std::filesystem::path path = libPath;
	auto filename = path.filename().generic_wstring();

	for (IHook* hook : gAllHooks) {
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

void pre_lib_unload_hooks_patches(std::wstring_view libFilename) {
	PLOG_DEBUG << "pre_lib_unload_hooks_patches iterating, unloaded module: " << libFilename;
	for (IHook* hook : gAllHooks) {
		if (hook->associated_module() == libFilename) {
			hook->detach();
		}
	}


}

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = GlobalHook_LoadLibraryA->getInlineHook().call<HMODULE, LPCSTR>(lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << "LoadLibraryA: " << wLibFileName;
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = GlobalHook_LoadLibraryW->getInlineHook().call<HMODULE, LPCWSTR>(lpLibFileName);

	PLOG_DEBUG << L"LoadLibraryW: " << lpLibFileName;
	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = GlobalHook_LoadLibraryExA->getInlineHook().call<HMODULE, LPCSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	PLOG_DEBUG << L"LoadLibraryExA: " << wLibFileName;
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = GlobalHook_LoadLibraryExW->getInlineHook().call<HMODULE, LPCWSTR, HANDLE, DWORD>(lpLibFileName, hFile, dwFlags);

	PLOG_DEBUG << L"LoadLibraryExW: " << lpLibFileName;
	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

BOOL hkFreeLibrary(HMODULE hLibModule) {
	PLOG_DEBUG << L"FreeLibrary";
	


	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));
	PLOG_DEBUG << L"FreeLibrary: " << moduleFilePath;

	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

	pre_lib_unload_hooks_patches(filename);
	dll_cache::remove_from_cache(filename);
	return GlobalHook_FreeLibrary->getInlineHook().call<BOOL, HMODULE>(hLibModule); 
}


#pragma once

typedef HMODULE(*LoadLibraryA_t)(LPCSTR lpLibFileName);
typedef HMODULE(*LoadLibraryW_t)(LPCWSTR lpLibFileName);
typedef HMODULE(*LoadLibraryExW_t)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(*LoadLibraryExA_t)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef BOOL(*FreeLibrary_t)(HMODULE hLibModule);

class hook_manager
{
private:
	//std::vector<IHook*> mAllHooks;
	static std::vector<std::shared_ptr<Hook>> mAllHooks;


	static std::shared_ptr<InlineHook> mHook_LoadLibraryA;
	static std::shared_ptr<InlineHook> mHook_LoadLibraryW;
	static std::shared_ptr<InlineHook> mHook_LoadLibraryExA;
	static std::shared_ptr<InlineHook> mHook_LoadLibraryExW;
	static std::shared_ptr<InlineHook> mHook_FreeLibrary;

	// For keeping track of dll loading and unloading
	static HMODULE newLoadLibraryA(LPCSTR lpLibFileName);
	static HMODULE newLoadLibraryW(LPCWSTR lpLibFileName);
	static HMODULE newLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	static HMODULE newLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	static BOOL newFreeLibrary(HMODULE hLibModule);

	
	static void pre_lib_unload_hooks_patches(std::wstring_view libFilename); // called by newFreeLibrary
	static void post_lib_load_hooks_patches(std::wstring_view libPath); // called by newLoadLibraries

public:
	hook_manager();
	~hook_manager();


	void detach_all();
	void EnableEnemyRandomizer();

	void addHook(std::shared_ptr<Hook> newHook)
	{
		mAllHooks.push_back(newHook);
	}




};
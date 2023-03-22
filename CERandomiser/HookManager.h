#pragma once
#include "Hook.h"




class HookManager
{
private:
	static HookManager& get() {
		static HookManager instance;
		return instance;
	}


	std::vector<std::shared_ptr<Hook>> mAllHooks;


	std::shared_ptr<InlineHook> mHook_LoadLibraryA;
	std::shared_ptr<InlineHook> mHook_LoadLibraryW;
	std::shared_ptr<InlineHook> mHook_LoadLibraryExA;
	std::shared_ptr<InlineHook> mHook_LoadLibraryExW;
	std::shared_ptr<InlineHook> mHook_FreeLibrary;

	// For keeping track of dll loading and unloading
	static HMODULE newLoadLibraryA(LPCSTR lpLibFileName);
	static HMODULE newLoadLibraryW(LPCWSTR lpLibFileName);
	static HMODULE newLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	static HMODULE newLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	static BOOL newFreeLibrary(HMODULE hLibModule);

	
	static void preModuleUnload_UpdateHooks(std::wstring_view libFilename); // called by newFreeLibrary
	static void postModuleLoad_UpdateHooks(std::wstring_view libPath); // called by newLoadLibraries

public:
	HookManager();
	~HookManager()
	{
		PLOG_VERBOSE << "HookManager destructor called";
	}


	void detachAllHooks();

	static void addHook(std::shared_ptr<Hook> newHook)
	{
		get().mAllHooks.push_back(newHook);
	}




};
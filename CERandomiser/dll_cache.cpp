#include "pch.h"
#include "dll_cache.h"

void dll_cache::initialize()
{
	auto& instance = get();

	// Clear the cache
	instance.mCache.clear();

	// Fill with current values
	HMODULE hMods[1024];
	HANDLE hProcess = GetCurrentProcess();
	DWORD cbNeeded;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			if (GetModuleBaseName(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				MODULEINFO info;
				if (GetModuleInformation(hProcess, hMods[i], &info, sizeof(info))) {
					std::wstring name{ szModName };
					instance.mCache.emplace(name, info);
				}
			}
		}
	}
}

bool dll_cache::add_to_cache(const std::wstring& dllName, HMODULE handle)
{
	auto& instance = get();

	// check if in cache
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		return false;
	}

	// if not add to cache
	MODULEINFO info;
	if (GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(info))) {
		instance.mCache.emplace(dllName, info);
	}
	else {
		throw std::exception("Could not cache module");
	}

	return true;
}

bool dll_cache::remove_from_cache(const std::wstring& dllName)
{
	auto& instance = get();
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		instance.mCache.erase(it);
		return true;
	}

	return false;
}

std::optional<HMODULE> dll_cache::get_module_handle(const std::wstring& dllName)
{
	auto& instance = get();
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		return (HMODULE)it->second.lpBaseOfDll;
	}

	return std::nullopt;
}

std::optional<MODULEINFO> dll_cache::get_module_info(const std::wstring& dllName)
{
	auto& instance = get();
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		return it->second;
	}

	return std::nullopt;
}

bool dll_cache::module_in_cache(const std::wstring& dllName)
{
	auto& instance = get();
	//instance.mCache.find()
	//return instance.mCache.contains(dllName);
	auto it = instance.mCache.find(dllName);
	return it != instance.mCache.end();

}
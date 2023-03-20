#pragma once


/// <summary>
/// The dll_cache contains the current status of ALL loaded DLLs
/// With hooks to LoadLibrary/FreeLibrary this should always be up to date.
/// </summary>
class dll_cache
{
private:
	// DLL Name, Base Address
	std::unordered_map<std::wstring, MODULEINFO> mCache;

	static dll_cache& get() {
		static dll_cache instance;
		return instance;
	}

	dll_cache() = default;
	~dll_cache() = default;

public:

	// Wipes the cache and gets the current status of ALL dlls
	// This is usually very expensive (10s of milliseconds) so don't call this often
	static void initialize();
	// Returns true if successfully added to cache
	static bool add_to_cache(const std::wstring& dllName, HMODULE handle);
	// Returns true if succssfully removed from cache
	static bool remove_from_cache(const std::wstring& dllName);

	static std::optional<HMODULE> get_module_handle(const std::wstring& dllName);
	static std::optional<MODULEINFO> get_module_info(const std::wstring& dllName);
	static bool module_in_cache(const std::wstring& dllName);
};
#include "pch.h"
#include "WindowsUtilities.h"


std::wstring str_to_wstr(const std::string str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wStr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wStr, wchars_num);
	return std::wstring(wStr);
}

std::string wstr_to_str(const std::wstring wstr)
{
	int chars_num = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, wstr.c_str(), -1, NULL, 0, NULL, NULL);

	char* str = new char[chars_num];
	WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, wstr.c_str(), -1, str, chars_num, NULL, NULL);
	return std::string(str);
}

void patch_pointer(void* dest_address, uintptr_t new_address)
{
	unsigned long old_protection, unused;
	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect(dest_address, 8, PAGE_EXECUTE_READWRITE, &old_protection);

	//write the memory into the program and overwrite previous value
	std::memcpy(dest_address, &new_address, 8);

	//reset the permissions of the address back to oldProtection after writting memory
	VirtualProtect(dest_address, 8, old_protection, &unused);
}

void patch_memory(void* dest_address, void* src_address, size_t size)
{
	unsigned long old_protection, unused;
	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect(dest_address, 8, PAGE_EXECUTE_READWRITE, &old_protection);

	//write the memory into the program and overwrite previous value
	std::memcpy(dest_address, src_address, size);

	//reset the permissions of the address back to oldProtection after writting memory
	VirtualProtect(dest_address, 8, old_protection, &unused);
}
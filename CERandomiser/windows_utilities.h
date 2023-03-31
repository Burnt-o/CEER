#pragma once

#define safe_release(p) if (p) { p->Release(); p = nullptr; } 
std::wstring str_to_wstr(const std::string str);
std::string wstr_to_str(const std::wstring wstr);


void patch_pointer(void* dest_address, uintptr_t new_address);
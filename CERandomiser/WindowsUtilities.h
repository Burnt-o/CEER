#pragma once

#define nameof(x) #x

#define safe_release(p) if (p) { p->Release(); p = nullptr; } 

std::wstring str_to_wstr(const std::string str);
std::string wstr_to_str(const std::wstring wstr);


void patch_pointer(void* dest_address, uintptr_t new_address);
void patch_memory(void* dest_address, void* src_address, size_t size);

void acquire_global_unhandled_exception_handler();
void release_global_unhandled_exception_handler();

std::string ResurrectException();
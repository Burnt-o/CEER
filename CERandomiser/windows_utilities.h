#pragma once


std::wstring str_to_wstr(const std::string str);
std::string wstr_to_str(const std::wstring wstr);

void acquire_global_unhandled_exception_handler();
void release_global_unhandled_exception_handler();
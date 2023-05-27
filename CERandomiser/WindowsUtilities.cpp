#include "pch.h"
#include "WindowsUtilities.h"
#include "InitParameter.h"

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


void make_minidump(EXCEPTION_POINTERS* e)
{
	auto hDbgHelp = LoadLibraryA("dbghelp");
	if (hDbgHelp == nullptr)
		return;
	auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	if (pMiniDumpWriteDump == nullptr)
		return;

	SYSTEMTIME t;
	GetSystemTime(&t);
	std::string dumpFilePath = g_ourInitParameters->injectorPath + std::format(
			"\\CEER_MCCDMP_{:04}{:02}{:02}_{:02}{:02}{:02}.dmp",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	

	auto hFile = CreateFileA(dumpFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		PLOG_FATAL << "Failed to create crash dump file at " << dumpFilePath;
		return;
	}


	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = e;
	exceptionInfo.ClientPointers = FALSE;

	pMiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
		e ? &exceptionInfo : nullptr,
		nullptr,
		nullptr);
	PLOG_FATAL << "Dumped crash information to " << dumpFilePath;
	CloseHandle(hFile);

	return;
}

static LPTOP_LEVEL_EXCEPTION_FILTER OriginalUnhandledExceptionFilter;
LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
	PLOG_FATAL << "Unhandled exception, creating minidump!";
	make_minidump(e);
	return EXCEPTION_CONTINUE_SEARCH;
}

void acquire_global_unhandled_exception_handler()
{
	OriginalUnhandledExceptionFilter = SetUnhandledExceptionFilter(unhandled_handler);
}

void release_global_unhandled_exception_handler()
{
	if (OriginalUnhandledExceptionFilter) {
		SetUnhandledExceptionFilter(OriginalUnhandledExceptionFilter);
	}
}


std::string ResurrectException()
{
try {
	throw;
}
catch (const std::exception& e) {
	return e.what();
}
catch (...) {
	return "�nknown exception!";
}
}
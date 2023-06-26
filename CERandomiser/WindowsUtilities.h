#pragma once

#define nameof(x) #x

#define safe_release(p) if (p) { p->Release(); p = nullptr; } 


extern std::uniform_real_distribution<double> zeroToOne;

// decent chunk of this nicked from Scales' MCCTAS, with slight modifications
std::wstring str_to_wstr(const std::string str);
std::string wstr_to_str(const std::wstring wstr);


void patch_pointer(void* dest_address, uintptr_t new_address);
void patch_memory(void* dest_address, void* src_address, size_t size);

void acquire_global_unhandled_exception_handler();
void release_global_unhandled_exception_handler();

std::string ResurrectException();

std::string getShortName(std::string in);



struct VersionInfo
{
	DWORD major, minor, build, revision;
	friend std::ostream& operator << (std::ostream& o, VersionInfo const& a)
	{
		o << std::format("{}.{}.{}.{}", a.major, a.minor, a.build, a.revision);
		return o;
	}

	bool operator == (const VersionInfo& other)
	{
		return major == other.major && minor == other.minor && build == other.build && revision == other.revision;
	}
};



VersionInfo getFileVersion(const char* filename);

#define acronymOf(x) getShortName(#x).c_str()
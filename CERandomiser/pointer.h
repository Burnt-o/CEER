#pragma once
#include "pch"
enum class pointer_type { RESOLVED, MODULEOFFSET, BASEOFFSET };

class pointer {
private:
	pointer_type mPointerType;
	void* mBaseAddress;
	std::vector<int64_t> mOffsets;
	std::wstring mModuleName; // "main" is keyword for executable, everything else is eg "halo1.dll" 
	std::optional<void*> dereference_pointer(void* base, std::vector<int64_t> offsets);

public:
	pointer() = default;
	pointer(void* const& baseAddress)
		: mBaseAddress(baseAddress), mPointerType(pointer_type::RESOLVED), mModuleName(L""), mOffsets()
	{
		PLOG_VERBOSE << "RESOLVED pointer constructed";

		this->mPointerType = pointer_type::RESOLVED;
		PLOG_VERBOSE << "THIS SHOULD BE THINGY: " << (int)this->mPointerType;
	}

	pointer(const std::wstring moduleName, const std::vector<int64_t>& offsets)
		: mModuleName(moduleName), mOffsets(offsets), mPointerType(pointer_type::MODULEOFFSET), mBaseAddress(nullptr)
	{
		PLOG_VERBOSE << "MODULEOFFSET pointer constructed";

		this->mPointerType = pointer_type::MODULEOFFSET;
		PLOG_VERBOSE << "THIS SHOULD BE THINGY: " << (int)this->mPointerType;
	}

	pointer(void* const& baseAddress, const std::vector<int64_t>& offsets)
		: mBaseAddress(baseAddress), mOffsets(offsets), mPointerType(pointer_type::BASEOFFSET), mModuleName(L"")
	{
		PLOG_VERBOSE << "BASEOFFSET pointer constructed";

		this->mPointerType = pointer_type::BASEOFFSET;
		PLOG_VERBOSE << "THIS SHOULD BE THINGY: " << (int)this->mPointerType;
	}

	std::optional<void*> resolve();

	// Could add a system for caching the results of resolve, but I don't think we really need the marginal performance improvement
};



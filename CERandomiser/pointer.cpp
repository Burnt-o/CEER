#include "pch.h"
#include "pointer.h"
#include "dll_cache.h"



std::optional<void*> pointer::dereference_pointer(void* base, std::vector<int64_t> offsets)
{
	uintptr_t baseAddress = (uintptr_t)base; //cast to uintptr_t so we can do math to it
	if (offsets.size() > 0)
	{
		baseAddress += offsets[0];
	
		for (int i = 1; i < offsets.size(); i++) //skip the first offset since we already handled it
		{
			if (IsBadReadPtr((void*)baseAddress, 8)) // check that it's good to read before we read it
			{
				PLOG_INFO << "failed dereferencing pointer, bad read @ 0x" << baseAddress;
				return std::nullopt;
			}

			baseAddress = (uintptr_t)*(void**)baseAddress; // read the value at baseaddress and assign it to base address
			baseAddress += offsets[i]; // add the offset
		}
	}

	if (IsBadReadPtr((void*)baseAddress, 8)) // check that it's still good to read
	{
		PLOG_INFO << "failed dereferencing pointer, bad read @ 0x" << baseAddress;
		return std::nullopt;
	}

	return (void*)baseAddress;

}


std::optional<void*> pointer::resolve()
{
	PLOG_VERBOSE << "resolving pointer, type: " << (int)this->mPointerType;
	void* deferenceBase = nullptr;
	uintptr_t baseIntPtr = (uintptr_t)0;
	std::optional<void*> moduleAddress = std::nullopt;
	
	switch (this->mPointerType)
	{
	case pointer_type::RESOLVED:
		return IsBadReadPtr(this->mBaseAddress, 1) ? std::nullopt : std::optional(this->mBaseAddress);
		break;


	case pointer_type::MODULEOFFSET:
		PLOG_VERBOSE << "REEEE";
		moduleAddress = dll_cache::get_module_handle(this->mModuleName);
		if (!moduleAddress.has_value()) return std::nullopt;
		PLOG_VERBOSE << "moduleAddress: " << moduleAddress.value();
		return dereference_pointer(moduleAddress.value(), this->mOffsets);
		break;

	case pointer_type::BASEOFFSET:
		return dereference_pointer(this->mBaseAddress, this->mOffsets);
		break;


	default:
		break;

	}
}





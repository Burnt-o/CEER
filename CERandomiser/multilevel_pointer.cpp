#include "pch.h"
#include "multilevel_pointer.h"
#include "dll_cache.h"
#include "windows_utilities.h"

void* PointerTypes::ExeOffset::mEXEAddress = nullptr;


bool MultilevelPointer::dereference_pointer(void* base, std::vector<int64_t> offsets, void** resolvedOut) const
{
	uintptr_t baseAddress = (uintptr_t)base; //cast to uintptr_t so we can do math to it
	if (offsets.size() > 0)
	{
		for (int i = 0; i < offsets.size(); i++) //skip the first offset since we already handled it
		{
			if (IsBadReadPtr((void*)baseAddress, 8)) // check that it's good to read before we read it
			{
				*SetLastErrorByRef() << "dereferencing failed during looping of offsets" << std::endl
					<< "base: " << base << std::endl
					<< "offsets index at failure" << i << std::endl
					<< "offsets.size() " << offsets.size() << std::endl
					<< "base += offsets[0]: " << (void*)baseAddress << std::endl
					<< "bad read address: " << base << std::endl;
				return false;
			}

			if (i == 0)
			{
				baseAddress += offsets[0];
			}
			else
			{
				baseAddress = (uintptr_t) * (void**)baseAddress; // read the value at baseaddress and assign it to base address
				baseAddress += offsets[i]; // add the offset
			}

		}
	}

	if (IsBadReadPtr((void*)baseAddress, 8)) // check that it's still good to read now that we're done with it
	{
		*SetLastErrorByRef() << "dereferencing failed after looping through offsets" << std::endl
			<< "base: " << base << std::endl
			<< "offsets.size() " << offsets.size() << std::endl
			<< "base += offsets[0]: " << (void*)baseAddress << std::endl
			<< "bad read address: " << base << std::endl;
		return false;
	}

	*resolvedOut = (void*)baseAddress;
	return true;;

}

bool PointerTypes::ExeOffset::resolve(void** resolvedOut) const
{
	if (!PointerTypes::ExeOffset::mEXEAddress)
	{
		PointerTypes::ExeOffset::mEXEAddress = GetModuleHandleA(NULL);// null means get the handle for the currently running process
		if (!PointerTypes::ExeOffset::mEXEAddress) { PLOG_ERROR << "Couldn't get exe process handle"; throw("Couldn't get exe process handle"); }
	}
	return dereference_pointer(this->mEXEAddress, this->mOffsets, resolvedOut);
}

bool PointerTypes::BaseOffset::resolve(void** resolvedOut) const
{
	return dereference_pointer(this->mBaseAddress, this->mOffsets, resolvedOut);
}

bool PointerTypes::ModuleOffset::resolve(void** resolvedOut) const
{
	auto moduleAddress = dll_cache::get_module_handle(this->mModuleName);
	if (!moduleAddress.has_value()) 
	{
		*SetLastErrorByRef() << "ModuleOffset resolution failed, module not found with name: " << wstr_to_str(this->mModuleName) << std::endl;
		return false;
	}
	PLOG_VERBOSE << "moduleAddress: " << moduleAddress.value();

	return dereference_pointer(moduleAddress.value(), this->mOffsets, resolvedOut);
}

bool PointerTypes::Resolved::resolve(void** resolvedOut) const
{
	if (IsBadReadPtr(this->mBaseAddress, 1))
	{
		*SetLastErrorByRef() << "Resolved resolution failed, bad read pointer: " << this->mBaseAddress << std::endl;
		return false;
	}
	*resolvedOut = (void*)this->mBaseAddress;
	return true;
}




void PointerTypes::BaseOffset::updateBaseAddress(void* const& baseAddress)
{
	this->mBaseAddress = baseAddress;
}


// special readData variation for strings, to handle MCC's short-string-optimization
bool MultilevelPointer::readString(std::string& resolvedOut) const
{


	void* pString;
	if (!this->resolve(&pString)) return false;

	PLOG_VERBOSE << "readString parsing @" << pString;

	uint64_t strLength = *(uint64_t*)(((uintptr_t)pString) + 0x10); // int value of string length is stored at +0x10 from string start
	uint64_t strCapacity = *(uint64_t*)(((uintptr_t)pString) + 0x18); // int value of string length is stored at +0x10 from string start

	PLOG_VERBOSE << "readString parsing strLength @" << std::hex << (((uintptr_t)pString) + 0x10);
	PLOG_VERBOSE << "readString parsing strCapacity @" << std::hex << (((uintptr_t)pString) + 0x18);

	// Validity checks
	if (strLength == 0) // empty string or invalid pointer
	{
		*SetLastErrorByRef() << "readString failed, strLength was zero : " << std::hex << strLength << std::endl;
		return false;
	}

	if (strCapacity < 0x0F) // Capacity will always be atleast 0x0F (size of short-string buffer)
	{
		*SetLastErrorByRef() << "readString failed, strCapacity was less than 0x10 : " << std::hex << strCapacity << std::endl;
		return false;
	}

	if (strLength > strCapacity) // Capacity will always bigger or equal to Length
	{
		*SetLastErrorByRef() << "readString failed, strCapacity was less than strLength : " << std::hex << strLength << " : " << strCapacity << std::endl;
		return false;
	}

	strLength += 1; // Add one to our strLength so we can get the null termination character too

	auto potentialChars = std::make_unique<char[]>(strLength); // + 1 so we get the null termination character too

	// Handle shortstring vs longstring
	if (strLength <= 0x10)
	{
		PLOG_VERBOSE << "short string case";
		//short string case - the string is stored in the buffer
		std::memcpy(potentialChars.get(), pString, strLength); // our void* is actually the char*, so we can copy that

	}
	else
	{
		PLOG_VERBOSE << "long string case";
		//long string case. follow the pointer in the buffer to the real char* (ie we're dealing with a char**)
		std::memcpy(potentialChars.get(), *(void**)pString, strLength);
	}

	// Confirm that our string is valid
	// The string should contain a null termination char at the END, and ONLY at the end
	for (int i = 0; i < strLength; i++)
	{
		// if not the last char in the string
		if (i != strLength - 1)
		{
			// failure if it's a null char
			if (potentialChars.get()[i] == '\0')
			{
				*SetLastErrorByRef() << "readString failed, null termination character found before end of string" << std::endl;
				return false;
			}
		}
		else // we're at end of string
		{
			// failure if it's NOT a null char
			if (potentialChars.get()[i] != '\0')
			{
				*SetLastErrorByRef() << "readString failed, null termination character not found at end of string" << std::endl;
				return false;
			}
		}
	}

	// copy chars to a string
	resolvedOut = std::string(potentialChars.get());

	return true;



}

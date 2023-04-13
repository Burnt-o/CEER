#pragma once
#include "pch.h"







// base factory class
class MultilevelPointer {
private:

	// Common to all multilevel_pointers
	static inline std::stringstream mLastError{};
	bool readString(std::string& resolvedOut) const; // special case of readData that handles short-string-optimization

protected:
	bool dereferencePointer(const void* const& base, const std::vector<int64_t>& offsets, void** resolvedOut) const;
	static std::stringstream* SetLastErrorByRef()
	{
		mLastError.clear();
		mLastError.str("");
		return &mLastError;
	}
	virtual ~MultilevelPointer() = default;
public:


	// Factory methods
	// Pointer is relative to the exe address
	static std::shared_ptr<MultilevelPointer> make(const std::vector<int64_t> offsets);
	//static MultilevelPointer* make(const std::vector<int64_t>& offsets);

	// Pointer is relative to some void* baseAddress
	static std::shared_ptr<MultilevelPointer> make(void* const baseAddress, const std::vector<int64_t> offsets);

	// Pointer is relative to a module eg halo1.dll
	static std::shared_ptr<MultilevelPointer> make(const std::wstring_view moduleName, const std::vector<int64_t> offsets);

	// Pointer is already fully resolved (used for stuff that never changes address)
	static std::shared_ptr<MultilevelPointer> make(void* const baseAddress);

	// The useful stuff
	virtual bool resolve(void** resolvedOut) const = 0; // Overriden in derived classes

	template<typename T>
	bool readData(T* resolvedOut) const
	{
		if (typeid(T) == typeid(std::string))
		{
			return readString(*(std::string*)resolvedOut);
		}

		void* address;
		if (!this->resolve(&address)) return false;

		*resolvedOut = *(T*)address;
		return true;
	}

	template<typename T>
	bool readArrayData(T* resolvedOut, size_t arraySize) const
	{
		if (typeid(T) == typeid(std::string))
		{
			return readString(*(std::string*)resolvedOut);
		}

		void* address;
		if (!this->resolve(&address)) return false;

		memcpy(resolvedOut, address, arraySize);
		return true;
	}

	static std::string GetLastError()
	{
		return mLastError.str();
	}

};


namespace PointerTypes // So we don't pollute global namespace
{
	class ExeOffset : public MultilevelPointer {
	private:
		const std::vector<int64_t> mOffsets;
		static void* mEXEAddress;
	public:
		explicit ExeOffset(const std::vector<int64_t> offsets) : mOffsets(offsets) {}
		bool MultilevelPointer::resolve(void** resolvedOut) const override;
	};

	class BaseOffset : public MultilevelPointer {
	private:
		void* mBaseAddress;
		const std::vector<int64_t> mOffsets;
	public:
		explicit BaseOffset(void* const& baseAddress, const std::vector<int64_t> offsets) : mBaseAddress(baseAddress), mOffsets(offsets) {}
		bool MultilevelPointer::resolve(void** resolvedOut) const override;
		void updateBaseAddress(void* const& baseAddress);
	};

	class ModuleOffset : public MultilevelPointer {
	private:
		const std::wstring mModuleName;
		const std::vector<int64_t> mOffsets;
	public:
		explicit ModuleOffset(const std::wstring_view& moduleName, const std::vector<int64_t> offsets) : mModuleName(moduleName), mOffsets(offsets) {}
		bool MultilevelPointer::resolve(void** resolvedOut) const override;
	};

	class Resolved : public MultilevelPointer {
	private:
		const void* mBaseAddress;
	public:
		explicit Resolved(void* const& baseAddress) : mBaseAddress(baseAddress) {}
		bool MultilevelPointer::resolve(void** resolvedOut) const override;
	};



}


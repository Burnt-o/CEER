#pragma once

#include "multilevel_pointer.h"
#include "midhook_context_interpreter.h"

// Uses libcurl to download PointerData.xml from the github page,
// instantiating all the MultilevelPointers (and other data) specific to the current MCC version we were injected into

class PointerManager
{

private:	
	// singleton
	static PointerManager& get() {
		static PointerManager instance;
		return instance;
	}


	class PointerManagerImpl;
	std::unique_ptr<PointerManagerImpl> impl;
public:
	PointerManager();
	~PointerManager();

	static std::shared_ptr<MultilevelPointer> getMultilevelPointer(std::string dataName);
	static std::shared_ptr<MidhookContextInterpreter> getMidhookContextInterpreter(std::string dataName);
};
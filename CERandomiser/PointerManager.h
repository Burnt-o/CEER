#pragma once

#include "multilevel_pointer.h"
#include "midhook_context_interpreter.h"
#include "curl/curl.h"
// Uses libcurl to download PointerData.xml from the github page,
// instantiating all the MultilevelPointers (and other data) specific to the current MCC version we were injected into
class PointerManager
{

private:

	// Functions run by constructor, in order of execution
	void downloadXML(std::string url, std::string& outXML);
	void readXMLlocally(std::string pathToFile, std::string& outXML);
	void getCurrentMCCVersion(std::string& outCurrentMCCVersion);
	void filterXMLtoCurrentVersion(std::string currentMCCVersion, std::string& outXML);
	void instantiateMultilevelPointers(std::string xml);
	void instantiateMidhookContextInterpreters(std::string xml);

	// data mapped by strings
	std::map<std::string, std::shared_ptr<MultilevelPointer>> mMultilevelPointerData;
	std::map<std::string, std::shared_ptr<MidhookContextInterpreter>> mMidhookContextInterpreterData;

public:
	PointerManager();
	~PointerManager() = default;

	bool getMultilevelPointer(std::string dataName);
	bool getMidhookContextInterpreter(std::string dataName);
};


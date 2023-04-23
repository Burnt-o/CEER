#include "pch.h"
#include "PointerManager.h"
#include "curl/curl.h" // to get PointerData.xml from github
#include <winver.h> // to get version string of MCC
#include <pugixml.hpp>

#define useDevPointerData 1



class PointerManager::PointerManagerImpl {


    private:

        // Functions run by constructor, in order of execution
        void downloadXML(std::string url);
        std::string readLocalXML();
        std::string getCurrentMCCVersion();
        void parseXML(std::string& xml);
        void processVersionedEntry(pugi::xml_node entry);
        void instantiateMultilevelPointer(pugi::xml_node entry, std::string entryType, std::string entryName);
        void instantiateMidhookContextInterpreter(pugi::xml_node entry, std::string entryName);

        std::string currentGameVersion;



    public:
        PointerManagerImpl();
        ~PointerManagerImpl() = default;

        // data mapped by strings
        static std::map<std::string, std::shared_ptr<MultilevelPointer>> mMultilevelPointerData;
        static std::map<std::string, std::shared_ptr<MidhookContextInterpreter>> mMidhookContextInterpreterData;
};

PointerManager::PointerManager() : impl(new PointerManagerImpl) {}
PointerManager::~PointerManager() = default; // https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/

PointerManager::PointerManagerImpl::PointerManagerImpl()
{

    try
    {
        PLOG_INFO << "downloading pointerData.xml";
        downloadXML("https://raw.githubusercontent.com/Burnt-o/HaloCheckpointManager/HCM2/HCM3/PointerData.xml");
    }
    catch (InitException ex)
    {
        PLOG_ERROR << "Failed to download CEER PointerData xml, trying local backup";
    }


    std::string pointerData = readLocalXML();
    this->currentGameVersion = getCurrentMCCVersion();
    PLOG_INFO << "MCC Version: " << currentGameVersion;

    parseXML(pointerData);
    PLOG_DEBUG << "pointer parsing complete";
    for (const auto& [key, value] : mMultilevelPointerData)
    {
        PLOG_DEBUG << std::format("stored key: {}", key);
    }

}



std::shared_ptr<MultilevelPointer> PointerManager::getMultilevelPointer(std::string dataName)
{
    if (!get().impl.get()->mMultilevelPointerData.contains(dataName))
    {
        PLOG_ERROR << "no valid pointer data for " << dataName;
        throw InitException(std::format("pointerData was null for {}", dataName));
    }

    return get().impl.get()->mMultilevelPointerData.at(dataName);

}

std::shared_ptr<MidhookContextInterpreter> PointerManager::getMidhookContextInterpreter(std::string dataName)
{
    if (!get().impl.get()->mMidhookContextInterpreterData.contains(dataName))
    {
        PLOG_ERROR << "no valid pointer data for " << dataName;
        throw InitException(std::format("pointerData was null for {0}", dataName));
    }

    return get().impl.get()->mMidhookContextInterpreterData.at(dataName);
}


std::map<std::string, std::shared_ptr<MultilevelPointer>> PointerManager::PointerManagerImpl::mMultilevelPointerData{};
std::map<std::string, std::shared_ptr<MidhookContextInterpreter>> PointerManager::PointerManagerImpl::mMidhookContextInterpreterData{};






std::string ExePath() {
    char buffer[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}


std::string PointerManager::PointerManagerImpl::readLocalXML()
{
    std::string pathToFile;
#if useDevPointerData == 1
    pathToFile = "C:\\Users\\mauri\\source\\repos\\CERandomiser\\CERandomiser\\CEERPointerData.xml";
#else
    pathToFile = ExePath() + "CEERPointerData.xml";
#endif

    std::ifstream inFile(pathToFile.c_str());
    if (inFile.is_open())
    {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();
        PLOG_INFO << "local XML read succesfully!";
        return buffer.str();
    }
    else
    {
        std::stringstream er;
        er << "Failed to open file : " << GetLastError() << std::endl << "at: " << pathToFile;
        throw InitException(er.str().c_str());
    }

}


#pragma region libcurl helpers
// write the data into a `std::string` rather than to a file.
std::size_t write_data(void* buf, std::size_t size, std::size_t nmemb,
    void* userp)
{
    if (auto sp = static_cast<std::string*>(userp))
    {
        sp->append(static_cast<char*>(buf), size * nmemb);
        return size * nmemb;
    }

    return 0;
}

// A deleter to use in the smart pointer for automatic cleanup
struct curl_dter {
    void operator()(CURL* curl) const
    {
        if (curl) curl_easy_cleanup(curl);
    }
};

// A smart pointer to automatically clean up out CURL session
using curl_uptr = std::unique_ptr<CURL, curl_dter>;
#pragma endregion //libcurl helpers

void PointerManager::PointerManagerImpl::downloadXML(std::string url)
{
    // Download from the intertubes
    std::string xmlString;
    if (auto curl = curl_uptr(curl_easy_init()))
    {
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &xmlString);

        CURLcode ec;
        if ((ec = curl_easy_perform(curl.get())) != CURLE_OK)
            throw InitException(curl_easy_strerror(ec));

    }

    // Write to local file
    std::string pathToFile = ExePath() + "CEERPointerData.xml";
    std::ofstream outFile(pathToFile.c_str());
    if (outFile.is_open())
    {
        outFile << xmlString;
        outFile.close();
        PLOG_INFO << "local XML file written!";
        return;
    }
    else
    {
        std::stringstream er;
        er << "Failed to write file : " << GetLastError() << std::endl << "at: " << pathToFile;
        throw InitException(er.str().c_str());
    }

}



std::string getFileVersion(const char* filename)
{
    DWORD dwHandle, size = GetFileVersionInfoSizeA(filename, &dwHandle);
    if (size == 0)
    {
        throw InitException(std::format("fileInfoVersionSize was zero, error: {}", GetLastError()));

    }

    std::vector<char> buffer;
    buffer.reserve(size);


    if (!GetFileVersionInfoA(filename, dwHandle, size, buffer.data()))
    {
        throw InitException(std::format("GetFileVersionInfoA failed, error: {}", GetLastError()));
    }

    VS_FIXEDFILEINFO* pvi;
    size = sizeof(VS_FIXEDFILEINFO);
    if (!VerQueryValueA(buffer.data(), "\\", (LPVOID*)&pvi, (unsigned int*)&size))
    {
        throw InitException(std::format("VerQueryValueA failed, error: {}", GetLastError()));
    }
    std::string outVersionInfo = std::format("{}.{}.{}.{}",
        pvi->dwProductVersionMS >> 16,
        pvi->dwFileVersionMS & 0xFFFF,
        pvi->dwFileVersionLS >> 16,
        pvi->dwFileVersionLS & 0xFFFF
    );

    return outVersionInfo;

}

std::string PointerManager::PointerManagerImpl::getCurrentMCCVersion()
{
    std::string outCurrentMCCVersion;
    HMODULE mccProcess = GetModuleHandle(NULL);
    char mccProcessPath[MAX_PATH];
    GetModuleFileNameA(mccProcess, mccProcessPath, sizeof(mccProcessPath));

    outCurrentMCCVersion = getFileVersion(mccProcessPath);

    PLOG_DEBUG << "mccVersionInfo: " << outCurrentMCCVersion;
    PLOG_DEBUG << "size: " << outCurrentMCCVersion.size();
    if (outCurrentMCCVersion.size() != 10)
    {
        throw InitException("mccVersionInfo was incorrect size!");
    }
    if (!outCurrentMCCVersion.starts_with("1."))
    {
        throw InitException("mccVersionInfo did not start with \"1.\"!");
    }

    return outCurrentMCCVersion;

}




void PointerManager::PointerManagerImpl::parseXML(std::string& xml)
{
    using namespace pugi;
    xml_document doc;
    xml_parse_result result = doc.load_string(xml.c_str());
    if (!result)
    {
        std::stringstream er;
        er << "XML [" << xml << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
        er << "Error description: " << result.description() << "\n";
        er << "Error offset: " << result.offset << " (error at [..." << (xml.c_str() + result.offset) << "]\n\n";
        er << "xml in question: \n\n" << xml;
        throw InitException(er.str());
    }

    xml_node root = doc.child("root");

    for (xml_node entry = root.first_child(); entry; entry = entry.next_sibling())
    //for (xml_node entry : root.children())
    {
        std::string entryName = entry.name();
        PLOG_DEBUG << "Processing entry, name: " << entryName;
        if (entryName == "VersionedEntry")
        {
                processVersionedEntry(entry);
        }

    }



}

void PointerManager::PointerManagerImpl::processVersionedEntry(pugi::xml_node entry)
{
    using namespace pugi;
    std::string entryName = entry.attribute("Name").value();
    PLOG_DEBUG << "processing versionedEntry, name: " << entry.name();
    for (xml_node versionEntry = entry.first_child(); versionEntry; versionEntry = versionEntry.next_sibling())
    {
        if (strcmp(versionEntry.attribute("Version").value(), currentGameVersion.c_str())) // We only want the versionEntries of the current MCC version
        {
            PLOG_VERBOSE << "No version match";
            continue;
        }
        PLOG_DEBUG << "Matching version found";

        std::string entryType = entry.attribute("Type").value(); // Convert to std::string
        // Check what type it is
        if (entryType.starts_with("MultilevelPointer"))
        {
            PLOG_DEBUG << "instantiating MultilevelPointer";
            instantiateMultilevelPointer(versionEntry, entryType, entryName);

        }
        else if (entryType.starts_with("MidhookContextInterpreter"))
        {
            PLOG_DEBUG << "instantiating MidhookContextInterpreter";
            instantiateMidhookContextInterpreter(versionEntry, entryName);
        }
    }
}

// Checks for "0x" to know if it's a hex string. Also checks for negatives ("-" at start)
int64_t stringToInt(std::string& string)
{
    std::string justNumbers = string;
    bool negativeFlag = string.starts_with("-");
    if (negativeFlag) justNumbers = string.substr(1); // remove the negative sign from string
    bool hexFlag = string.starts_with("0x");
    if (hexFlag) justNumbers = justNumbers.substr(2); // remove the hex sign from string


    try
    {
        int64_t result = stoi(string, 0, hexFlag ? 16 : 10);

        if (negativeFlag) result *= -1;
        PLOG_VERBOSE << std::format("stringToInt converted {0} to {1}", string, result);
        return result;
    }
    catch (std::invalid_argument ex)
    {
        throw InitException(std::format("Error parsing string to int: {}", string));
    }

}


std::vector<int64_t> getOffsetsFromXML(pugi::xml_node versionEntry)
{
    using namespace pugi;
    xml_node offsetArray = versionEntry.child("Offsets");
    std::vector<int64_t> result;

    for (xml_node offsetElement = offsetArray.first_child(); offsetElement; offsetElement = offsetElement.next_sibling())
    {
        std::string offsetString = offsetElement.text().get();
        result.emplace_back(stringToInt(offsetString));
    }

    return result;

}


void PointerManager::PointerManagerImpl::instantiateMultilevelPointer(pugi::xml_node versionEntry, std::string entryType, std::string entryName)
{
    std::shared_ptr<MultilevelPointer> result;
    if (entryType == "MultilevelPointer::ExeOffset")
    {
        PLOG_DEBUG << "exeOffset";
        auto offsets = getOffsetsFromXML(versionEntry);
        result = MultilevelPointer::make(offsets);
    }
    else if (entryType == "MultilevelPointer::ModuleOffset")
    {
        PLOG_DEBUG << "moduleOffset";
        std::string moduleString = versionEntry.child("Module").text().get();
        auto offsets = getOffsetsFromXML(versionEntry);
        result = MultilevelPointer::make(str_to_wstr(moduleString), offsets);
    }
    else
    {
        PLOG_ERROR << "INVALID MULTILEVEL POINTER TYPE: " << entryType;
        return;
    }

    PLOG_DEBUG << "MultilevelPointer added to map: " << entryName;
    mMultilevelPointerData.try_emplace(entryName, result);

}


void PointerManager::PointerManagerImpl::instantiateMidhookContextInterpreter(pugi::xml_node versionEntry, std::string entryName)
{
    std::shared_ptr<MidhookContextInterpreter> result;
    std::vector<Register> parameterRegisters;

    using namespace pugi;

    xml_node paramArray = versionEntry.first_child();
    for (xml_node parameter = paramArray.first_child(); parameter; parameter = parameter.next_sibling())
    {
        std::string registerText = parameter.text().as_string();
        if (!stringToRegister.contains(registerText))
        {
            throw InitException(std::format("invalid parameter string when parsing MidhookContextInterpreter->{}: {}", entryName, registerText));
        }

        parameterRegisters.push_back(stringToRegister.at(registerText));
    }

    if (parameterRegisters.empty())
    {
        throw InitException(std::format("no parameter strings found when parsing MidhookContextInterpreter {}", entryName));
    }

    result = std::make_shared<MidhookContextInterpreter>(parameterRegisters);

    PLOG_DEBUG << "MidhookContextInterpreter added to map: " << entryName;
    mMidhookContextInterpreterData.try_emplace(entryName, result);
   
}

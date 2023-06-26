#include "pch.h"
#include "PointerManager.h"
#include "curl/curl.h" // to get PointerData.xml from github
#include <winver.h> // to get version string of MCC
#include <pugixml.hpp>
#include "InitParameter.h"
#include "CEERVersioning.h"
#define useDevPointerData 1
#define debugPointerManager 1

PointerManager* PointerManager::instance = nullptr;

enum class MCCProcessType
{
    Steam,
    WinStore
};


class PointerManager::PointerManagerImpl {


    private:
        std::string pointerDataLocation;

        // Functions run by constructor, in order of execution
        void downloadXML(std::string url);
        std::string readLocalXML();
        VersionInfo getCurrentMCCVersion();
        MCCProcessType getCurrentMCCType();
        void parseXML(std::string& xml);
        VersionInfo processLatestCEERVersion(pugi::xml_node entry);
        void processVersionedEntry(pugi::xml_node entry);
        void instantiateMultilevelPointer(pugi::xml_node entry, std::string entryType, std::string entryName);
        void instantiateMidhookContextInterpreter(pugi::xml_node entry, std::string entryName);
        template <typename T>
        void instantiateVectorFloat(pugi::xml_node entry, std::string entryName);
        template <typename T>
        void instantiateVectorInteger(pugi::xml_node entry, std::string entryName);


        std::string currentGameVersion;
        MCCProcessType currentProcessType;


    public:
        PointerManagerImpl();
        ~PointerManagerImpl() = default;

        // data mapped by strings
        static std::map<std::string, std::shared_ptr<MultilevelPointer>> mMultilevelPointerData;
        static std::map<std::string, std::shared_ptr<MidhookContextInterpreter>> mMidhookContextInterpreterData;
        static std::map<std::string, std::vector<std::any>> mVectorData;
};

PointerManager::PointerManager() : impl(new PointerManagerImpl) 
{
    if (instance != nullptr)
    {
        throw InitException("Cannot have more than one PointerManager");
    }
    instance = this;
}
PointerManager::~PointerManager() = default; // https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/

PointerManager::PointerManagerImpl::PointerManagerImpl()
{

#if debugPointerManager == 0
    // set plog severity to info temporarily
    auto oldSeverity = plog::get()->getMaxSeverity();
    plog::get()->setMaxSeverity(plog::info);
#endif

    // Set pointerDataLocation 
    pointerDataLocation = g_ourInitParameters->injectorPath;
    pointerDataLocation += +"\\CEERPointerData.xml";

    try
    {
        PLOG_INFO << "downloading pointerData.xml";
        downloadXML("https://raw.githubusercontent.com/Burnt-o/CEER/master/CERandomiser/CEERPointerData.xml");
    }
    catch (InitException ex)
    {
        PLOG_ERROR << "Failed to download CEER PointerData xml, trying local backup";
    }


    std::string pointerData = readLocalXML();
    std::stringstream buf; buf << getCurrentMCCVersion();
    this->currentGameVersion = buf.str();
    this->currentProcessType = getCurrentMCCType();
    PLOG_INFO << "MCC Version: " << currentGameVersion;

    parseXML(pointerData);
    PLOG_DEBUG << "pointer parsing complete";
    for (const auto& [key, value] : mMultilevelPointerData)
    {
        PLOG_DEBUG << std::format("stored key: {}", key);
    }


#if debugPointerManager == 0
    // set severity back to normal
    plog::get()->setMaxSeverity(oldSeverity);
#endif

}



std::shared_ptr<MultilevelPointer> PointerManager::getMultilevelPointer(std::string dataName)
{
    if (!instance->impl.get()->mMultilevelPointerData.contains(dataName))
    {
        PLOG_ERROR << "no valid pointer data for " << dataName;
        throw InitException(std::format("pointerData was null for {}", dataName));
    }

    return instance->impl.get()->mMultilevelPointerData.at(dataName);

}

std::shared_ptr<MidhookContextInterpreter> PointerManager::getMidhookContextInterpreter(std::string dataName)
{
    if (!instance->impl.get()->mMidhookContextInterpreterData.contains(dataName))
    {
        PLOG_ERROR << "no valid pointer data for " << dataName;
        throw InitException(std::format("pointerData was null for {0}", dataName));
    }

    return instance->impl.get()->mMidhookContextInterpreterData.at(dataName);
}

template <>
std::vector<byte> PointerManager::getVectorImpl<byte>(std::string dataName)
{
    auto anyVec = instance->impl.get()->mVectorData.at(dataName);
    PLOG_VERBOSE << "a";
    std::vector<byte> out;
    out.reserve(anyVec.size());
    PLOG_VERBOSE << "b";
    for (int i = 0; i < anyVec.size(); i++)
    {
        out.push_back(std::any_cast<byte>(anyVec.at(i)));
    }
    PLOG_VERBOSE << "c";
    return out;
}


template <>
std::vector<byte> PointerManager::getVector<byte>(std::string dataName)
{
    if (!instance->impl.get()->mVectorData.contains(dataName))
    {
        PLOG_ERROR << "no valid pointer data for " << dataName;
        throw InitException(std::format("pointerData was null for {0}", dataName));
    }

    try
    {
        return getVectorImpl<byte>(dataName);
    }
    catch (const std::bad_any_cast& e)
    {
        throw InitException(std::format("Bad typecast of vector data with name {}: {}", dataName, e.what()));
    }
}

std::map<std::string, std::shared_ptr<MultilevelPointer>> PointerManager::PointerManagerImpl::mMultilevelPointerData{};
std::map<std::string, std::shared_ptr<MidhookContextInterpreter>> PointerManager::PointerManagerImpl::mMidhookContextInterpreterData{};
std::map<std::string, std::vector<std::any>> PointerManager::PointerManagerImpl::mVectorData{};




//
//std::string ExePath() {
//    char buffer[MAX_PATH] = { 0 };
//    GetModuleFileNameA(NULL, buffer, MAX_PATH);
//    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
//    return std::string(buffer).substr(0, pos);
//}


std::string PointerManager::PointerManagerImpl::readLocalXML()
{
    std::string pathToFile;
#if CEER_DEBUG
    pathToFile = "C:\\Users\\mauri\\source\\repos\\CERandomiser\\CERandomiser\\CEERPointerData.xml";
#else
    pathToFile = pointerDataLocation;
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
    std::string pathToFile = pointerDataLocation;
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




VersionInfo PointerManager::PointerManagerImpl::getCurrentMCCVersion()
{
    VersionInfo outCurrentMCCVersion;
    HMODULE mccProcess = GetModuleHandle(NULL);
    char mccProcessPath[MAX_PATH];
    GetModuleFileNameA(mccProcess, mccProcessPath, sizeof(mccProcessPath));

    PLOG_DEBUG << "Getting file version info of mcc at: " << mccProcessPath;
    outCurrentMCCVersion = getFileVersion(mccProcessPath);

    PLOG_DEBUG << "mccVersionInfo: " << outCurrentMCCVersion;

    if (outCurrentMCCVersion.major != 1)
    {
        std::stringstream buf;
        buf << outCurrentMCCVersion;
        throw InitException(std::format("mccVersionInfo did not start with \"1.\"! Actual read version: {}", buf.str()).c_str());
    }

    return outCurrentMCCVersion;

}


MCCProcessType PointerManager::PointerManagerImpl::getCurrentMCCType()
{
    std::string outCurrentMCCType;
    HMODULE mccProcess = GetModuleHandle(NULL);
    char mccProcessPath[MAX_PATH];
    GetModuleFileNameA(mccProcess, mccProcessPath, sizeof(mccProcessPath));

    std::string mccName = mccProcessPath;
    mccName = mccName.substr(mccName.find_last_of("\\") + 1, mccName.size() - mccName.find_last_of("\\") - 1);

    // checks need to ignore letter case
    if (boost::iequals(mccName, "MCCWinStore-Win64-Shipping.exe")) 
    {
        return MCCProcessType::WinStore;
    }
    else if (boost::iequals(mccName, "MCC-Win64-Shipping.exe"))
    {
        return MCCProcessType::Steam;
    }
    else
    {
        throw InitException(std::format("MCC process had the wrong name!: {}", mccName));
    }

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
        else if (entryName == "LatestCEERVersion")
        {
            CEERVersioning::SetLatestVersion(processLatestCEERVersion(entry));
        }
        else
        {
            PLOG_ERROR << "Unexpected item in pointer data: " << entry.name();
        }

    }



}

VersionInfo PointerManager::PointerManagerImpl::processLatestCEERVersion(pugi::xml_node entry)
{
    using namespace pugi;
    VersionInfo latestVer{0,0,0,0};


    latestVer.major = entry.child("Major").text().as_int();
    latestVer.minor = entry.child("Minor").text().as_int();
    latestVer.build = entry.child("Build").text().as_int();
    latestVer.revision = entry.child("Revision").text().as_int();

    if (latestVer == VersionInfo{0, 0, 0, 0})
        throw InitException("Could not read latest version info");
    return latestVer;
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

        // check for steam vs winstore flag
        if (!versionEntry.attribute("ProcessType").empty())
        {
            PLOG_DEBUG << "Checking process type";
            if ((currentProcessType == MCCProcessType::Steam && !strcmp(versionEntry.attribute("ProcessType").value(), "Steam"))
                || (currentProcessType == MCCProcessType::WinStore && !strcmp(versionEntry.attribute("ProcessType").value(), "WinStore")))
            {
                PLOG_VERBOSE << "wrong process type";
                continue;
            }
        }

                // check for duplicates
        if (mMultilevelPointerData.contains(entryName))
        {
            throw InitException(std::format("Entry already exists in pointerData, entryName {}\n", entryName));
        }

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
        else if (entryType.starts_with("Vector"))
        {
            PLOG_DEBUG << "instantiating Vector";
            std::string typeName = entry.attribute("Typename").as_string();
            // I'll have to manually add each type I want to support here.
            if (typeName == nameof(byte))
                instantiateVectorInteger<byte>(versionEntry, entryName);
            else if (typeName == nameof(int))
                instantiateVectorInteger<int>(versionEntry, entryName);
            else if (typeName == nameof(long))
                instantiateVectorInteger<long>(versionEntry, entryName);
            else if (typeName == nameof(float))
                instantiateVectorFloat<float>(versionEntry, entryName);
            else if (typeName == nameof(double))
                instantiateVectorFloat<double>(versionEntry, entryName);
            else if (typeName == nameof(long double))
                instantiateVectorFloat<long double>(versionEntry, entryName);
            else
                throw InitException(std::format("Unsupported typename passed to instantiateVector {}: {}", entryName, typeName));

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
    std::vector<ParameterLocation> parameterRegisters;

    using namespace pugi;

    xml_node paramArray = versionEntry.first_child();
    for (xml_node parameter = paramArray.first_child(); parameter; parameter = parameter.next_sibling())
    {
        std::string parameterLocationText = parameter.text().as_string();

        auto mathSymbol = parameterLocationText.find_first_of('+');
        if (mathSymbol == std::string::npos)
        {
            mathSymbol = parameterLocationText.find_first_of('-');
        }
        
        std::string registerText = mathSymbol == std::string::npos ? parameterLocationText : parameterLocationText.substr(0, mathSymbol);
        PLOG_VERBOSE << "registerText: " << registerText;


        if (!stringToRegister.contains(registerText))
        {
            throw InitException(std::format("invalid parameter string when parsing MidhookContextInterpreter->{}: {}", entryName, parameterLocationText));
        }

        if (mathSymbol != std::string::npos)
        {
            PLOG_VERBOSE << "parsing RSP/RBP offset";
            // parsing the number from the string is a pain in the ass
            std::string offsetText = parameterLocationText.substr(mathSymbol, parameterLocationText.size());

            PLOG_VERBOSE << "offsetText " << offsetText;
            int offset = offsetText.contains("0x") ? stoi(offsetText, 0, 16) : stoi(offsetText);
            PLOG_VERBOSE << "offset " << offset;
            std::vector<int> offsets = { offset }; // TODO: rewrite this section to be capable of handling multiple levels of offsets
            parameterRegisters.push_back(ParameterLocation(stringToRegister.at(registerText), offsets));

        }
        else
        {
            parameterRegisters.push_back(ParameterLocation(stringToRegister.at(registerText)));
        }


    }

    if (parameterRegisters.empty())
    {
        throw InitException(std::format("no parameter strings found when parsing MidhookContextInterpreter {}", entryName));
    }

    result = std::make_shared<MidhookContextInterpreter>(parameterRegisters);

    PLOG_DEBUG << "MidhookContextInterpreter added to map: " << entryName;
    mMidhookContextInterpreterData.try_emplace(entryName, result);
   
}


template <typename T>
void PointerManager::PointerManagerImpl::instantiateVectorInteger(pugi::xml_node versionEntry, std::string entryName)
{
    std::vector<std::any> out;

    std::string tmp;
    std::string s = versionEntry.first_child().text().as_string();
    PLOG_INFO << "instantiateVectorNumber: " << s;
    std::stringstream ss(s);


    while (std::getline(ss, tmp, ','))
    {
        // Hexadecimal conversion
        auto number = tmp.contains("0x") ? stoll(tmp, 0, 16) : stoll(tmp);
        try
        {
            out.push_back((T)number);

        }
        catch (const std::bad_cast& e)
        {
            throw InitException(std::format("Could not convert number to typename for entry {}: {}", entryName, e.what()));
        }
    }

    
    mVectorData.try_emplace(entryName, out);

   

}


template <typename T>
void PointerManager::PointerManagerImpl::instantiateVectorFloat(pugi::xml_node versionEntry, std::string entryName)
{
    std::vector < std::any > out;

    std::string tmp;
    std::string s = versionEntry.first_child().text().as_string();
    PLOG_INFO << "instantiateVectorNumber: " << s;
    std::stringstream ss(s);

  
    while (std::getline(ss, tmp, ','))
    {
        // string to long double which we dynamic_cast down to T
        auto number = stold(tmp);
        try
        {

            out.push_back((T)number);
        
        }
        catch (const std::bad_cast& e)
        {
            throw InitException(std::format("Could not convert number to typename for entry {}: {}", entryName, e.what()));
        }
    }
   
   

    mVectorData.try_emplace(entryName, out);



}

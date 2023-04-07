#include "pch.h"
#include "PointerManager.h"
#include <winver.h>
#define useLocalPointerData 1

PointerManager::PointerManager()
{

    std::string pointerData;

    
#if useLocalPointerData
    readXMLlocally("C:\\Users\\mauri\\source\\repos\\CERandomiser\\CERandomiser\\CEERPointerData.xml", pointerData);
#else
    // Just set to HCM's pointerdata file for the moment, for testing
    PLOG_INFO << "downloading pointerData.xml";
    downloadXML("https://raw.githubusercontent.com/Burnt-o/HaloCheckpointManager/HCM2/HCM3/PointerData.xml", pointerData);
#endif
   
    std::string currentMCCVersion;
    getCurrentMCCVersion(currentMCCVersion);

    PLOG_INFO << "MCC Version: " << currentMCCVersion;

}


void PointerManager::readXMLlocally(std::string pathToFile, std::string& outXML)
{
    std::ifstream inFile(pathToFile.c_str());
    if (inFile.is_open())
    {
        inFile >> outXML;
        inFile.close();
        PLOG_INFO << "local XML read succesfully!";
        return;
    }
    else
    {
        std::stringstream er;
        er << "Failed to open file : " << GetLastError() << std::endl << "at: " << pathToFile;
        throw expected_exception(er.str().c_str());
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

void PointerManager::downloadXML(std::string url, std::string& outXML)
{

    if (auto curl = curl_uptr(curl_easy_init()))
    {
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &outXML);

        CURLcode ec;
        if ((ec = curl_easy_perform(curl.get())) != CURLE_OK)
            throw expected_exception(curl_easy_strerror(ec));

    }

}



std::string getFileVersion(const char* filename)
{
    DWORD dwHandle, size = GetFileVersionInfoSizeA(filename, &dwHandle);
    if (size == 0)
    {
        throw expected_exception(std::format("fileInfoVersionSize was zero, error: {}", GetLastError()));

    }

    std::vector<char> buffer;
    buffer.reserve(size);


    if (!GetFileVersionInfoA(filename, dwHandle, size, buffer.data()))
    {
        throw expected_exception(std::format("GetFileVersionInfoA failed, error: {}", GetLastError()));
    }

    VS_FIXEDFILEINFO* pvi;
    size = sizeof(VS_FIXEDFILEINFO);
    if (!VerQueryValueA(buffer.data(), "\\", (LPVOID*)&pvi, (unsigned int*)&size))
    {
        throw expected_exception(std::format("VerQueryValueA failed, error: {}", GetLastError()));
    }
    std::string outVersionInfo = std::format("{}.{}.{}.{}",
         pvi->dwProductVersionMS >> 16,
         pvi->dwFileVersionMS & 0xFFFF,
         pvi->dwFileVersionLS >> 16,
         pvi->dwFileVersionLS & 0xFFFF
    );

    return outVersionInfo;

}

void PointerManager::getCurrentMCCVersion(std::string& outCurrentMCCVersion)
{
   
    HMODULE mccProcess = GetModuleHandle(NULL);
    char mccProcessPath[MAX_PATH];
    GetModuleFileNameA(mccProcess, mccProcessPath, sizeof(mccProcessPath));

    outCurrentMCCVersion = getFileVersion(mccProcessPath);

    PLOG_DEBUG << "mccVersionInfo: " << outCurrentMCCVersion;
    PLOG_DEBUG << "size: " << outCurrentMCCVersion.size();
    if (outCurrentMCCVersion.size() != 10)
    {
        throw expected_exception("mccVersionInfo was incorrect size!");
    }
    if (!outCurrentMCCVersion.starts_with("1."))
    {
        throw expected_exception("mccVersionInfo did not start with \"1.\"!");
    }
    
}

void PointerManager::filterXMLtoCurrentVersion(std::string currentMCCVersion, std::string& outXML)
{

}

void PointerManager::instantiateMultilevelPointers(std::string xml)
{

}

void PointerManager::instantiateMidhookContextInterpreters(std::string xml)
{

}





bool PointerManager::getMultilevelPointer(std::string dataName)
{
    return true;
}

bool PointerManager::getMidhookContextInterpreter(std::string dataName)
{
    return true;
}
#pragma once
#include <pugixml.hpp>
namespace OptionSerialisation
{
	extern pugi::xml_document serialiseAll();

	extern void deserialiseAll(pugi::xml_document& doc);
	extern void deserialiseAll(std::string in);

	extern void serialiseToFile();
	extern void deserialiseFromFile();

	extern std::string compressSettingsString(std::string in);
	extern std::string decompressSettingsString(std::string in);
}




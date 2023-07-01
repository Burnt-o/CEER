#pragma once
#include <pugixml.hpp>
namespace OptionSerialisation
{

	extern pugi::xml_document serialiseAllOptions(bool clipboardOnlySettings = false);
	extern void deserialiseAllOptions(pugi::xml_document& doc, bool clipboardOnly);

	extern void serialiseToFile();
	extern void deserialiseFromFile();

}




#include "pch.h"
#include "UnitInfo.h";

std::string getShortNameFromFull(std::string fullName)
{
	auto pos = fullName.find_last_of("\\");
	if (pos == std::string::npos) return fullName;
	std::string out = fullName.substr(pos + 1);
	return out;
}
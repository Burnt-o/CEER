#include "pch.h"
#include "TagInfoBase.h"


std::string TagInfoBase::getShortNameFromFull(std::string fullName)
{
	auto pos = fullName.find_last_of("\\");
	if (pos == std::string::npos) return fullName;
	std::string out = fullName.substr(pos + 1);
	return out;
}
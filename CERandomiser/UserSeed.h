#pragma once
#include "OptionsState.h"

class UserSeed
{
private:
	static uint64_t hashString(std::string str);

	static std::string generateString();

public:
	static uint64_t GetCurrentSeed()
	{
		// Generate a string if it's blank
		if(OptionsState::SeedString.GetValue().find_first_not_of(' ') == std::string::npos)
		{
			OptionsState::SeedString.GetValue() = generateString();
			OptionsState::SeedString.UpdateValueWithInput();
		}

		return hashString(OptionsState::SeedString.GetValue());

	}
};


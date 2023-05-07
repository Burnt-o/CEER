#include "pch.h"
#include "UserSeed.h"
#include "SetSeed.h"

uint64_t UserSeed::hashString(std::string str)
{
	uint64_t hash = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		SetSeed64 gen((unsigned char)str[i] + hash);
		hash = gen();
	}
	return hash;
}





 std::string UserSeed::generateString()
{
	static const char consonants[] = "bcdfghjklmnpqrstvwyxz";
	static const char vowels[] = "aeiou";

	std::string temp;
	srand(time(NULL));
	int stringLength = (rand() % 10) + 5;
	temp.reserve(stringLength);

	for (int i = 0; i < stringLength; i++)
	{
		if (i % 2 == 0)
		{
			temp += consonants[rand() % (sizeof(consonants) - 1)];
		}
		else
		{
			temp += vowels[rand() % (sizeof(vowels) - 1)];
		}
	}
	return temp;
}
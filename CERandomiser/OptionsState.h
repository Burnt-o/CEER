#pragma once
#include "Option.h"
#include "EnemyRule.h"




namespace OptionsState
{
	extern Option<std::string> SeedString;

#pragma region gameplay modifiers
	extern Option<bool> EnemyRandomiser;
	extern Option<bool> RandomiserIncludesFlameThrowers;
	extern Option<bool> EnemySpawnMultiplier;

	extern std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules;
	extern std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules;
#pragma endregion gameplay modifiers
#pragma region texture rando
	extern Option<bool> TextureRandomiser;
	extern Option<double> TextureRandomiserPercent;
	extern Option<bool> TextureRestrictToCategory;
	extern Option<bool> TextureIncludeCharacter;
	extern Option<bool> TextureIncludeWeapVehi;
	extern Option<bool> TextureIncludeEffect;
	extern Option<bool> TextureIncludeLevel;
	extern Option<bool> TextureIncludeUI;

#pragma endregion texture rando

	extern std::vector<SerialisableOption*> allSerialisableOptions;
};


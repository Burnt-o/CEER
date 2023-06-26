#pragma once
#include "Option.h"
#include "EnemyRule.h"




namespace OptionsState
{
	extern Option<std::string> SeedString;

#pragma region gameplay modifiers
	extern Option<bool> EnemyRandomiser;
#if includeFlamethrowerFloodOption == 1
	extern Option<bool> RandomiserIncludesFlameThrowers;
#endif
	extern Option<bool> EnemySpawnMultiplier;

	extern std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules;
	extern std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules;
#pragma endregion gameplay modifiers
#pragma region TextureRandomiser
	extern Option<bool> TextureRandomiser;
	extern Option<double> TextureRandomiserPercent;
	extern Option<bool> TextureRestrictToCategory;
	extern Option<bool> TextureIncludeCharacter;
	extern Option<bool> TextureIncludeWeapVehi;
	extern Option<bool> TextureIncludeEffect;
	extern Option<bool> TextureIncludeLevel;
	extern Option<bool> TextureIncludeUI;

	extern Option<bool> TextureSeizureMode;
	extern Option<int> TextureFramesBetweenSeizures;

#pragma endregion TextureRandomiser

#pragma region SoundRandomiser
	extern Option<bool> SoundRandomiser;
	extern Option<double> SoundRandomiserPercent;
	extern Option<bool> SoundRestrictToCategory;
	extern Option<bool> SoundIncludeDialog;
	extern Option<bool> SoundIncludeMusic;
	extern Option<bool> SoundIncludeAnimations;
	extern Option<bool> SoundIncludeEffects;
	extern Option<bool> SoundIncludeWeapVehi;

#pragma endregion SoundRandomiser
	extern Option<bool> VerboseLogging;
	extern Option<bool> CheckForUpdates;


	extern std::vector<SerialisableOption*> allSerialisableOptions;
};


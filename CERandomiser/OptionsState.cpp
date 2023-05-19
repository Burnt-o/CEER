#include "pch.h"
#include "OptionsState.h"



namespace OptionsState
{

	Option<std::string> SeedString(
		"gargamel",
		[](std::string newValue)
		{
			return true;
			// TODO: should I add a check for non-alphanumeric characters? I think the xml serialisation can handle mostly any unicode char https://www.w3.org/TR/REC-xml/#charsets
		},
		nameof(SeedString)
	);

#pragma region gameplay modifiers

	Option<bool> EnemyRandomiser(
		false,
		[](bool newValue) { return true; },
		nameof(EnemyRandomiser)

	);

	Option<bool> RandomiserIncludesFlameThrowers(
		false,
		[](bool newValue) { return true; },
		nameof(RandomiserIncludesFlameThrowers)

	);

	Option<bool> EnemySpawnMultiplier(
		false,
		[](bool newValue) { return true; },
		nameof(EnemySpawnMultiplier)
	);

	// TODO: add default rules
	std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules; // starts empty
	std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules; // starts empty

#pragma endregion gameplay modifiers
#pragma region texture rando

	Option<bool> TextureRandomiser(
		false,
		[](bool newValue) { return true; },
		nameof(TextureRandomiser)

	);


	Option<double> TextureRandomiserPercent(
		100.,
		[](double newValue) { return newValue <= 100. && newValue >= 0.; },
		nameof(TextureRandomiserPercent)

	);

	Option<bool> TextureRestrictToCategory(
		true,
		[](bool newValue) { return true; },
		nameof(TextureRestrictToCategory)

	);

	Option<bool> TextureIncludeCharacter(
		true,
		[](bool newValue) { return true; },
		nameof(TextureIncludeCharacter)

	);

	Option<bool> TextureIncludeWeapVehi(
		true,
		[](bool newValue) { return true; },
		nameof(TextureIncludeWeapVehi)

	);

	Option<bool> TextureIncludeEffect(
		true,
		[](bool newValue) { return true; },
		nameof(TextureIncludeEffect)

	);
	Option<bool> TextureIncludeLevel(
		true,
		[](bool newValue) { return true; },
		nameof(TextureIncludeLevel)

	);
	Option<bool> TextureIncludeUI(
		true,
		[](bool newValue) { return true; },
		nameof(TextureIncludeUI)

	);








#pragma endregion texture rando

	std::vector<SerialisableOption*> allSerialisableOptions{ &SeedString, &EnemyRandomiser, &EnemySpawnMultiplier, &RandomiserIncludesFlameThrowers, 
		&TextureRandomiser, &TextureRandomiserPercent, &TextureRestrictToCategory, &TextureIncludeCharacter, &TextureIncludeWeapVehi, &TextureIncludeEffect, &TextureIncludeLevel, &TextureIncludeUI };
}


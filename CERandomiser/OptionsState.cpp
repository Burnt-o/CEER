#include "pch.h"
#include "OptionsState.h"



namespace OptionsState
{

	Option<std::string> SeedString(
		"gargamel",
		[](std::string newValue)
		{
			// chars to be alphanumeric
			for (char x : newValue)
			{
				if (!isalnum(x)) return false;
			}
			return true;
		},
		nameof(SeedString)
	);

#pragma region gameplay modifiers

	Option<bool> EnemyRandomiser(
		false,
		[](bool newValue) { return true; },
		nameof(EnemyRandomiser)

	);

#if includeFlamethrowerFloodOption == 1
	Option<bool> RandomiserIncludesFlameThrowers(
		false,
		[](bool newValue) { return true; },
		nameof(RandomiserIncludesFlameThrowers)

	);
#endif

	Option<bool> EnemySpawnMultiplier(
		false,
		[](bool newValue) { return true; },
		nameof(EnemySpawnMultiplier)
	);
	std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules;
	std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules;

#pragma endregion gameplay modifiers
#pragma region TextureRandomiser

	Option<bool> TextureRandomiser(
		false,
		[](bool newValue) { return true; },
		nameof(TextureRandomiser)

	);


	Option<double> TextureRandomiserPercent(
		100.,
		[](double newValue) { return newValue <= 100. && newValue > 0.; },
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

	Option<bool> TextureSeizureMode(
		false,
		[](bool newValue) { return true; },
		nameof(TextureSeizureMode)

	);
	Option<int> TextureFramesBetweenSeizures(
		10000,
		[](int newValue) { return newValue >= 1; },
		nameof(TextureFramesBetweenSeizures)

	);

#pragma endregion TextureRandomiser

#pragma region SoundRandomiser
	Option<bool> SoundRandomiser(
		false,
		[](bool newValue) { return true; },
		nameof(SoundRandomiser)

	);


	Option<double> SoundRandomiserPercent(
		100.,
		[](double newValue) { return newValue <= 100. && newValue > 0.; },
		nameof(SoundRandomiserPercent)

	);

	Option<bool> SoundRestrictToCategory(
		true,
		[](bool newValue) { return true; },
		nameof(SoundRestrictToCategory)

	);

	Option<bool> SoundIncludeDialog(
		true,
		[](bool newValue) { return true; },
		nameof(SoundIncludeDialog)

	);

	Option<bool> SoundIncludeMusic(
		true,
		[](bool newValue) { return true; },
		nameof(SoundIncludeMusic)

	);

	Option<bool> SoundIncludeAnimations(
		true,
		[](bool newValue) { return true; },
		nameof(SoundIncludeAnimations)

	);
	Option<bool> SoundIncludeEffects(
		true,
		[](bool newValue) { return true; },
		nameof(SoundIncludeEffects)

	);
	Option<bool> SoundIncludeWeapVehi(
		true,
		[](bool newValue) { return true; },
		nameof(SoundIncludeWeapVehi)

	);

#pragma endregion SoundRandomiser

	std::vector<SerialisableOption*> allSerialisableOptions{ &SeedString,  
#if includeFlamethrowerFloodOption == 1
		&RandomiserIncludesFlameThrowers, 
#endif
		&TextureRandomiserPercent, &TextureRestrictToCategory, &TextureIncludeCharacter, &TextureIncludeWeapVehi, &TextureIncludeEffect, &TextureIncludeLevel, &TextureIncludeUI, &TextureSeizureMode, &TextureFramesBetweenSeizures,
	 & SoundRestrictToCategory, & SoundIncludeDialog, & SoundIncludeMusic, & SoundIncludeAnimations, & SoundIncludeEffects, & SoundIncludeWeapVehi,
	
	// main toggles last as they will fire onChange events
		& EnemyRandomiser, &EnemySpawnMultiplier, &TextureRandomiser, & SoundRandomiser,
	};
}


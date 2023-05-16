#include "pch.h"
#include "OptionsState.h"



namespace OptionsState
{
	Option<std::string> SeedString(
		"gargamel",
		[](std::string newValue)
		{
			if (newValue.contains('z'))
			{
				PLOG_DEBUG << "z detected!";
				return false;
			}
			return true;
		},
		nameof(SeedString)
	);


	Option<bool> EnemyRandomiser(
		false,
		[](bool newValue) { return true; },
		nameof(EnemyRandomiser)

	);

	Option<bool> EnemySpawnMultiplier(
		false,
		[](bool newValue) { return true; },
		nameof(EnemySpawnMultiplier)
	);


	std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules; // starts empty
	std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules; // starts empty
	std::vector<SerialisableOption*> allSerialisableOptions{ &SeedString, &EnemyRandomiser, &EnemySpawnMultiplier };
}


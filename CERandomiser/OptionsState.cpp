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


	std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules; // starts empty
	std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules; // starts empty
	std::vector<SerialisableOption*> allSerialisableOptions{ &SeedString, &EnemyRandomiser, &EnemySpawnMultiplier, &RandomiserIncludesFlameThrowers };
}


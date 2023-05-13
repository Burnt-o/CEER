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
		}
	);

	Option<bool> AutoGenerateSeed(
		false,
		[](bool newValue) { return true; }
	);



	Option<bool> EnemyRandomiser(
		false,
		[](bool newValue) { return true; }
	);

	Option<bool> EnemySpawnMultiplier(
		false,
		[](bool newValue) { return true; }
	);


	std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules; // starts empty
	std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules; // starts empty
}


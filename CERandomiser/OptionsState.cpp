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


	Option<bool> MasterToggle(
		false,
		[](bool newValue) { return true; }
	);

	Option<bool> EnemyRandomiser(
		false,
		[](bool newValue) { return true; }
	);


	std::vector<std::unique_ptr<EnemyRandomiserRule>> currentRules; // starts empty
}


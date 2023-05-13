#pragma once
#include "Option.h"
#include "EnemyRule.h"




namespace OptionsState
{
	extern Option<std::string> SeedString;
	extern Option<bool> AutoGenerateSeed;


	extern Option<bool> EnemyRandomiser;
	extern Option<bool> EnemySpawnMultiplier;

	extern std::vector<std::unique_ptr<EnemyRule>> currentRandomiserRules;
	extern std::vector<std::unique_ptr<EnemyRule>> currentMultiplierRules;

};


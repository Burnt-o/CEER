#pragma once
#include "Option.h"
#include "EnemyRandomiserRule.h"




namespace OptionsState
{
	extern Option<std::string> SeedString;
	extern Option<bool> AutoGenerateSeed;

	extern Option<bool> MasterToggle;
	extern Option<bool> EnemyRandomiser;

	extern std::vector<std::unique_ptr<EnemyRandomiserRule>> currentRules;

};


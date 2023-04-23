#include "pch.h"
#include "OptionsState.h"


namespace OptionsState
{
	Option<bool> MasterToggle(
		false,
		[](bool newValue) { return true; }
	);

	Option<bool> EnemyRandomiser(
		false,
		[](bool newValue) { return true; }
	);
}


#include "pch.h"
#include "OptionsState.h"


	//std::mutex optionsStateMutex;
namespace OptionsState
{
	Option<bool> EnemyRandomiserEnabled(
		false,
		[](bool newValue) { return true; }
	);
}


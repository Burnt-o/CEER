#include "pch.h"
#include "EnemyRandomiserGroup.h"

namespace builtInGroups
{
	const EnemyRandomiserGroup everything("Everything", "All NPC's (and the player where applicable)", [](std::string_view checkEnemy) { return true; });
	const EnemyRandomiserGroup hunters("Hunters", "Hunters", [](std::string_view checkEnemy) { return checkEnemy.contains("hunter"); });

	const std::vector<EnemyRandomiserGroup> builtInGroups{ everything, hunters };
}

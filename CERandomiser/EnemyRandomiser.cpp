#include "pch.h"
#include "EnemyRandomiser.h"

EnemyRandomiser* EnemyRandomiser::instance = nullptr;

void EnemyRandomiser::onEnemyRandomiserEnabledChanged(bool& newValue, bool& oldValue)
{
	if (newValue && !oldValue)
	{
		PLOG_INFO << "cummies";
		instance->needToLoadGameData = true;
	}
}

void EnemyRandomiser::onLevelLoadEvent(HaloLevel newLevel)
{
	PLOG_INFO << "amogus";
	instance->needToLoadGameData = true;
}
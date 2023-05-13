#include "pch.h"
#include "LevelLoadHook.h"


LevelLoadHook* LevelLoadHook::instance = nullptr;
 
bool LevelLoadHook::isLevelAlreadyLoaded(HaloLevel& outCurrentLevel)
{
	uint32_t scenDatum = 0;
	uint16_t stateIndicator;

	// if the stateIndicator doesn't resolve then we have a serious issue
	if (!instance->stateIndicator.get()->readData(&stateIndicator))
	{
		return false;
	}

	// check we're not in loading screen
	if (stateIndicator == 44) return false;

	// if the currentCacheAddress succesfully resolves and scenDatum isn't 0 then a level is loaded
	if (instance->currentCacheAddress.get()->readData(&scenDatum) && scenDatum != 0)
	{
		outCurrentLevel = instance->getCurrentLevel();
		return true;
	}
	return false;
}
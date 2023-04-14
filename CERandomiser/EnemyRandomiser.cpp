#include "pch.h"
#include "EnemyRandomiser.h"

EnemyRandomiser* EnemyRandomiser::instance = nullptr;

void EnemyRandomiser::onMasterToggleChanged(bool& newValue, bool& oldValue)
{
	if (newValue && !oldValue)
	{
		PLOG_INFO << "cummies";
		instance->needToLoadGameData = true;

		instance->actvSpawnHook.get()->setWantsToBeAttached(newValue);
		instance->encounterSpawnHook.get()->setWantsToBeAttached(newValue);
	}
}

void EnemyRandomiser::onLevelLoadEvent(HaloLevel newLevel)
{
	PLOG_INFO << "amogus";
	instance->needToLoadGameData = true;
}




void EnemyRandomiser::actvSpawnHookFunction(SafetyHookContext& ctx)
{
	enum 
	{
		actvIndex,
		encounterIndex,
		squadIndex,
		unitIndex
	};

	auto* ctxInterpreter = instance->actvSpawnFunctionContext.get();

	if (instance->needToLoadGameData)
	{
		if (!loadGameData())
			return;
	}

	auto* actvIndexRef = ctxInterpreter->getParameterRef(ctx, actvIndex);	
	PLOG_DEBUG << "actv spawning, index: " << *actvIndexRef;

	// Change it to hunter for testing
	* actvIndexRef = 5;

}

// This function unrandomises spawn position RNG
// It runs once per squad spawn and sets the spawn-position-rng value to a mix of encIndex, sqdIndex, and our seed
void EnemyRandomiser::encounterSpawnHookFunction(SafetyHookContext& ctx)
{
	enum 
	{
		encounterIndex,
		squadIndex
	};
	auto* ctxInterpreter = instance->encounterSpawnFunctionContext.get();


	if (instance->needToLoadGameData)
	{
		if (!loadGameData())
			return;
	}
	auto* encounterIndexRef = ctxInterpreter->getParameterRef(ctx, encounterIndex);
	auto* squadIndexRef = ctxInterpreter->getParameterRef(ctx, squadIndex);
	srand(instance->ourSeed + *encounterIndexRef + (*squadIndexRef * 100));
	*instance->spawnPositionRNGResolved = rand() % 0x100;

}

bool EnemyRandomiser::loadGameData()
{
	PLOG_DEBUG << "here's where we would load the game data, enemies etc";
	if (!instance->spawnPositionRNG.get()->resolve((void**)&instance->spawnPositionRNGResolved))
	{
		PLOG_ERROR << "could not resolve spawnPositionRNG";
		return false;
	}
	

	instance->needToLoadGameData = false;
	return true;
}
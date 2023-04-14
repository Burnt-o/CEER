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
		actvIndex
	};

	auto* ctxInterpreter = instance->actvSpawnFunctionContext.get();

	if (instance->needToLoadGameData)
	{
		loadGameData();
	}

	auto* actvIndexRef = ctxInterpreter->getParameterRef(ctx, actvIndex);	
	PLOG_DEBUG << "actv spawning, index: " << *actvIndexRef;

	// Change it to hunter for testing
	* actvIndexRef = 5;

}

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
		loadGameData();
	}

	PLOG_DEBUG << "encounter spawning, enc: " << *ctxInterpreter->getParameterRef(ctx, encounterIndex) 
		<< ", sqd: " << *ctxInterpreter->getParameterRef(ctx, squadIndex);
}

void EnemyRandomiser::loadGameData()
{
	PLOG_DEBUG << "here's where we would load the game data, enemies etc";

	instance->needToLoadGameData = false;
}
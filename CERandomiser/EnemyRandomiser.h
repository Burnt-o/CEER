#pragma once
#include "HaloEnums.h"
#include "ModuleHook.h"
#include "PointerManager.h"

class EnemyRandomiser
{
private:
	static EnemyRandomiser* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing

	// handle to our callback of OptionsState::MasterToggle so we can remove it in destructor
	eventpp::CallbackList<void(bool&, bool&)>::Handle mMasterToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&, bool&)>& mMasterToggleEvent;
	// handle to our callback of LevelLoadHook so we can remove it in destructor
	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	// What we run when masterToggle changes
	static void onMasterToggleChanged(bool& newValue, bool& oldValue);

	// What we run when new level is loaded changes
	static void onLevelLoadEvent(HaloLevel newLevel);

	// Used to know if we need to read thru the enemy data and setup the probability table
	// Is set true by onEnemyRandomiserEnabledChanged w/ true value 
		// and by levelLoadCallback
	// Hooks check this and run init if true
	bool needToLoadGameData = false;


	// Hooks
	std::shared_ptr<ModuleMidHook> actvSpawnHook;
	std::shared_ptr<ModuleMidHook> encounterSpawnHook;

	// Hook Functions
	static void actvSpawnHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> actvSpawnFunctionContext;

	static void encounterSpawnHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> encounterSpawnFunctionContext;

	// Game Data
	static void loadGameData();
	//TODO


public:
	explicit EnemyRandomiser(eventpp::CallbackList<void(bool&, bool&)>& enabledEvent, eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent)
		: mMasterToggleEvent(enabledEvent), mLevelLoadEvent(levelLoadEvent)
	{
		if (instance != nullptr)
		{
			throw ExpectedException("Cannot have more than one EnemyRandomiser");
		}
		instance = this;

		// Listen to the events we care about
		mMasterToggleCallbackHandle = enabledEvent.append(&onMasterToggleChanged);
		mLevelLoadCallbackHandle = levelLoadEvent.append(&onLevelLoadEvent);

		// Set up our hooks
		try
		{
			auto actvSpawnFunction = PointerManager::getMultilevelPointer("actvSpawnFunction");
			actvSpawnFunctionContext = PointerManager::getMidhookContextInterpreter("actvSpawnFunctionContext");

			auto encounterSpawnFunction = PointerManager::getMultilevelPointer("encounterSpawnFunction");
			encounterSpawnFunctionContext = PointerManager::getMidhookContextInterpreter("encounterSpawnFunctionContext");

			actvSpawnHook = ModuleMidHook::make(L"halo1.dll", actvSpawnFunction, actvSpawnHookFunction, false);
			encounterSpawnHook = ModuleMidHook::make(L"halo1.dll", encounterSpawnFunction, encounterSpawnHookFunction, false);
		}
		catch (ExpectedException& ex)
		{
			ex.prepend("EnemyRandomisercould not resolve hooks: ");
			throw ex;
		}


	}

	~EnemyRandomiser()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		// Unsubscribe events
		mMasterToggleEvent.remove(mMasterToggleCallbackHandle);
		mLevelLoadEvent.remove(mLevelLoadCallbackHandle);

		//TODO: destroy hooks

		instance = nullptr;
	}

};


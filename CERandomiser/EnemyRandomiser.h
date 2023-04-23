#pragma once
#include "HaloEnums.h"
#include "ModuleHook.h"
#include "PointerManager.h"
#include "MapReader.h"
#include "SetSeed.h"




struct UnitInfo {
	std::string fullName;
	std::string shortName;

	double probabilityOfRandomize = 1.0;
	double probabilityOfRoll = 1.0;
	int thingIndex;
	faction defaultTeam;
};




class EnemyRandomiser
{
private:
	static EnemyRandomiser* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing


	// ref to MapReader for reading map stuff like actorPalettes and etc
	MapReader* mapReader;

	// handle to our callback of OptionsState::MasterToggle so we can remove it in destructor
	eventpp::CallbackList<void(bool&)>::Handle mMasterToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mMasterToggleEvent;
	// handle to our callback of LevelLoadHook so we can remove it in destructor
	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	// What we run when masterToggle changes
	static void onMasterToggleChanged(bool& newValue);

	// What we run when new level is loaded changes
	static void onLevelLoadEvent(HaloLevel newLevel);



	// Pointers
	std::shared_ptr<MultilevelPointer> spawnPositionRNG;

	// Hooks
	std::shared_ptr<ModuleMidHook> actvSpawnHook;
	static void actvSpawnHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> actvSpawnFunctionContext;

	std::shared_ptr<ModuleMidHook> placeObjectHook;
	static void placeObjectHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> placeObjectFunctionContext;

	std::shared_ptr<ModuleMidHook> encounterSpawnHook;
	static void encounterSpawnHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> encounterSpawnFunctionContext;

	std::shared_ptr<ModuleMidHook> fixUnitFactionHook;
	static void fixUnitFactionHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> fixUnitFactionFunctionContext;

	std::shared_ptr<ModuleMidHook> vehicleExitHook;
	static void vehicleExitHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> vehicleExitFunctionContext;

	std::shared_ptr<ModuleMidHook> aiGoToVehicleHook;
	static void aiGoToVehicleHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> aiGoToVehicleFunctionContext;

	std::shared_ptr<ModuleMidHook> aiLoadInVehicleHook;
	static void aiLoadInVehicleHookFunction(SafetyHookContext& ctx);




	// Game Data
	uint64_t ourSeed = 0x12355678;
	uint32_t* spawnPositionRNGResolved = nullptr;
	//TODO


	UnitInfo readActorInfo(actorTagReference* actor);
	UnitInfo readBipedInfo(bipedTagReference* biped);

	void evaluateActors(actorPaletteWrapper actorPalette); 
	void evaluateBipeds(bipedPaletteWrapper bipedPalette); 
	

	std::discrete_distribution<int> actorIndexDistribution;
	std::discrete_distribution<int> bipedIndexDistribution;
	std::uniform_real_distribution<float> zeroToOne{ 0.f, 1.f };

	std::map<int, UnitInfo> actorMap;
	std::map<int, UnitInfo> bipedMap;

	faction lastSpawnedUnitsFaction = faction::Undefined;



public:
	explicit EnemyRandomiser(eventpp::CallbackList<void(bool&)>& enabledEvent, eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent, MapReader* mapR)
		: mMasterToggleEvent(enabledEvent), mLevelLoadEvent(levelLoadEvent), mapReader(mapR)
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one EnemyRandomiser");
		}
		instance = this;

		// Listen to the events we care about
		mMasterToggleCallbackHandle = enabledEvent.append(&onMasterToggleChanged);
		mLevelLoadCallbackHandle = levelLoadEvent.append(&onLevelLoadEvent);

		// Set up our hooks and get our pointers
		try
		{
			spawnPositionRNG = PointerManager::getMultilevelPointer("spawnPositionRNG");

			auto actvSpawnFunction = PointerManager::getMultilevelPointer("actvSpawnFunction");
			actvSpawnFunctionContext = PointerManager::getMidhookContextInterpreter("actvSpawnFunctionContext");

			auto placeObjectFunction = PointerManager::getMultilevelPointer("placeObjectFunction");
			placeObjectFunctionContext = PointerManager::getMidhookContextInterpreter("placeObjectFunctionContext");

			auto encounterSpawnFunction = PointerManager::getMultilevelPointer("encounterSpawnFunction");
			encounterSpawnFunctionContext = PointerManager::getMidhookContextInterpreter("encounterSpawnFunctionContext");

			auto fixUnitFactionFunction = PointerManager::getMultilevelPointer("fixUnitFactionFunction");
			fixUnitFactionFunctionContext = PointerManager::getMidhookContextInterpreter("fixUnitFactionFunctionContext");

			auto vehicleExitFunction = PointerManager::getMultilevelPointer("vehicleExitFunction");
			vehicleExitFunctionContext = PointerManager::getMidhookContextInterpreter("vehicleExitFunctionContext");

			auto aiGoToVehicleFunction = PointerManager::getMultilevelPointer("aiGoToVehicleFunction");
			aiGoToVehicleFunctionContext = PointerManager::getMidhookContextInterpreter("aiGoToVehicleFunctionContext");

			auto aiLoadInVehicleFunction = PointerManager::getMultilevelPointer("aiLoadInVehicleFunction");

			actvSpawnHook = ModuleMidHook::make(L"halo1.dll", actvSpawnFunction, actvSpawnHookFunction, false);
			placeObjectHook = ModuleMidHook::make(L"halo1.dll", placeObjectFunction, placeObjectHookFunction, false);
			encounterSpawnHook = ModuleMidHook::make(L"halo1.dll", encounterSpawnFunction, encounterSpawnHookFunction, false);
			fixUnitFactionHook = ModuleMidHook::make(L"halo1.dll", fixUnitFactionFunction, fixUnitFactionHookFunction, false);
			vehicleExitHook = ModuleMidHook::make(L"halo1.dll", vehicleExitFunction, vehicleExitHookFunction, false);
			aiGoToVehicleHook = ModuleMidHook::make(L"halo1.dll", aiGoToVehicleFunction, aiGoToVehicleHookFunction, false);
			aiLoadInVehicleHook = ModuleMidHook::make(L"halo1.dll", aiLoadInVehicleFunction, aiLoadInVehicleHookFunction, false);
		}
		catch (InitException& ex)
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


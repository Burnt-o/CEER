#pragma once
#include "HaloEnums.h"
#include "ModuleHook.h"
#include "PointerManager.h"
#include "MapReader.h"
#include "SetSeed.h"
#include "UnitInfo.h"


	struct spawnInfo {
	datum mDatum;
	faction mFaction;
	SetSeed64 mSeed;
	spawnInfo(datum d, faction f, SetSeed64 s) : mDatum(d), mFaction(f), mSeed(s) {}
};

struct objectData { // for bipds, vehis, scenery etc
	uint16_t paletteIndex;
	uint16_t nameIndex;
	uint16_t placementFlags;
	uint16_t desiredPermutation;
	float posX;
	float posY;
	float pozZ;
	float rotX;
	float rotY;
	float rotZ;
	// then a bunch of stuff I don't care about (depends on the object type, bipd vs vehi vs scen etc)
	// length of the struct varies by object type
};



extern "C" typedef __int64 __stdcall placeObjectFunction(tagBlock* paletteTable, objectData* spawningObject);


extern "C" typedef bool __stdcall ProcessSquadUnitFunction(uint16_t encounterIndex, __int16 squadIndex, __int16 unknown);

// The definitions for this class are divided into 
	// EnemyRandomiserLevelLoad.cpp
	// EnemyRandomiserHooks.cpp

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
	std::shared_ptr<MultilevelPointer> gameSpawnRNG;
	std::shared_ptr<MultilevelPointer> gameRNG;

	// Hooks




	std::shared_ptr<ModuleMidHook> fixMajorUpgradeHook;
	static void fixMajorUpgradeHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> fixMajorUpgradeFunctionContext;

	std::shared_ptr<ModuleMidHook> vehicleExitHook;
	static void vehicleExitHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> vehicleExitFunctionContext;

	std::shared_ptr<ModuleMidHook> aiGoToVehicleHook;
	static void aiGoToVehicleHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> aiGoToVehicleFunctionContext;

	std::shared_ptr<ModuleMidHook> aiLoadInVehicleHook;
	static void aiLoadInVehicleHookFunction(SafetyHookContext& ctx);
	// doesn't have interpreter, just modifies r flags


	std::shared_ptr<ModuleMidHook> fixUnitFactionHook;
	static void fixUnitFactionHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> fixUnitFactionFunctionContext;


	std::shared_ptr<ModuleMidHook> setActorDatumHook;
	static void setActorDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> setActorDatumFunctionContext;

	static placeObjectFunction newPlaceObjectFunction;
	std::shared_ptr<ModuleInlineHook> placeObjectHook;

	static ProcessSquadUnitFunction newProcessSquadUnitFunction;
	std::shared_ptr<ModuleInlineHook> processSquadUnitHook;

	std::shared_ptr<ModuleMidHook> getSquadUnitIndexHook;
	static void getSquadUnitIndexHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> getSquadUnitIndexFunctionContext;
	
	std::shared_ptr<ModuleMidHook> spawnPositionFuzzHook;
	static void spawnPositionFuzzHookFunction(SafetyHookContext& ctx);
	//std::shared_ptr<MidhookContextInterpreter> spawnPositionFuzzFunctionContext;


	// data passed between hooks
	static faction hookData_currentUnitsFaction;
	static datum hookData_currentUnitDatum;
	static int hookData_currentSquadUnitIndex;
	static bool hookData_fixSentinelPosition;
	static uint64_t hookData_currentUnitSeed;





	//static std::vector<spawnInfo> hookData_squadUnits;




#pragma region OnLevelLoadData

	// Game Data
	uint64_t ourSeed = 0x12355678;
	uint32_t* spawnPositionRNGResolved = nullptr;
	//TODO


	UnitInfo readActorInfo(const datum actorDatum);
	UnitInfo readBipedInfo(const datum bipedDatum);

	void evaluateActors(); 
	void evaluateBipeds(); 
	


	std::vector<datum> actorDatumVector; // needed for sampling UnitInfos discrete_distribution
	std::map<datum, UnitInfo> actorMap;

	std::vector<datum> bipedDatumVector; // needed for sampling UnitInfos discrete_distribution
	std::map<datum, UnitInfo> bipedMap;


#pragma endregion OnLevelLoadData






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
			gameSpawnRNG = PointerManager::getMultilevelPointer("gameSpawnRNG");
			gameRNG = PointerManager::getMultilevelPointer("gameRNG");















			
			//fixes
			auto vehicleExitFunction = PointerManager::getMultilevelPointer("vehicleExitFunction");
			vehicleExitFunctionContext = PointerManager::getMidhookContextInterpreter("vehicleExitFunctionContext");
			vehicleExitHook = ModuleMidHook::make(L"halo1.dll", vehicleExitFunction, vehicleExitHookFunction, false);

			auto aiGoToVehicleFunction = PointerManager::getMultilevelPointer("aiGoToVehicleFunction");
			aiGoToVehicleFunctionContext = PointerManager::getMidhookContextInterpreter("aiGoToVehicleFunctionContext");
			aiGoToVehicleHook = ModuleMidHook::make(L"halo1.dll", aiGoToVehicleFunction, aiGoToVehicleHookFunction, false);

			auto aiLoadInVehicleFunction = PointerManager::getMultilevelPointer("aiLoadInVehicleFunction");
			aiLoadInVehicleHook = ModuleMidHook::make(L"halo1.dll", aiLoadInVehicleFunction, aiLoadInVehicleHookFunction, false);
			
			// bipeds
			auto placeObjectFunction = PointerManager::getMultilevelPointer("placeObjectFunction");
			placeObjectHook = ModuleInlineHook::make(L"halo1.dll", placeObjectFunction, newPlaceObjectFunction, false);

			// actors
			auto getSquadUnitIndexFunction = PointerManager::getMultilevelPointer("getSquadUnitIndexFunction");
			getSquadUnitIndexFunctionContext = PointerManager::getMidhookContextInterpreter("getSquadUnitIndexFunctionContext");
			getSquadUnitIndexHook = ModuleMidHook::make(L"halo1.dll", getSquadUnitIndexFunction, getSquadUnitIndexHookFunction, false);

			auto processSquadUnitFunction = PointerManager::getMultilevelPointer("processSquadUnitFunction");
			processSquadUnitHook = ModuleInlineHook::make(L"halo1.dll", processSquadUnitFunction, newProcessSquadUnitFunction, false);

			auto setActorDatumFunction = PointerManager::getMultilevelPointer("setActorDatumFunction");
			setActorDatumFunctionContext = PointerManager::getMidhookContextInterpreter("setActorDatumFunctionContext");
			setActorDatumHook = ModuleMidHook::make(L"halo1.dll", setActorDatumFunction, setActorDatumHookFunction, false);

			auto fixUnitFactionFunction = PointerManager::getMultilevelPointer("fixUnitFactionFunction");
			fixUnitFactionFunctionContext = PointerManager::getMidhookContextInterpreter("fixUnitFactionFunctionContext");
			fixUnitFactionHook = ModuleMidHook::make(L"halo1.dll", fixUnitFactionFunction, fixUnitFactionHookFunction, false);

			auto fixMajorUpgradeFunction = PointerManager::getMultilevelPointer("fixMajorUpgradeFunction");
			fixMajorUpgradeFunctionContext = PointerManager::getMidhookContextInterpreter("fixMajorUpgradeFunctionContext");
			fixMajorUpgradeHook = ModuleMidHook::make(L"halo1.dll", fixMajorUpgradeFunction, fixMajorUpgradeHookFunction, false);

			auto spawnPositionFuzzFunction = PointerManager::getMultilevelPointer("spawnPositionFuzzFunction");
			//spawnPositionFuzzFunctionContext = PointerManager::getMidhookContextInterpreter("spawnPositionFuzzFunctionContext");
			spawnPositionFuzzHook = ModuleMidHook::make(L"halo1.dll", spawnPositionFuzzFunction, spawnPositionFuzzHookFunction, false);



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

	static void debug()
	{
		instance->evaluateActors();
	}

};


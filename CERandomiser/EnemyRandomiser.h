#pragma once
#include "HaloEnums.h"
#include "ModuleHook.h"
#include "PointerManager.h"
#include "MapReader.h"
#include "SetSeed.h"
#include "UnitInfo.h"
#include "OptionsState.h"




	struct spawnInfo {
	datum mDatum;
	faction mFaction;
	SetSeed64 mSeed;
	spawnInfo(datum d, faction f, SetSeed64 s) : mDatum(d), mFaction(f), mSeed(s) {}
};



struct objectData { // for bipds, vehis, scenery etc
	int16_t paletteIndex;
	int16_t nameIndex;
	uint16_t placementFlags;
	uint16_t desiredPermutation;
	float posX;
	float posY;
	float pozZ;
	float rotX;
	float rotY;
	float rotZ;
	// The following information is specific to bipeds
	uint16_t unknown;
	uint8_t appearencePlayerIndex;
	float bodyVitalityPercent;
	bool spawnsDead;
};



extern "C" typedef __int64 __stdcall placeObjectFunction(objectData* spawningObject, tagBlock* paletteTable);

extern "C" typedef bool __stdcall ProcessSquadUnitFunction(uint16_t encounterIndex, __int16 squadIndex);

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

	// handle to our callback of OptionsState::EnemyRandomiser so we can remove it in destructor
	eventpp::CallbackList<void(bool&)>::Handle mEnemyRandomiserToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mEnemyRandomiserToggleEvent;
	// handle to our callback of OptionsState::EnemySpawnMultiplier so we can remove it in destructor
	eventpp::CallbackList<void(bool&)>::Handle mEnemySpawnMultiplierToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mEnemySpawnMultiplierToggleEvent;
	// handle to our callback of LevelLoadHook so we can remove it in destructor
	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	// What we run when masterToggle changes
	static void onEnemyRandomiserToggleChange(bool& newValue);
	static void onEnemySpawnMultiplierToggleChange(bool& newValue);
	void onEitherOptionChange();


	// What we run when new level is loaded changes
	// can also be invoked by options being enabled when a level is already loaded
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

#if bipedRandomisation == 1
	std::shared_ptr<ModuleMidHook> setBipedDatumHook;
	static void setBipedDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> setBipedDatumFunctionContext;

	static placeObjectFunction newPlaceObjectFunction;
	std::shared_ptr<ModuleInlineHook> placeObjectHook;

	std::shared_ptr<ModulePatch> fixWinstoreBipedCrashPatch;
#endif
	static ProcessSquadUnitFunction newProcessSquadUnitFunction;
	std::shared_ptr<ModuleInlineHook> processSquadUnitHook;




	
	std::shared_ptr<ModuleMidHook> spawnPositionFuzzHook;
	static void spawnPositionFuzzHookFunction(SafetyHookContext& ctx);
     std::shared_ptr<MidhookContextInterpreter> spawnPositionFuzzFunctionContext;


	// data passed between hooks
	static faction hookData_currentUnitsFaction;
	static datum hookData_currentUnitDatum;
	static int hookData_currentSquadUnitIndex;
	static int hookData_lastUnitSpawnPositionIndex;
	static bool hookData_fixSentinelPosition;
	static uint64_t hookData_currentUnitSeed;
	static bool hookData_unitRandomised;

	static datum hookData_currentBipedDatum;




	//static std::vector<spawnInfo> hookData_squadUnits;




#pragma region OnLevelLoadData

	// Game Data
	uint64_t ourSeed = 0x12355678;

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

	std::once_flag lazyInitOnceFlag;
	void lazyInit();



public:
	explicit EnemyRandomiser(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent, MapReader* mapR)
		: mLevelLoadEvent(levelLoadEvent), mapReader(mapR), mEnemyRandomiserToggleEvent(OptionsState::EnemyRandomiser.valueChangedEvent)
		, mEnemySpawnMultiplierToggleEvent(OptionsState::EnemySpawnMultiplier.valueChangedEvent)
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one EnemyRandomiser");
		}
		instance = this;

		// Listen to the events we care about
		mEnemyRandomiserToggleCallbackHandle = mEnemyRandomiserToggleEvent.append(&onEnemyRandomiserToggleChange);
		mEnemySpawnMultiplierToggleCallbackHandle = mEnemySpawnMultiplierToggleEvent.append(&onEnemySpawnMultiplierToggleChange);
		mLevelLoadCallbackHandle = levelLoadEvent.append(&onLevelLoadEvent);

		


	}

	~EnemyRandomiser()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		// Unsubscribe events
		mEnemyRandomiserToggleEvent.remove(mEnemyRandomiserToggleCallbackHandle);
		mEnemySpawnMultiplierToggleEvent.remove(mEnemySpawnMultiplierToggleCallbackHandle);
		mLevelLoadEvent.remove(mLevelLoadCallbackHandle);

		// destroy hooks
#define safe_destroy_hook(x) if (x.get()) x->setWantsToBeAttached(false)

		safe_destroy_hook(fixMajorUpgradeHook);
		safe_destroy_hook(vehicleExitHook);
		safe_destroy_hook(aiGoToVehicleHook);
		safe_destroy_hook(aiLoadInVehicleHook);
		safe_destroy_hook(fixUnitFactionHook);
		safe_destroy_hook(setActorDatumHook);
#if bipedRandomisation == 1
		safe_destroy_hook(placeObjectHook);
		safe_destroy_hook(setBipedDatumHook);
#endif
		safe_destroy_hook(processSquadUnitHook);
		safe_destroy_hook(spawnPositionFuzzHook);


		instance = nullptr;
	}

};


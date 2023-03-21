#pragma once
#include "pch.h"
#include "enemy_randomizer.h"
#include "midhook_context_interpreter.h"
#include "ModuleCache.h"
#include "multilevel_pointer.h"




// hook function forward declaration

void Hook_EvaluateEncounter(SafetyHookContext& ctx);
void Hook_SpawnActvByIndex(SafetyHookContext& ctx);

struct encounter_context
{
	UINT64 encounter, squad, unit;
	encounter_context(UINT64 e, UINT64 s, UINT64 u) : encounter(e), squad(s), unit(u) {};
};

namespace enemy_randomizer_state
{
	namespace // private vars
	{
		std::mutex er_state_lock;
		encounter_context enc_context(0, 0, 0);
		bool entities_loaded = false;
	}

	encounter_context GetEncounterData()
	{
		er_state_lock.lock();
		static encounter_context pass = enc_context;
		er_state_lock.unlock();
		return pass;
	}
	void SetEncounterData(UINT64 e, UINT64 s, UINT64 u)
	{
		er_state_lock.lock();
		enc_context.encounter = e;
		enc_context.squad = s;
		enc_context.unit = u;
		er_state_lock.unlock();
	}

	void IncrementEncounterUnitIndex()
	{
		er_state_lock.lock();
		enc_context.unit += 1;
		er_state_lock.unlock();
	}

	bool GetEntitiesLoaded()
	{
		er_state_lock.lock();
		static bool pass = entities_loaded;
		er_state_lock.unlock();
		return pass;
	}

	void SetEntitiesLoaded(bool value)
	{
		er_state_lock.lock();
		entities_loaded = value;
		er_state_lock.unlock();
	}

}


namespace enemy_randomizer_manager
{
	void er_set_hooks(bool value)
	{
		mh_Hook_SpawnActvByIndex->set_WantsToBeEnabled(value);
		mh_Hook_EvaluateEncounter->set_WantsToBeEnabled(value);
		if (value && ModuleCache::isModuleInCache(L"halo1.dll"))
		{
			er_load_entities();
		}

	}


	void er_load_entities()
	{
		enemy_randomizer_state::SetEntitiesLoaded(false);

		try
		{
			//TODO: load entity (actv/bipd) data so we know what our randomizer pool is
			if (false) throw expected_exception("false was true! Boy that's crazy huh");
		}
		catch (expected_exception e)
		{
			PLOG_ERROR << "er_load_entities failed: " << e.what();
			return; // leave EntitiesLoaded flag set to false
		}
		enemy_randomizer_state::SetEntitiesLoaded(true);
	}
}


//pointers
#define MCC_VER_2645
#ifdef MCC_VER_2645
MultilevelPointer* p_Hook_SpawnActvByIndex = MultilevelPointer::make(std::wstring(L"halo1.dll"), { 0xC540B5 });
MultilevelPointer* p_Hook_EvaluateEncounter = MultilevelPointer::make(std::wstring(L"halo1.dll"), { 0xC51DD7 });
#endif // MCC_VER_2645


//hooks objects
MidHook* mh_Hook_SpawnActvByIndex = new MidHook(L"Hook_SpawnActvByIndex", p_Hook_SpawnActvByIndex, Hook_SpawnActvByIndex, false, L"halo1.dll");
MidHook* mh_Hook_EvaluateEncounter = new MidHook(L"Hook_EvaluateEncounter", p_Hook_EvaluateEncounter, Hook_EvaluateEncounter, false, L"halo1.dll");


//hook functions
MidhookContextInterpreter* MCI_SpawnActvByIndex = new MidhookContextInterpreter({Register::rcx});
void Hook_SpawnActvByIndex(SafetyHookContext& ctx) {
	if (!enemy_randomizer_state::entities_loaded) { PLOG_DEBUG << "Hook_SpawnActvByIndex bailing, entities not loaded"; return; }
	uint64_t* actorVariantIndex = MCI_SpawnActvByIndex->getParameterRef(ctx, 0);

	*actorVariantIndex = 3; // set the index for 3 just for testing 

	enemy_randomizer_state::IncrementEncounterUnitIndex();
}

MidhookContextInterpreter* MCI_EvaluateEncounter = new MidhookContextInterpreter({ Register::r11, Register::rsi });
void Hook_EvaluateEncounter(SafetyHookContext& ctx) {
	if (!enemy_randomizer_state::entities_loaded) { PLOG_DEBUG << "Hook_EvaluateEncounter bailing, entities not loaded"; return; }
	uint64_t* encounterIndex = MCI_EvaluateEncounter->getParameterRef(ctx, 0);
	uint64_t* squadIndex = MCI_EvaluateEncounter->getParameterRef(ctx, 1);

	PLOG_VERBOSE << "Encounter evaluating: " << std::endl
		<< "encounter: " << *encounterIndex
		<< "squad: " << *squadIndex;

	enemy_randomizer_state::SetEncounterData(*encounterIndex, *squadIndex, 0);
	
}
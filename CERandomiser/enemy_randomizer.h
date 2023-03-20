#pragma once
#include "hook.h"
#include "pointer.h"


namespace enemy_randomizer_manager
{
	void er_set_hooks(bool value);
	void er_load_entities();
}

extern MidHook* mh_Hook_SpawnActvByIndex;
extern MidHook* mh_Hook_EvaluateEncounter;

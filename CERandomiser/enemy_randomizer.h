#pragma once
#include "hook.h"
#include "pointer.h"


namespace enemy_randomizer_manager
{
	void er_set_hooks(bool value);
	void er_load_entities();
}

extern mid_hook* mh_Hook_SpawnActvByIndex;
extern mid_hook* mh_Hook_EvaluateEncounter;

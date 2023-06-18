#include "pch.h"
#include "EnemyRandomiser.h"
#include "OptionsState.h"
#include "LevelLoadHook.h"
#include "EnemyRule.h"



std::uniform_real_distribution<double> zeroToOne{ 0.0, 1.0 };
faction EnemyRandomiser::hookData_currentUnitsFaction = faction::Undefined;
datum EnemyRandomiser::hookData_currentUnitDatum = nullDatum;
int EnemyRandomiser::hookData_currentSquadUnitIndex = 0;
bool EnemyRandomiser::hookData_fixSentinelPosition = false;
uint64_t EnemyRandomiser::hookData_currentUnitSeed = 0;
bool EnemyRandomiser::hookData_unitRandomised = false;
datum EnemyRandomiser::hookData_currentBipedDatum = nullDatum;
int EnemyRandomiser::hookData_lastUnitSpawnPositionIndex = -1;



#if bipedRandomisation == 1

constexpr std::string_view badBipedNames[] = { "nipple_grunt"};
bool isBadBipedName(std::string_view& nameView)
{
	for (auto& badName : badBipedNames)
	{
		if (nameView == badName) return true;
	}
	return false;
}


datum expectedBipedDatum = nullDatum;

// randomises bipeds (could do other stupid stuff here like randomise scenery, but ceebs)
__int64 EnemyRandomiser::newPlaceObjectFunction(objectData* spawningObject, tagBlock* paletteTableRef)
{
#define returnOriginal return instance->placeObjectHook.get()->getInlineHook().fastcall<__int64>(spawningObject, paletteTableRef)
#define throwFromNewPlaceObjectFunction(message) CEERRuntimeException ex(message); \
						RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier }); \
	returnOriginal \

	if (!instance) { PLOG_FATAL << "null instance! crash imminent"; return 0; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	PLOG_DEBUG << "BIPED";

	hookData_currentBipedDatum = nullDatum;
	auto paletteTable = (tagReference*)instance->mapReader->getTagAddress(paletteTableRef->pointer);

	constexpr auto bipdMagic = MapReader::stringToMagic("bipd");
	if (paletteTable->tagGroupMagic != bipdMagic) returnOriginal;

	// look up assigned name of this biped. There's a few we don't want to randomise, like the nipple_grunt at the end of Maw (the game will crash)
	auto objectNameIndex = spawningObject->nameIndex;
	PLOG_VERBOSE << "objectNameIndex: " << objectNameIndex;
	if (objectNameIndex >= 0)
	{
		auto nameView = instance->mapReader->getObjectName(objectNameIndex);
		PLOG_DEBUG << "biped name: " << nameView;
		if (isBadBipedName(nameView)) returnOriginal;
	}
	

	// lookup datum of currently spawning object
	paletteTable += (spawningObject->paletteIndex * 3);
	datum originalDatum = paletteTable->tagDatum;
	PLOG_VERBOSE << "original biped datum: " << originalDatum;

	expectedBipedDatum = originalDatum; // for debug

	auto& originalBipedInfo = instance->bipedMap.at(originalDatum);

	if (!originalBipedInfo.isValidUnit || spawningObject->spawnsDead) // randomising a dead biped can cause a crash (eg when rolling a popcorn or carrier that don't leave corpses)
	{
		hookData_currentBipedDatum = nullDatum;
		hookData_currentUnitsFaction = faction::Undefined;
		returnOriginal;
	}
			

		hookData_currentUnitsFaction = originalBipedInfo.defaultTeam;

		// roll randomisation
		UnitInfo* newUnit = &originalBipedInfo;// If not randomised, newUnit will actually be the original unit
		datum newUnitDatum = nullDatum;

		uint64_t seed = instance->ourSeed ^ (((uint64_t)spawningObject->posX << 32) + ((uint64_t)spawningObject->posY)); // Create a seed from the specific enemy data & XOR with the user-input seed
		SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
		PLOG_DEBUG << "seed set: " << std::hex << seed;

		PLOG_DEBUG << "probability of randomisze: " << originalBipedInfo.probabilityOfRandomize;
		if (zeroToOne(generator) < originalBipedInfo.probabilityOfRandomize) // Roll against the original units randomize probability, if true then we randomise
		{
			PLOG_DEBUG << "randomizing";
			auto unitRollIndex = originalBipedInfo.rollDistribution(generator);

			if (unitRollIndex >= instance->bipedDatumVector.size()) // safety check before we access the vector
			{
				throwFromNewPlaceObjectFunction("rolled biped index was too large!");
			}

			newUnitDatum = instance->bipedDatumVector.at(unitRollIndex); // Re-use the seed to see what new enemy we should roll into
			if (!instance->bipedMap.contains(newUnitDatum)) // safety check before we access the map
			{
				throwFromNewPlaceObjectFunction("biped spawning wasn't in our stored map!");
			}
			newUnit = &instance->bipedMap.at(newUnitDatum);
			PLOG_DEBUG << "new unit: " << newUnit->getShortName();


			hookData_currentBipedDatum = newUnitDatum;

		}


		returnOriginal;

}

int debugSetBipedDatum = 0;
void EnemyRandomiser::setBipedDatumHookFunction(SafetyHookContext& ctx)
{
	PLOG_DEBUG << "setBipedDatumHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	enum class param
	{
		BipedDatum,
	};
	auto* ctxInterpreter = instance->setBipedDatumFunctionContext.get();

	if (hookData_currentBipedDatum != nullDatum)
	{
		debugSetBipedDatum++;
		PLOG_DEBUG << "debugSetBipedDatum: " << debugSetBipedDatum;

		datum currentDatum = *(datum*)ctxInterpreter->getParameterRef(ctx, (int)param::BipedDatum);
		PLOG_DEBUG << "setBipedDatumHookFunction changing from " << currentDatum;
		PLOG_DEBUG << " to " << hookData_currentBipedDatum;

		if (currentDatum != expectedBipedDatum)
		{
			PLOG_ERROR << "WOAH WHAT THE FUCK";
			PLOG_ERROR << "expected" << expectedBipedDatum << " but got " << currentDatum;
		}

		if (debugSetBipedDatum != 1400)
		{
			*ctxInterpreter->getParameterRef(ctx, (int)param::BipedDatum) = (uint32_t)hookData_currentBipedDatum;
		}
		
		hookData_currentBipedDatum = nullDatum;
	}


}
#endif



void EnemyRandomiser::vehicleExitHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "vehicleExitHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		vehicleExitAnimationIndex
	};
	auto* ctxInterpreter = instance->vehicleExitFunctionContext.get();

	// index of 0xFFFF means the game couldn't find a valid vehicleExit animation for this unit
	if (*ctxInterpreter->getParameterRef(ctx, (int)param::vehicleExitAnimationIndex) == 0xFFFF)
	{
		// So we set it to zero ie use the first animation in the units animation list
		*ctxInterpreter->getParameterRef(ctx, (int)param::vehicleExitAnimationIndex) = 0;
	}

}

// I can't quite remember how this works, but the game script has an "ai_go_to_vehicle" command.
// If this command is run on an invalid kind of actor, the game will immediately crash.
// Luckily we can check for this condition and uhhh do something to fix it
void EnemyRandomiser::aiGoToVehicleHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "aiGoToVehicleHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		unknownCheck,
		unknownFix
	};
	auto* ctxInterpreter = instance->aiGoToVehicleFunctionContext.get();


	// We want to read the uint32 at [rcx + 0x10]
	auto rcx = *ctxInterpreter->getParameterRef(ctx, (int)param::unknownCheck);
	rcx += 0x10;
	auto checkValue = *(uint32_t*)rcx;

	// If the value is 0xFFFFFFFF, then the game will crash unless we fix something
	if (checkValue == 0xFFFFFFFF)
	{
		// the fix is setting rdi to 0xFFFF. I honestly don't remember why this works.
		*ctxInterpreter->getParameterRef(ctx, (int)param::unknownFix) = 0xFFFF;
	}


}

void EnemyRandomiser::aiLoadInVehicleHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "aiLoadInVehicleHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	// All we do here is set the zeroflag and parityflag to zero (bits 6 and 2 respectively)
	// (it'll be 1 if the ai trying to load into the vehicle isn't supposed to be in the vehicle)

	ctx.rflags = ctx.rflags & ~(1UL << 6) & ~(1UL << 2);
}

// With the multiplied enabled, NPC's that can't fit inside a dropship will instead just spawn at a default location, usually out of reach of the player.
// This causes softlocks as the player is unable to easily kill them and progress the game script.
// This hook is placed in a branch in the aiLoadInVehicle func, at a branch that's hit 
// only when all the vehicles seats are full. The hook simply kills (and hides) the enemy.
void EnemyRandomiser::killVehicleOverflowHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "killVehicleOverflowHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		pUnitObjectInstance,
	};
	auto* ctxInterpreter = instance->killVehicleOverflowFunctionContext.get();

	auto pUnitInstance = (byte*)*ctxInterpreter->getParameterRef(ctx, (int)param::pUnitObjectInstance);
	if (IsBadWritePtr(pUnitInstance, 8))
	{
		PLOG_ERROR << "Bad biped object instance pointer";
		return;
	}

	*(pUnitInstance + 0x1FB) = 1; // kill the unit
	*(pUnitInstance - 1) = 1; // hide the unit model

}




bool EnemyRandomiser::newProcessSquadUnitFunction(uint16_t encounterIndex, __int16 squadIndex)
{
#define returnOriginal return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex)
#define throwFromNewProcessSquadUnitFunction(message) CEERRuntimeException ex(message); \
						RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier }); \
	returnOriginal \

	PLOG_VERBOSE << "newProcessSquadFunction";
	if (!instance) { PLOG_FATAL << "null instance! imminent crash";  }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);


	PLOG_DEBUG << "encounterIndex: " << encounterIndex;
	PLOG_DEBUG << "squadIndex: " << squadIndex;


	datum originalActor;
	try
	{
		// Need to go to scenario tag and lookup the encounter & squad index ourselves, see what the original unit is
		originalActor = instance->mapReader->getEncounterSquadDatum(encounterIndex, squadIndex);

	}
	catch (CEERRuntimeException& ex)
	{
		RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier });
		returnOriginal;
	}

	PLOG_VERBOSE << "originalActor: " << originalActor;

	if (originalActor == nullDatum)
	{
		PLOG_VERBOSE << "bailing on nullDatum";
		hookData_currentUnitsFaction = faction::Undefined;
		hookData_currentUnitDatum = originalActor;
		returnOriginal;
	}

	PLOG_DEBUG << "original spawning actor: " << instance->mapReader->getTagName(originalActor);

	if (!instance->actorMap.contains(originalActor)) // safety check before we access the map
	{
		throwFromNewProcessSquadUnitFunction("actor spawning wasn't in our stored map!");
	}

	// get info of originally spawning actor
	UnitInfo& originalActorInfo = instance->actorMap.at(originalActor);

	if (!originalActorInfo.isValidUnit)
	{
		PLOG_DEBUG << "bailing on messing with invalid unit";
		// we shan't mess with it
		hookData_currentUnitsFaction = originalActorInfo.defaultTeam;
		hookData_currentUnitDatum = originalActor;
		returnOriginal;
	}

	// Some infinite enemy spawns won't give us a currentSquadUnitIndex. To get around this we'll precalculate the next units spawn position.
	uint16_t nextSpawnPositionIndex;
	try
	{
		 nextSpawnPositionIndex = instance->mapReader->getEncounterSquadSpawnCount(encounterIndex, squadIndex);
	}
	catch (CEERRuntimeException& ex)
	{
		RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier });
		returnOriginal;
	}

	PLOG_DEBUG << "nextSpawnPositionIndex: " << nextSpawnPositionIndex;


	// Construct a seed
	uint64_t seed = instance->ourSeed ^ (((uint64_t)encounterIndex << 32) + ((uint64_t)squadIndex << 24) + ((uint64_t)hookData_currentSquadUnitIndex << 16)) + ((uint64_t)nextSpawnPositionIndex << 8); // Create a seed from the specific enemy data & XOR with the user-input seed
	
	// clear hookdata so data doesn't bleed over
	hookData_currentSquadUnitIndex = 0;


	
	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
	PLOG_DEBUG << "seed set: " << std::hex << seed;
	PLOG_DEBUG << "spawnMultiplierPreRando: " << originalActorInfo.spawnMultiplierPreRando;
	// Apply pre-rando spawn multiplier
	double zeroToOneRoll = zeroToOne(generator);
	// A fun way to do a for loop. Use doubles!
	// This ensures that if the spawn multiplier is, say, 2.5x, we're guarenteed to spawn 2 enemies and have a 50% chance to spawn a third.

	for (double d = 0; d + zeroToOneRoll < originalActorInfo.spawnMultiplierPreRando; d++)
	{

		PLOG_DEBUG << "outer loop " << d;
		// Construct a new seed for each of these multiplied units
		seed += ((uint64_t)d << 8);
		generator = SetSeed64(seed);

		// Roll randomization for each of these units
		UnitInfo* newUnit = &originalActorInfo; // If not randomised, newUnit will actually be the original unit

		datum newUnitDatum = originalActor;
		PLOG_DEBUG << "probability of randomisze: " << originalActorInfo.probabilityOfRandomize;
		if (zeroToOne(generator) < originalActorInfo.probabilityOfRandomize) // Roll against the original units randomize probability, if true then we randomise
		{
			hookData_unitRandomised = true;

			PLOG_DEBUG << "randomizing";
			auto unitRollIndex = originalActorInfo.rollDistribution(generator);

			if (unitRollIndex >= instance->actorDatumVector.size()) // safety check before we access the vector
			{
				throwFromNewProcessSquadUnitFunction("rolled actor index was too large!");
			}

			newUnitDatum = instance->actorDatumVector.at(unitRollIndex); // Re-use the seed to see what new enemy we should roll into
			if (!instance->actorMap.contains(newUnitDatum)) // safety check before we access the map
			{
				throwFromNewProcessSquadUnitFunction("actor spawning wasn't in our stored map!");
			}
			newUnit = &instance->actorMap.at(newUnitDatum);
			PLOG_DEBUG << "new unit: " << newUnit->getShortName();
			
		}
		else
		{
			hookData_unitRandomised = false;
		}

		// Apply post-rando spawn multiplier
		double zeroToOneRollPostRando = zeroToOne(generator);
		for (double e = 0; e + zeroToOneRollPostRando < newUnit->spawnMultiplierPostRando; e++) // Important that we're checking the postRando multiplier of the NEW unit, not the original (ofc they will be the same if no randomisation occured, no biggie)
		{

			PLOG_DEBUG << "inner loop " << e;
			seed += (uint64_t)e;
			generator = SetSeed64(seed);
			// store needed data for other hooks
			hookData_currentUnitSeed = generator(); 

			if (hookData_unitRandomised)
			{
				// unrandomise spawn position / enemy randomness (nade count etc)
				// TODO:: looks like we need to do this && gameSpawnRNG even if unit not randomised
					// why don't they get the same rng w/o it ? 
					// obviously our std::rng dists are giving the same results.
				if (!instance->gameRNG.get()->writeData(&hookData_currentUnitSeed))
				{
					throwFromNewProcessSquadUnitFunction(std::format("failed to write gameRNG with {}, error {}", hookData_currentUnitSeed, MultilevelPointer::GetLastError()));
				}


				auto lowByte = (uint8_t)(hookData_currentUnitSeed);

				if (!instance->gameSpawnRNG.get()->writeData(&lowByte))
				{
					throwFromNewProcessSquadUnitFunction(std::format("failed to write gameSpawnRNG with {}, error {}", lowByte, MultilevelPointer::GetLastError()));
				}
			}


			
			hookData_currentUnitsFaction = originalActorInfo.defaultTeam; // needed for fixUnitFaction. Must be team of ORIGINAL unit to not fuck up encounter design
			PLOG_VERBOSE << "faction of original unit: " << factionToString.at(originalActorInfo.defaultTeam);
			hookData_currentUnitDatum = newUnitDatum; // store the data that the setActorDatum hook will need
			hookData_fixSentinelPosition = !originalActorInfo.isSentinel && newUnit->isSentinel; // used in position hook to move sentinels up from the ground
			// Spawn it! Call the original function. This will cause our fixUnitFaction and setActorDatum hooks to be hit, once per call.
					// They will ensure this by setting hookData_currentUnitsFaction & hookData_currentUnitDatum to null values after they're done.
			instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex);
		}

	}

	return true; // Tell the calling game function that we handled everything fine
	
}


void EnemyRandomiser::setActorDatumHookFunction(SafetyHookContext& ctx)
{
	PLOG_DEBUG << "setActorDatumHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	enum class param
	{
		actorDatum,
	};
	auto* ctxInterpreter = instance->setActorDatumFunctionContext.get();

	if (hookData_currentUnitDatum != nullDatum)
	{
		PLOG_DEBUG << "setting datum to " << hookData_currentUnitDatum;
		*ctxInterpreter->getParameterRef(ctx, (int)param::actorDatum) = (uint32_t)hookData_currentUnitDatum;
		hookData_currentUnitDatum = nullDatum;
	}


}


//setSpawnRNG

//setSpawnPosition



// TODO: rename to setUnitFaction
// Sets the (probably) randomized enemy's faction to be the same as that of the enemy before randomization
void EnemyRandomiser::fixUnitFactionHookFunction(SafetyHookContext& ctx)
{

	PLOG_VERBOSE << "fixUnitFactionHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		currentlySpawningUnitsFaction
	};
	auto* ctxInterpreter = instance->fixUnitFactionFunctionContext.get();

	if (hookData_currentUnitsFaction != faction::Undefined)
	{
		*ctxInterpreter->getParameterRef(ctx, (int)param::currentlySpawningUnitsFaction) = (UINT)hookData_currentUnitsFaction;
		PLOG_DEBUG << "Setting unit faction to " << factionToString.at(hookData_currentUnitsFaction);
		hookData_currentUnitsFaction = faction::Undefined;
	}
	else
	{
		PLOG_ERROR << "unit spawned with undefined last-unit-spawned-faction";
	}

}



void EnemyRandomiser::fixMajorUpgradeHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "fixMajorUpgradeHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	if (hookData_unitRandomised)
	{
		// Disallow major upgrades
		// set zeroFlag to 1 (bit 6) also parity flag (bit 2)
		ctx.rflags = ctx.rflags | (1UL << 6) | (1UL << 2);
	}


}

void EnemyRandomiser::spawnPositionFuzzHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "spawnPositionFuzzHookFunction";
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	enum class param
	{
		positionVec3
	};
	auto* ctxInterpreter = instance->spawnPositionFuzzFunctionContext.get();
	
	struct vec3 { float x, y, z; };

	SetSeed64 genx(hookData_currentUnitSeed);
	SetSeed64 geny(hookData_currentUnitSeed + 0xFF);
	float adjustx = (zeroToOne(genx) - 0.5f) / 10.f;
	float adjusty = (zeroToOne(geny) - 0.5f) / 10.f;

	vec3* pCurrentPosition = (vec3*)ctxInterpreter->getParameterRef(ctx, (int)param::positionVec3);
	PLOG_DEBUG << "pCurrentPosition: " << pCurrentPosition;



	if (hookData_fixSentinelPosition)
	{
		PLOG_VERBOSE << "applying sentinel position fix";
		pCurrentPosition->z += 1.f;
		hookData_fixSentinelPosition = false;
	}

	pCurrentPosition->x += adjustx;
	pCurrentPosition->y += adjusty;


}
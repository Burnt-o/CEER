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





__int64 EnemyRandomiser::newPlaceObjectFunction(tagBlock* paletteTableRef, objectData* spawningObject)
{
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	return instance->placeObjectHook.get()->getInlineHook().fastcall<__int64>(paletteTableRef, spawningObject);

	auto paletteTable = (tagReference*)instance->mapReader->getTagAddress(paletteTableRef->pointer);

	constexpr auto bipdMagic = MapReader::stringToMagic("bipd");
	if (paletteTable->tagGroupMagic != bipdMagic) return instance->placeObjectHook.get()->getInlineHook().fastcall<__int64>(paletteTableRef, spawningObject);

	//TODO

}






void EnemyRandomiser::vehicleExitHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "vehicleExitHookFunction";
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
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	// All we do here is set the zeroflag and parityflag to zero (bits 6 and 2 respectively)
	// (it'll be 1 if the ai trying to load into the vehicle isn't supposed to be in the vehicle)

	ctx.rflags = ctx.rflags & ~(1UL << 6) & ~(1UL << 2);
}





bool EnemyRandomiser::newProcessSquadUnitFunction(uint16_t encounterIndex, __int16 squadIndex)
{
#define returnOriginal return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex)
#define throwFromNewProcessSquadUnitFunction(message) CEERRuntimeException ex(message); \
						RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier }); \
	returnOriginal \

	PLOG_VERBOSE << "newProcessSquadFunction";
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
	PLOG_VERBOSE << "nullDatum: " << nullDatum;
	PLOG_VERBOSE << "equal? " << (originalActor == nullDatum);

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


	// Construct a seed
	uint64_t seed = instance->ourSeed ^ (((uint64_t)encounterIndex << 32) + ((uint64_t)squadIndex << 24) + ((uint64_t)hookData_currentSquadUnitIndex << 16)); // Create a seed from the specific enemy data & XOR with the user-input seed
	PLOG_DEBUG << "construction seed from" << std::hex << std::endl
		<< "instance->ourSeed " << instance->ourSeed << std::endl
		<< "encounterIndex " << encounterIndex << std::endl
		<< "((uint64_t)encounterIndex << 32) " << ((uint64_t)encounterIndex << 24) << std::endl
		<< "squadIndex " << squadIndex << std::endl
		<< "((uint64_t)squadIndex << 16) " << ((uint64_t)squadIndex << 16) << std::endl
		<< "hookData_currentSquadUnitIndex" << hookData_currentSquadUnitIndex << std::endl
	<< "(hookData_currentSquadUnitIndex << 8)" << (hookData_currentSquadUnitIndex << 8);
	
	
	
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

void EnemyRandomiser::getSquadUnitIndexHookFunction(SafetyHookContext& ctx)
{
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	PLOG_DEBUG << "getSquadUnitIndexHookFunction";
	enum class param
	{
		unitIndex,
	};
	auto* ctxInterpreter = instance->getSquadUnitIndexFunctionContext.get();

	hookData_currentSquadUnitIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::unitIndex);
	PLOG_VERBOSE << "setting hookData_currentSquadUnitIndex to " << hookData_currentSquadUnitIndex;
}


void EnemyRandomiser::setActorDatumHookFunction(SafetyHookContext& ctx)
{
	PLOG_DEBUG << "setActorDatumHookFunction";
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
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		majorDatum,
		normalDatum
	};
	auto* ctxInterpreter = instance->fixMajorUpgradeFunctionContext.get();


	if (hookData_unitRandomised)
	{
		// Disallow major upgrades
		auto normDat = *(uint32_t*)ctxInterpreter->getParameterRef(ctx, (int)param::normalDatum);
		*ctxInterpreter->getParameterRef(ctx, (int)param::majorDatum) = normDat;
		return;
	}
	else
	{
		return;
	}


}

void EnemyRandomiser::spawnPositionFuzzHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "spawnPositionFuzzHookFunction";
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
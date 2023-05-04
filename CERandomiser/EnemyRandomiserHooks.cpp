#include "pch.h"
#include "EnemyRandomiser.h"
#include "OptionsState.h"
#include "LevelLoadHook.h"
#include "EnemyRule.h"



std::uniform_real_distribution<double> zeroToOne{ 0.0, 1.0 };
faction EnemyRandomiser::hookData_currentUnitsFaction = faction::Undefined;
datum EnemyRandomiser::hookData_currentUnitDatum = nullDatum;
int EnemyRandomiser::hookData_currentSquadUnitIndex = 0;


// statics
//faction EnemyRandomiser::hookData_currentUnitsFaction = faction::Undefined;
//datum EnemyRandomiser::hookData_currentUnitDatum = nullDatum;
//int EnemyRandomiser::hookData_currentSquadUnitIndex = 0;

//void EnemyRandomiser::actvSpawnHookFunction(SafetyHookContext& ctx)
//{
//	return;
//	PLOG_VERBOSE << "actvSpawnHookFunction";
//	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
//	enum class param
//	{
//		actvPaletteIndex,
//		encounterIndex,
//		squadIndex,
//		unitIndex
//	};
//
//	auto* ctxInterpreter = instance->actvSpawnFunctionContext.get();
//
//	auto originalActvPaletteIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::actvPaletteIndex); // The actorPalette index of the type of unit trying to spawn
//	auto encounterIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::encounterIndex); // the index of its encounter 
//	auto squadIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::squadIndex); // the index of its squad
//	auto unitSquadIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::unitIndex); // this is the nth unit of that squad
//
//	if (!instance->actorMap.contains(originalActvPaletteIndex)) // safety check before we access the actorMap
//	{
//		throw_from_hook("actorMap did not contain currently spawning actor!", &OptionsState::MasterToggle)
//	}
//
//	auto originalActor = instance->actorMap.find(originalActvPaletteIndex)->second;
//
//	hookData_currentUnitsFaction = originalActor.defaultTeam; 	// Store the faction of the originally spawning unit - this data is used in fixUnitFactionHookFunction to unfuck allegiances
//
//
//	// Don't reroll invalid units
//	if (!originalActor.isValidUnit) return;
//
//	double randomizeProbability = originalActor.probabilityOfRandomize; // Lookup the probability that the originally spawning unit should be randomized
//	if (randomizeProbability == 0.0) return; // If the probability is zero then we won't randomize the enemy
//
//
//	uint64_t seed = instance->ourSeed ^ ((encounterIndex << 32) + (squadIndex << 16) + unitSquadIndex); // Create a seed from the specific enemy data & XOR with the user-input seed
//	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
//	PLOG_VERBOSE << "Seed: " << std::hex << seed;
//	PLOG_VERBOSE << "gen:  " << std::hex << generator();
//	if (zeroToOne(generator) > randomizeProbability) return; // Roll against the units randomize probability, if fail then we won't randomize the enemy
//
//	auto unitRoll = originalActor.rollDistribution(generator);// Re-use the seed to see what new enemy we should roll into
//	PLOG_VERBOSE << "rolled actv index: " << unitRoll;
//	*ctxInterpreter->getParameterRef(ctx, (int)param::actvPaletteIndex) = unitRoll; 	// Change the register containing the actvIndex to be the new unit
//
//
//}
//
//void EnemyRandomiser::placeObjectHookFunction(SafetyHookContext& ctx)
//{
//	return;
//	PLOG_VERBOSE << "placeObjectHookFunction";
//	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
//	enum class param
//	{
//		objectType,
//		paletteIndex,
//		objectIndex,
//		nameIndex,
//		paletteTable,
//	};
//
//	enum class objectType
//	{
//		Biped, // This is what we care about
//		Vehicle,
//		Weapon,
//		Equipment,
//		Garbage,
//		Projectile,
//		Scenery,
//		Machine,
//		Control,
//		LightFixture,
//		Placeholder,
//		SoundScenery,
//	};
//
//
//
//
//	auto* ctxInterpreter = instance->placeObjectFunctionContext.get();
//
//	auto objType = (objectType)*ctxInterpreter->getParameterRef(ctx, (int)param::objectType);
//
//	// TODO: change the whole thing to use the paletteTable magic lookup. The "objectType" param is not at all consistent 
//	if (!IsBadReadPtr((void*)ctx.rdx, 4))
//	{
//		if (((tagReference*)ctx.rdx)->tagGroupMagic == MapReader::stringToMagic("bipd"))
//		{
//			PLOG_DEBUG << "bipd happening";
//		}
//		else
//		{
//			PLOG_DEBUG << "non bipd tag palette: " << std::hex << ((tagReference*)ctx.rdx)->tagGroupMagic;
//		}
//	}
//	else
//	{
//		PLOG_VERBOSE << "could not read rdx";
//	}
//
//
//
//
//
//	if (objType != objectType::Biped) return; // We only care about biped spawns
//
//	auto bipdPaletteIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::paletteIndex); // The biped Palette index of the type of unit trying to spawn
//	auto bipdScenIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::objectIndex); // the index of the list of bipeds to spawn in the scenario tag
//	auto bipdNameIndex = (int)*ctxInterpreter->getParameterRef(ctx, (int)param::nameIndex); // the index of the name of this specific biped (not all bipeds have names)
//
//	if (!instance->bipedMap.contains(bipdPaletteIndex)) // safety check before we access the biepdMap
//	{
//		throw_from_hook("bipedMap did not contain currently spawning biped!", &OptionsState::MasterToggle)
//	}
//	auto originalBiped = instance->bipedMap.find(bipdPaletteIndex)->second;
//
//	hookData_currentUnitsFaction = originalBiped.defaultTeam; 	// Store the faction of the originally spawning unit - this data is used in fixUnitFactionHookFunction to unfuck allegiances
//
//	// Don't reroll invalid units
//	if (!originalBiped.isValidUnit) return;
//
//
//	// Safety check for the nipple-grunt at the end of the Maw.
//	// He cannot be randomised or the game will crash.
//	// We can use the nameIndex to look up the name of the currently spawning biped
//	PLOG_VERBOSE << "name of currently spawning biped: " << instance->mapReader->getObjectName(bipdNameIndex);
//	if (instance->mapReader->getObjectName(bipdNameIndex) == "nipple_grunt") return;
//
//	double randomizeProbability = originalBiped.probabilityOfRandomize; // Lookup the probability that the originally spawning unit should be randomized
//	if (randomizeProbability == 0.0) return; // If the probability is zero then we won't randomize the enemy
//
//	uint64_t seed = instance->ourSeed + bipdScenIndex; // Create a seed from the specific biped spawning
//	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
//	PLOG_VERBOSE << "Seed: " << std::hex << seed;
//	PLOG_VERBOSE << "gen:  " << std::hex << generator();
//	if (zeroToOne(generator) > randomizeProbability) return; // Roll against the units randomize probability, if fail then we won't randomize the enemy
//
//	auto unitRoll = originalBiped.rollDistribution(generator); // Re-use the seed to see what new enemy we should roll into
//	PLOG_VERBOSE << "rolled biped index: " << unitRoll;
//	*ctxInterpreter->getParameterRef(ctx, (int)param::paletteIndex) = unitRoll; 	// Change the register containing the actvIndex to be the new unit
//
//
//
//}



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





bool EnemyRandomiser::newProcessSquadUnitFunction(uint16_t encounterIndex, __int16 squadIndex, __int16 unknown)
{
	PLOG_VERBOSE << "newProcessSquadFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	//if (encounterIndex > 0xFFFF)
	//{
	//	PLOG_DEBUG << "encounter index was bad, returning early";
	//	return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
	//}


	datum originalActor;
	try
	{
		// Need to go to scenario tag and lookup the encounter & squad index ourselves, see what the original unit is
		originalActor = instance->mapReader->getEncounterSquadDatum(encounterIndex, squadIndex);
		PLOG_DEBUG << "original spawning actor: " << instance->mapReader->getTagName(originalActor);
	}
	catch (CEERRuntimeException& ex)
	{
		RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
		return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
	}

	if (!instance->actorMap.contains(originalActor)) // safety check before we access the map
	{
		CEERRuntimeException ex("actor spawning wasn't in our stored map!");
		RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
		return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
	}

	// get info of originally spawning actor
	UnitInfo& originalActorInfo = instance->actorMap.at(originalActor);

	if (!originalActorInfo.isValidUnit)
	{
		PLOG_DEBUG << "bailing on messing with invalid unit";
		// we shan't mess with it
		hookData_currentUnitsFaction = originalActorInfo.defaultTeam;
		hookData_currentUnitDatum = originalActor;
		return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
	}


	// Construct a seed
	uint64_t seed = instance->ourSeed ^ (((uint64_t)encounterIndex << 32) + ((uint64_t)squadIndex << 16) + hookData_currentSquadUnitIndex); // Create a seed from the specific enemy data & XOR with the user-input seed
	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
	PLOG_DEBUG << "seed set: " << std::hex << seed;
	
	// Apply pre-rando spawn multiplier
	double zeroToOneRoll = zeroToOne(generator);
	// A fun way to do a for loop. Use doubles!
	// This ensures that if the spawn multiplier is, say, 2.5x, we're guarenteed to spawn 2 enemies and have a 50% chance to spawn a third.
	for (double d = 0; d + zeroToOneRoll < originalActorInfo.spawnMultiplierPreRando; d++)
	{
		PLOG_DEBUG << "outer loop " << d;
		// Construct a new seed for each of these multiplied units
		seed += ((uint64_t)d) << 8;
		generator = SetSeed64(seed);

		// Roll randomization for each of these units
		UnitInfo& newUnit = originalActorInfo; // If not randomised, newUnit will actually be the original unit
		datum newUnitDatum = originalActor;
		PLOG_DEBUG << "probability of randomisze: " << originalActorInfo.probabilityOfRandomize;
		if (zeroToOne(generator) < originalActorInfo.probabilityOfRandomize) // Roll against the original units randomize probability, if true then we randomise
		{
			PLOG_DEBUG << "randomizing";
			auto unitRollIndex = originalActorInfo.rollDistribution(generator);

			if (unitRollIndex >= instance->actorDatumVector.size()) // safety check before we access the vector
			{
				CEERRuntimeException ex("rolled actor index was too large!");
				RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
				return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
			}

			newUnitDatum = instance->actorDatumVector.at(unitRollIndex); // Re-use the seed to see what new enemy we should roll into
			if (!instance->actorMap.contains(newUnitDatum)) // safety check before we access the map
			{
				CEERRuntimeException ex("actor spawning wasn't in our stored map!");
				RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
				return instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
			}
			newUnit = instance->actorMap.at(newUnitDatum);
			PLOG_DEBUG << "new unit: " << newUnit.getShortName();
			
		}

		// Apply post-rando spawn multiplier
		double zeroToOneRollPostRando = zeroToOne(generator);
		for (double e = 0; e + zeroToOneRoll < newUnit.spawnMultiplierPostRando; e++) // Important that we're checking the postRando multiplier of the NEW unit, not the original (ofc they will be the same if no randomisation occured, no biggie)
		{
			PLOG_DEBUG << "inner loop " << e;
			// store needed data for other hooks
			hookData_currentUnitsFaction = originalActorInfo.defaultTeam; // needed for fixUnitFaction. Must be team of ORIGINAL unit to not fuck up encounter design
			hookData_currentUnitDatum = newUnitDatum; // store the data that the setActorDatum hook will need
			// Spawn it! Call the original function. This will cause our fixUnitFaction and setActorDatum hooks to be hit, once per call.
					// They will ensure this by setting hookData_currentUnitsFaction & hookData_currentUnitDatum to null values after they're done.
			instance->processSquadUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
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
	auto* ctxInterpreter = instance->setActorDatumFunctionContext.get();

	hookData_currentSquadUnitIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::unitIndex);
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
	return;
	PLOG_VERBOSE << "fixUnitFactionHookFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		currentlySpawningUnitsFaction
	};
	auto* ctxInterpreter = instance->fixUnitFactionFunctionContext.get();

	if (hookData_currentUnitsFaction != faction::Undefined)
	{
		*ctxInterpreter->getParameterRef(ctx, (int)param::currentlySpawningUnitsFaction) = (UINT)hookData_currentUnitsFaction;
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
	auto majorDat = *(uint32_t*)ctxInterpreter->getParameterRef(ctx, (int)param::majorDatum);
	PLOG_DEBUG << "majorDatum: " << std::hex << majorDat;
	if (majorDat == 0xFFFFFFFF)
	{
		auto normDat = *(uint32_t*)ctxInterpreter->getParameterRef(ctx, (int)param::normalDatum);
		PLOG_DEBUG << "applying major upgrade fix: " << std::hex << normDat;
		*ctxInterpreter->getParameterRef(ctx, (int)param::majorDatum) = normDat;
	}

}

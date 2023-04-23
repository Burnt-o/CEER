#include "pch.h"
#include "EnemyRandomiser.h"
#include "OptionsState.h"
#include "LevelLoadHook.h"


EnemyRandomiser* EnemyRandomiser::instance = nullptr;

void EnemyRandomiser::onMasterToggleChanged(bool& newValue)
{
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

	if (newValue == false) // Master toggle was disabled
	{
		// turn hooks off
		PLOG_INFO << "Enemy Randomiser disabling hooks";

		instance->actvSpawnHook.get()->setWantsToBeAttached(newValue);
		instance->placeObjectHook.get()->setWantsToBeAttached(newValue);
		instance->encounterSpawnHook.get()->setWantsToBeAttached(newValue);
		instance->fixUnitFactionHook.get()->setWantsToBeAttached(newValue);
		instance->vehicleExitHook.get()->setWantsToBeAttached(newValue);
		instance->aiGoToVehicleHook.get()->setWantsToBeAttached(newValue);
		instance->aiLoadInVehicleHook.get()->setWantsToBeAttached(newValue);
		return;
	}
	else // Master toggle was enabled
	{
		if (OptionsState::EnemyRandomiser.GetValue())
		{
			// turn hooks on
			PLOG_INFO << "Enemy Randomiser enabling hooks";

			instance->actvSpawnHook.get()->setWantsToBeAttached(newValue);
			instance->placeObjectHook.get()->setWantsToBeAttached(newValue);
			instance->encounterSpawnHook.get()->setWantsToBeAttached(newValue);
			instance->fixUnitFactionHook.get()->setWantsToBeAttached(newValue);
			instance->vehicleExitHook.get()->setWantsToBeAttached(newValue);
			instance->aiGoToVehicleHook.get()->setWantsToBeAttached(newValue);
			instance->aiLoadInVehicleHook.get()->setWantsToBeAttached(newValue);


			// Check if already loaded into a level - if so call onLevelLoad
			HaloLevel outCurrentLevel;
			if (LevelLoadHook::isLevelAlreadyLoaded(outCurrentLevel))
			{
				onLevelLoadEvent(outCurrentLevel);
			}

		}


	}
}

template<size_t n>
void RemoveBadRoll(UnitInfo& info, const std::array<const char*, n> badNames) {
	for (auto badName : badNames)
	{
		if (info.fullName.contains(badName))
		{
			info.probabilityOfRoll = 0.0;
			return;
		}
	}
}

template<size_t n>
void RemoveBadRandomize(UnitInfo& info, const std::array<const char*, n> badNames) {
	for (auto badName : badNames)
	{
		if (info.fullName.contains(badName))
		{
			info.probabilityOfRandomize = 0.0;
			return;
		}
	}
}

// Turns "characters\hunter\hunter" into "hunter"
std::string getShortNameFromFull(std::string fullName)
{
	auto pos = fullName.find_last_of("\\");
	if (pos == std::string::npos) return fullName;
	std::string out = fullName.substr(pos + 1);
	return out;
}

constexpr std::array badUnitNames = { "flame thrower", "monitor", "captain", "engineer", "wounded", "cyborg", "cortana", "pilot", "detector" };

UnitInfo EnemyRandomiser::readActorInfo(actorTagReference* actor)
{
	constexpr std::array badActorsToRandomize = badUnitNames;
	constexpr std::array badActorsToRoll = badUnitNames;

	if (actor->tagGroupMagic != MapReader::reverseStringMagic("actv")) throw CEERRuntimeException("actv invalid magic!");

	UnitInfo thisActorInfo;
	thisActorInfo.fullName = instance->mapReader->getTagName(actor);
	thisActorInfo.shortName = getShortNameFromFull(thisActorInfo.fullName);

	bipedTagReference* bipdRef = instance->mapReader->getActorsBiped(actor);
	if (bipdRef)
	{
		thisActorInfo.defaultTeam = readBipedInfo(bipdRef).defaultTeam;
	}
	else
	{
		thisActorInfo.defaultTeam = faction::Undefined;
	}



	RemoveBadRandomize(thisActorInfo, badActorsToRandomize);
	RemoveBadRoll(thisActorInfo, badActorsToRoll);

	PLOG_DEBUG << thisActorInfo.shortName;

	return thisActorInfo;
}

UnitInfo EnemyRandomiser::readBipedInfo(bipedTagReference* biped)
{
	constexpr std::array badBipedsToRandomize = badUnitNames;
	constexpr std::array badBipedsToRoll = badUnitNames;

	if (biped->tagGroupMagic != MapReader::reverseStringMagic("bipd")) throw CEERRuntimeException("bipd invalid magic!");

	UnitInfo thisBipedInfo;
	thisBipedInfo.fullName = instance->mapReader->getTagName(biped);
	thisBipedInfo.shortName = getShortNameFromFull(thisBipedInfo.fullName);
	thisBipedInfo.defaultTeam = instance->mapReader->getBipedFaction(biped);

	RemoveBadRandomize(thisBipedInfo, badBipedsToRandomize);
	RemoveBadRoll(thisBipedInfo, badBipedsToRoll);

	PLOG_DEBUG << thisBipedInfo.shortName;

	return thisBipedInfo;
}

void EnemyRandomiser::evaluateActors(actorPaletteWrapper actorPalette)
{
	actorMap.clear();
	PLOG_DEBUG << "evaluating actors, count: " << actorPalette.tagCount;
	actorTagReference* currentActor = actorPalette.firstTag;
	for (int i = 0; i < actorPalette.tagCount; i++)
	{
		PLOG_DEBUG << i;
		actorMap.try_emplace(i, readActorInfo(currentActor));
		currentActor++;
	}

	// Setup the probability distribution that the randomizer will sample from
	std::vector<double> indexWeights;
	double cumulativeWeight = 0.0;
	for (auto& [index, randoInfo] : actorMap)
	{
		indexWeights.push_back(randoInfo.probabilityOfRoll);
		cumulativeWeight += randoInfo.probabilityOfRoll;
	}
	if (cumulativeWeight <= 0.0) throw CEERRuntimeException("no valid actors to roll!");

	PLOG_INFO << "Total actor weight: " << cumulativeWeight;

	actorIndexDistribution.param(std::discrete_distribution<int>::param_type(indexWeights.begin(), indexWeights.end()));
}

void EnemyRandomiser::evaluateBipeds(bipedPaletteWrapper bipedPalette)
{

	bipedMap.clear();
	PLOG_DEBUG << "evaluating bipeds, count: " << bipedPalette.tagCount;
	bipedTagReference* currentBiped = bipedPalette.firstTag;
	for (int i = 0; i < bipedPalette.tagCount; i++)
	{
		PLOG_DEBUG << i;
		bipedMap.try_emplace(i, readBipedInfo(currentBiped));
		currentBiped++;
	}

	
	// Setup the probability distribution that the randomizer will sample from
	std::vector<double> indexWeights;
	double cumulativeWeight = 0.0;
	for (auto& [index, randoInfo] : bipedMap)
	{
		indexWeights.push_back(randoInfo.probabilityOfRoll);
		cumulativeWeight += randoInfo.probabilityOfRoll;
	}
	if (cumulativeWeight <= 0.0) throw CEERRuntimeException("no valid bipeds to roll!");

	PLOG_INFO << "Total biped weight: " << cumulativeWeight;

	bipedIndexDistribution.param(std::discrete_distribution<int>::param_type(indexWeights.begin(), indexWeights.end()));

}






void EnemyRandomiser::onLevelLoadEvent(HaloLevel newLevel)
{
	try
	{
		std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
		PLOG_DEBUG << "loading game data";
		uintptr_t spawnposrng;
		if (!instance->spawnPositionRNG.get()->resolve(&spawnposrng)) throw CEERRuntimeException("Could not resolve spawnPositionRNG");
		instance->spawnPositionRNGResolved = (uint32_t*)spawnposrng;

		actorPaletteWrapper actorPalette = instance->mapReader->getActorPalette();
		PLOG_VERBOSE << "Actor palette tagCount: " << actorPalette.tagCount;

		instance->evaluateActors(actorPalette);

		bipedPaletteWrapper bipedPalette = instance->mapReader->getBipedPalette();
		PLOG_VERBOSE << "Biped palette tagCount: " << bipedPalette.tagCount;

		instance->evaluateBipeds(bipedPalette);



	}
	catch (CEERRuntimeException& ex)
	{
		PLOG_ERROR << "exception in EnemyRandomiser onLevelLoadEvent: " << ex.what();
		RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
	}
	catch (...)
	{
		CEERRuntimeException ex(ResurrectException());
		PLOG_FATAL << "unhandled exception in enemyRandomiser onLevelLoadEvent: " << ex.what();
		RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
	}

}




void EnemyRandomiser::actvSpawnHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "actvSpawnHookFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		actvPaletteIndex,
		encounterIndex,
		squadIndex,
		unitIndex
	};

	auto* ctxInterpreter = instance->actvSpawnFunctionContext.get();

	auto originalActvPaletteIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::actvPaletteIndex); // The actorPalette index of the type of unit trying to spawn
	auto encounterIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::encounterIndex); // the index of its encounter 
	auto squadIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::squadIndex); // the index of its squad
	auto unitSquadIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::unitIndex); // this is the nth unit of that squad

	if (!instance->actorMap.contains(originalActvPaletteIndex)) // safety check before we access the actorMap
	{
		throw_from_hook("actorMap did not contain currently spawning actor!", &OptionsState::MasterToggle)
	}

	instance->lastSpawnedUnitsFaction = instance->actorMap[originalActvPaletteIndex].defaultTeam; 	// Store the faction of the originally spawning unit - this data is used in fixUnitFactionHookFunction to unfuck allegiances

	float randomizeProbability = instance->actorMap[originalActvPaletteIndex].probabilityOfRandomize; // Lookup the probability that the originally spawning unit should be randomized
	if (randomizeProbability == 0.f) return; // If the probability is zero then we won't randomize the enemy


	uint64_t seed = instance->ourSeed ^ (encounterIndex << 32 + squadIndex << 16 + unitSquadIndex); // Create a seed from the specific enemy data & XOR with the user-input seed
	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
	PLOG_VERBOSE << "Seed: " << std::hex << seed;
	PLOG_VERBOSE << "gen:  " << std::hex << generator();
	if (instance->zeroToOne(generator) > randomizeProbability) return; // Roll against the units randomize probability, if fail then we won't randomize the enemy


	auto unitRoll = instance->actorIndexDistribution(generator); // Re-use the seed to see what new enemy we should roll into
	PLOG_VERBOSE << "rolled actv index: " << unitRoll;
	*ctxInterpreter->getParameterRef(ctx, (int)param::actvPaletteIndex) = unitRoll; 	// Change the register containing the actvIndex to be the new unit


}

void EnemyRandomiser::placeObjectHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "placeObjectHookFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		objectType,
		paletteIndex,
		objectIndex,
		nameIndex,
	};

	enum class objectType
	{
		Biped, // This is what we care about
		Vehicle,
		Weapon,
		Equipment,
		Garbage,
		Projectile,
		Scenery,
		Machine,
		Control,
		LightFixture,
		Placeholder,
		SoundScenery,
	};

	auto* ctxInterpreter = instance->placeObjectFunctionContext.get();

	auto objType = (objectType)*ctxInterpreter->getParameterRef(ctx, (int)param::objectType);
	if (objType != objectType::Biped) return; // We only care about biped spawns

	auto bipdPaletteIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::paletteIndex); // The biped Palette index of the type of unit trying to spawn
	auto bipdScenIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::objectIndex); // the index of the list of bipeds to spawn in the scenario tag
	auto bipdNameIndex = (int)*ctxInterpreter->getParameterRef(ctx, (int)param::nameIndex); // the index of the name of this specific biped (not all bipeds have names)

	if (!instance->bipedMap.contains(bipdPaletteIndex)) // safety check before we access the biepdMap
	{
		throw_from_hook("bipedMap did not contain currently spawning biped!", &OptionsState::MasterToggle)
	}

	instance->lastSpawnedUnitsFaction = instance->bipedMap[bipdPaletteIndex].defaultTeam; 	// Store the faction of the originally spawning unit - this data is used in fixUnitFactionHookFunction to unfuck allegiances

	// Safety check for the nipple-grunt at the end of the Maw.
	// He cannot be randomised or the game will crash.
	// We can use the nameIndex to look up the name of the currently spawning biped
	PLOG_VERBOSE << "name of currently spawning biped: " << instance->mapReader->getObjectName(bipdNameIndex);
	if (instance->mapReader->getObjectName(bipdNameIndex) == "nipple_grunt") return;

	float randomizeProbability = instance->bipedMap[bipdPaletteIndex].probabilityOfRandomize; // Lookup the probability that the originally spawning unit should be randomized
	if (randomizeProbability == 0.f) return; // If the probability is zero then we won't randomize the enemy

	uint64_t seed = instance->ourSeed + bipdScenIndex; // Create a seed from the specific biped spawning
	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
	PLOG_VERBOSE << "Seed: " << std::hex << seed;
	PLOG_VERBOSE << "gen:  " << std::hex << generator();
	if (instance->zeroToOne(generator) > randomizeProbability) return; // Roll against the units randomize probability, if fail then we won't randomize the enemy


	auto unitRoll = instance->bipedIndexDistribution(generator); // Re-use the seed to see what new enemy we should roll into
	PLOG_VERBOSE << "rolled biped index: " << unitRoll;
	*ctxInterpreter->getParameterRef(ctx, (int)param::paletteIndex) = unitRoll; 	// Change the register containing the actvIndex to be the new unit

}


// This function unrandomises spawn position RNG
// It runs once per squad spawn and sets the spawn-position-rng value to a mix of encIndex, sqdIndex, and our seed
// TODO: rename to squadSpawnHook
void EnemyRandomiser::encounterSpawnHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "encounterSpawnHookFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		encounterIndex,
		squadIndex
	};
	auto* ctxInterpreter = instance->encounterSpawnFunctionContext.get();

	

	auto encounterIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::encounterIndex);
	auto squadIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::squadIndex);
	SetSeed64 generator(instance->ourSeed + encounterIndex + (squadIndex * 100));
	*instance->spawnPositionRNGResolved = generator() % 0x100;


}

// TODO: rename to setUnitFaction
// Sets the (probably) randomized enemy's faction to be the same as that of the enemy before randomization
void EnemyRandomiser::fixUnitFactionHookFunction(SafetyHookContext& ctx)
{
	PLOG_VERBOSE << "fixUnitFactionHookFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		currentlySpawningUnitsFaction
	};
	auto* ctxInterpreter = instance->fixUnitFactionFunctionContext.get();

	if (instance->lastSpawnedUnitsFaction != faction::Undefined)
	{
		*ctxInterpreter->getParameterRef(ctx, (int)param::currentlySpawningUnitsFaction) = (UINT)instance->lastSpawnedUnitsFaction;
		instance->lastSpawnedUnitsFaction = faction::Undefined;
	}
	else
	{
		PLOG_ERROR << "unit spawned with undefined last-unit-spawned-faction";
	}

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

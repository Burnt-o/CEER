#include "pch.h"
#include "EnemyRandomiser.h"
#include "OptionsState.h"
#include "LevelLoadHook.h"
#include "EnemyRandomiserRule.h"

EnemyRandomiser* EnemyRandomiser::instance = nullptr;

void EnemyRandomiser::onMasterToggleChanged(bool& newValue)
{
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	instance->ProcessEncounterUnitHook.get()->setWantsToBeAttached(true);
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
bool isValidUnit(UnitInfo& info, const std::array<const char*, n> badNames)
{
	for (auto badName : badNames)
	{
		if (info.getFullName().contains(badName))
		{
			return false;
		}
	}
	return true;
}


constexpr std::array badUnitNames = { "flame", "monitor", "captain", "engineer", "wounded", "cyborg", "cortana", "pilot", "detector" };

UnitInfo EnemyRandomiser::readActorInfo(const tagElement& actor)
{
	UnitInfo thisActorInfo(instance->mapReader->getTagName(actor.tagDatum));
	thisActorInfo.defaultTeam = instance->mapReader->getActorsFaction(actor.tagDatum);
	PLOG_DEBUG << thisActorInfo.getShortName();
	thisActorInfo.isValidUnit = isValidUnit(thisActorInfo, badUnitNames);

	return thisActorInfo;
}

UnitInfo EnemyRandomiser::readBipedInfo(const tagElement& biped)
{
	UnitInfo thisBipedInfo(instance->mapReader->getTagName(biped.tagDatum));
	thisBipedInfo.defaultTeam = instance->mapReader->getBipedFaction(biped.tagDatum);
	PLOG_DEBUG << thisBipedInfo.getShortName();
	thisBipedInfo.isValidUnit = isValidUnit(thisBipedInfo, badUnitNames);

	return thisBipedInfo;
}


void EnemyRandomiser::evaluateActors()
{
	PLOG_DEBUG << "Evaluating actors.";
	datumToActorMap.clear();

	auto tagTable = instance->mapReader->getTagTable();		PLOG_DEBUG << "TagTable count: " << tagTable.size();
	auto actvMagic = MapReader::stringToMagic("actv");

	for (const auto& tag : tagTable)
	{
		//PLOG_VERBOSE << "index" << tag.tagDatum.index;
		if (tag.tagGroupMagic != actvMagic || tag.tagDatum == nullDatum) continue;
		datumToActorMap.try_emplace(tag.tagDatum, readActorInfo(tag));

	}

	// Iterate over all actors, and for each one construct their rollDistribution
	for (auto& [datum, actor] : datumToActorMap)
	{
		if (!actor.isValidUnit) continue;

		EnemyRandomiserGroup* rollGroup = nullptr;
		// Evaluate rules (in reverse, so rules at top of GUI overwrite ones at bottom)
		for (auto& rule : std::ranges::views::reverse(OptionsState::currentRules))
		{
			PLOG_DEBUG << "Actor checking if a rule applies to it: " << actor.getShortName();
			if (rule.get()->getType() == RuleType::RandomiseXintoY)
			{
				RandomiseXintoY* thisRule = dynamic_cast<RandomiseXintoY*>(rule.get());
				if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

				if (thisRule->randomiseGroupSelection.isMatch(actor))
				{
					PLOG_DEBUG << "Actor found a matching rule: " << actor.getShortName();
					actor.probabilityOfRandomize = thisRule->randomisePercent.GetValue() / 100.f;
					rollGroup = &thisRule->rollGroupSelection;
				}
			}
		}

		PLOG_DEBUG << "Constructing rollDistribution for " << actor.getShortName();
		std::vector<double> indexWeights;
		double cumulativeWeight = 0.0;
		for (auto& [datum, otherActor] : datumToActorMap) // Iterate over all actors again, checking if they're within the rollGroup
		{

			if (rollGroup != nullptr && rollGroup->isMatch(otherActor) && otherActor.isValidUnit)
			{
				indexWeights.push_back(1.0);
				cumulativeWeight += 1.0;
			}
			else
			{
				if (otherActor.getShortName().contains("marine") && otherActor.getShortName().contains("sniper") && otherActor.getShortName().contains("major"))
				{
					PLOG_ERROR << "what the fuck";
					PLOG_VERBOSE << "otherActor bad" << std::endl
						<< "rollGroup != nullptr: " << (rollGroup != nullptr) << std::endl
						<< "rollGroup->isMatch(otherActor): " << (rollGroup->isMatch(otherActor)) << std::endl
						<< "otherActor.isValidUnit: " << (otherActor.isValidUnit) << std::endl;
					std::string compString = "marine_armored sniper rifle major";
					PLOG_ERROR << "otherActor.getShortName().length: " << otherActor.getShortName().length();
					PLOG_ERROR << "compString.length: " << compString.length();
					PLOG_ERROR << "actual comparison: " << (otherActor.getShortName() == "marine_armored sniper rifle major");
					PLOG_ERROR << "data comparison: " << (otherActor.getShortName().data() == "marine_armored sniper rifle major");
					PLOG_ERROR << "contains comparison: " << (otherActor.getShortName().contains("marine_armored sniper rifle major"));

					//throw (
					//	
					//	"Ah.. I see the issue. The recompiled map has all the actors, but they're not all in the actor palette. We're only iterating the actor palette. Huh.
					//	"We should go back to throwing on having no valid rolls btw, I think"
					//	")

				}


				indexWeights.push_back(0.0);
				cumulativeWeight += 0.0;
			}
		}
		if (cumulativeWeight <= 0.0)
		{
			PLOG_ERROR << std::format("actor {} had no valid rolls to roll, strange", actor.getShortName());
			actor.probabilityOfRandomize = 0.0;
		}
		else
		{
			actor.rollDistribution.param(std::discrete_distribution<int>::param_type(indexWeights.begin(), indexWeights.end()));
		}


	}

}


void EnemyRandomiser::evaluateBipeds()
{


	PLOG_DEBUG << "Evaluating bipeds.";
	datumToBipedMap.clear();

	auto tagTable = instance->mapReader->getTagTable();
	auto bipdMagic = MapReader::stringToMagic("bipd");

	for (const auto& tag : tagTable)
	{
		if (tag.tagGroupMagic != bipdMagic || tag.tagDatum == nullDatum) continue;
		datumToBipedMap.try_emplace(tag.tagDatum, readBipedInfo(tag));

	}
	
	// Iterate over all bipeds, and for each one construct their rollDistribution
	for (auto& [datum, biped] : datumToBipedMap)
	{
		if (!biped.isValidUnit) continue;

		EnemyRandomiserGroup* rollGroup = nullptr;
		// Evaluate rules (in reverse, so rules at top of GUI overwrite ones at bottom)
		for (auto& rule : std::ranges::views::reverse(OptionsState::currentRules))
		{
			if (rule.get()->getType() == RuleType::RandomiseXintoY)
			{
				RandomiseXintoY* thisRule = dynamic_cast<RandomiseXintoY*>(rule.get());
				if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

				if (thisRule->randomiseGroupSelection.isMatch(biped))
				{
					biped.probabilityOfRandomize = thisRule->randomisePercent.GetValue() / 100.f;
					rollGroup = &thisRule->rollGroupSelection;
				}

			}
		}

		std::vector<double> indexWeights;
		double cumulativeWeight = 0.0;
		for (auto& [datum, otherBiped] : datumToBipedMap) // Iterate over all bipeds again, checking if they're within the rollGroup
		{
			if (rollGroup && rollGroup->isMatch(otherBiped) && otherBiped.isValidUnit)
			{
				indexWeights.push_back(1.0);
				cumulativeWeight += 1.0;
			}
			else
			{
				indexWeights.push_back(0.0);
				cumulativeWeight += 0.0;
			}
		}
		if (cumulativeWeight <= 0.0)
		{
			PLOG_ERROR << std::format("biped {} had no valid rolls to roll, strange", biped.getShortName());
			biped.probabilityOfRandomize = 0.0;
		}
		else
		{
			biped.rollDistribution.param(std::discrete_distribution<int>::param_type(indexWeights.begin(), indexWeights.end()));
		}


	}

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


		instance->evaluateActors();

		instance->evaluateBipeds();



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
	return;
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

	auto originalActor = instance->actorMap.find(originalActvPaletteIndex)->second;

	instance->lastSpawnedUnitsFaction = originalActor.defaultTeam; 	// Store the faction of the originally spawning unit - this data is used in fixUnitFactionHookFunction to unfuck allegiances


	// Don't reroll invalid units
	if (!originalActor.isValidUnit) return;

	double randomizeProbability = originalActor.probabilityOfRandomize; // Lookup the probability that the originally spawning unit should be randomized
	if (randomizeProbability == 0.0) return; // If the probability is zero then we won't randomize the enemy


	uint64_t seed = instance->ourSeed ^ ((encounterIndex << 32) + (squadIndex << 16) + unitSquadIndex); // Create a seed from the specific enemy data & XOR with the user-input seed
	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
	PLOG_VERBOSE << "Seed: " << std::hex << seed;
	PLOG_VERBOSE << "gen:  " << std::hex << generator();
	if (instance->zeroToOne(generator) > randomizeProbability) return; // Roll against the units randomize probability, if fail then we won't randomize the enemy

	auto unitRoll = originalActor.rollDistribution(generator);// Re-use the seed to see what new enemy we should roll into
	PLOG_VERBOSE << "rolled actv index: " << unitRoll;
	*ctxInterpreter->getParameterRef(ctx, (int)param::actvPaletteIndex) = unitRoll; 	// Change the register containing the actvIndex to be the new unit


}

void EnemyRandomiser::placeObjectHookFunction(SafetyHookContext& ctx)
{
	return;
	PLOG_VERBOSE << "placeObjectHookFunction";
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		objectType,
		paletteIndex,
		objectIndex,
		nameIndex,
		paletteTable,
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

	// TODO: change the whole thing to use the paletteTable magic lookup. The "objectType" param is not at all consistent 
	if (!IsBadReadPtr((void*)ctx.rdx, 4))
	{
		if (((tagReference*)ctx.rdx)->tagGroupMagic == MapReader::stringToMagic("bipd"))
		{
			PLOG_DEBUG << "bipd happening";
		}
		else
		{
			PLOG_DEBUG << "non bipd tag palette: " << std::hex << ((tagReference*)ctx.rdx)->tagGroupMagic;
		}
	}
	else
	{
		PLOG_VERBOSE << "could not read rdx";
	}





	if (objType != objectType::Biped) return; // We only care about biped spawns

	auto bipdPaletteIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::paletteIndex); // The biped Palette index of the type of unit trying to spawn
	auto bipdScenIndex = *ctxInterpreter->getParameterRef(ctx, (int)param::objectIndex); // the index of the list of bipeds to spawn in the scenario tag
	auto bipdNameIndex = (int)*ctxInterpreter->getParameterRef(ctx, (int)param::nameIndex); // the index of the name of this specific biped (not all bipeds have names)

	if (!instance->bipedMap.contains(bipdPaletteIndex)) // safety check before we access the biepdMap
	{
		throw_from_hook("bipedMap did not contain currently spawning biped!", &OptionsState::MasterToggle)
	}
	auto originalBiped = instance->bipedMap.find(bipdPaletteIndex)->second;

	instance->lastSpawnedUnitsFaction = originalBiped.defaultTeam; 	// Store the faction of the originally spawning unit - this data is used in fixUnitFactionHookFunction to unfuck allegiances

	// Don't reroll invalid units
	if (!originalBiped.isValidUnit) return;


	// Safety check for the nipple-grunt at the end of the Maw.
	// He cannot be randomised or the game will crash.
	// We can use the nameIndex to look up the name of the currently spawning biped
	PLOG_VERBOSE << "name of currently spawning biped: " << instance->mapReader->getObjectName(bipdNameIndex);
	if (instance->mapReader->getObjectName(bipdNameIndex) == "nipple_grunt") return;

	double randomizeProbability = originalBiped.probabilityOfRandomize; // Lookup the probability that the originally spawning unit should be randomized
	if (randomizeProbability == 0.0) return; // If the probability is zero then we won't randomize the enemy

	uint64_t seed = instance->ourSeed + bipdScenIndex; // Create a seed from the specific biped spawning
	SetSeed64 generator(seed); // Needed to interact with <random>, also twists our number
	PLOG_VERBOSE << "Seed: " << std::hex << seed;
	PLOG_VERBOSE << "gen:  " << std::hex << generator();
	if (instance->zeroToOne(generator) > randomizeProbability) return; // Roll against the units randomize probability, if fail then we won't randomize the enemy

	auto unitRoll = originalBiped.rollDistribution(generator); // Re-use the seed to see what new enemy we should roll into
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





bool EnemyRandomiser::newProcessEncounterUnit(unsigned int encounterIndex, __int16 squadIndex, __int16 unknown)
{
	//PLOG_DEBUG << "test: " << encounterIndex;
	// call it twice as a test
	int extraSpawns = 1;
	for (int i = 0; i < extraSpawns; i++)
	{
		instance->ProcessEncounterUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);
	}

	// and do the original spawn
	return instance->ProcessEncounterUnitHook.get()->getInlineHook().fastcall<bool>(encounterIndex, squadIndex, unknown);;
}
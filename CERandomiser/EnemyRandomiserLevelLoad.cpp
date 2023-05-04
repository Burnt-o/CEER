#include "pch.h"
#include "EnemyRandomiser.h"
#include "OptionsState.h"
#include "LevelLoadHook.h"
#include "EnemyRule.h"

EnemyRandomiser* EnemyRandomiser::instance = nullptr;

void EnemyRandomiser::onMasterToggleChanged(bool& newValue)
{
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	if (newValue == false) // Master toggle was disabled
	{
		// turn hooks off
		PLOG_INFO << "Enemy Randomiser disabling hooks";


		instance->vehicleExitHook.get()->setWantsToBeAttached(newValue);
		instance->aiGoToVehicleHook.get()->setWantsToBeAttached(newValue);
		instance->aiLoadInVehicleHook.get()->setWantsToBeAttached(newValue);

		//instance->preSquadSpawnHook.get()->setWantsToBeAttached(newValue);
		instance->setActorDatumHook.get()->setWantsToBeAttached(newValue);
		//instance->fixUnitFactionHook.get()->setWantsToBeAttached(newValue);
		//instance->postSquadSpawnHook.get()->setWantsToBeAttached(newValue);
		instance->processSquadUnitHook.get()->setWantsToBeAttached(newValue);
		instance->getSquadUnitIndexHook.get()->setWantsToBeAttached(newValue);
		instance->fixMajorUpgradeHook.get()->setWantsToBeAttached(newValue);
		instance->spawnPositionFuzzHook.get()->setWantsToBeAttached(newValue);
		return;
	}
	else // Master toggle was enabled
	{
		if (OptionsState::EnemyRandomiser.GetValue())
		{
			// turn hooks on
			PLOG_INFO << "Enemy Randomiser enabling hooks";


			instance->fixUnitFactionHook.get()->setWantsToBeAttached(newValue);
			instance->vehicleExitHook.get()->setWantsToBeAttached(newValue);
			instance->aiGoToVehicleHook.get()->setWantsToBeAttached(newValue);
			instance->aiLoadInVehicleHook.get()->setWantsToBeAttached(newValue);



			//instance->preSquadSpawnHook.get()->setWantsToBeAttached(newValue);
			instance->setActorDatumHook.get()->setWantsToBeAttached(newValue);
			//instance->fixUnitFactionHook.get()->setWantsToBeAttached(newValue);
			//instance->postSquadSpawnHook.get()->setWantsToBeAttached(newValue);
			instance->processSquadUnitHook.get()->setWantsToBeAttached(newValue);
			instance->getSquadUnitIndexHook.get()->setWantsToBeAttached(newValue);
			instance->fixMajorUpgradeHook.get()->setWantsToBeAttached(newValue);
			instance->spawnPositionFuzzHook.get()->setWantsToBeAttached(newValue);

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

UnitInfo EnemyRandomiser::readActorInfo(const datum actorDatum)
{
	UnitInfo thisActorInfo(instance->mapReader->getTagName(actorDatum));
	thisActorInfo.defaultTeam = instance->mapReader->getActorsFaction(actorDatum);
	PLOG_DEBUG << thisActorInfo.getShortName();
	thisActorInfo.isValidUnit = isValidUnit(thisActorInfo, badUnitNames);

	return thisActorInfo;
}

UnitInfo EnemyRandomiser::readBipedInfo(const datum bipedDatum)
{
	UnitInfo thisBipedInfo(instance->mapReader->getTagName(bipedDatum));
	thisBipedInfo.defaultTeam = instance->mapReader->getBipedFaction(bipedDatum);
	PLOG_DEBUG << thisBipedInfo.getShortName();
	thisBipedInfo.isValidUnit = isValidUnit(thisBipedInfo, badUnitNames);

	return thisBipedInfo;
}


void EnemyRandomiser::evaluateActors()
{
	PLOG_DEBUG << "Evaluating actors.";
	actorDatumVector.clear();
	actorMap.clear();

	auto tagTable = instance->mapReader->getTagTable();		PLOG_DEBUG << "TagTable count: " << tagTable.size();
	constexpr auto actvMagic = MapReader::stringToMagic("actv");

	for (const auto& tag : tagTable)
	{
		//PLOG_VERBOSE << "index" << tag.tagDatum.index;
		if (tag.tagGroupMagic != actvMagic || tag.tagDatum == nullDatum) continue;
		actorDatumVector.emplace_back(tag.tagDatum);
		actorMap.emplace(tag.tagDatum, readActorInfo(tag.tagDatum));
	}

	// Iterate over all actors, and for each one construct their rollDistribution
	for (auto& [mDatum, actor] : actorMap)
	{
		if (!actor.isValidUnit) continue;

		EnemyGroup* rollGroup = nullptr;
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
		for (auto& [mDatum, otherActor] : actorMap) // Iterate over all actors again, checking if they're within the rollGroup
		{

			if (rollGroup != nullptr && rollGroup->isMatch(otherActor) && otherActor.isValidUnit)
			{
				indexWeights.push_back(1.0);
				cumulativeWeight += 1.0;
			}
			else
			{
				//if (otherActor.getShortName().contains("marine") && otherActor.getShortName().contains("sniper") && otherActor.getShortName().contains("major"))
				//{
				//	PLOG_ERROR << "what the fuck";
				//	PLOG_VERBOSE << "otherActor bad" << std::endl
				//		<< "rollGroup != nullptr: " << (rollGroup != nullptr) << std::endl
				//		<< "rollGroup->isMatch(otherActor): " << (rollGroup->isMatch(otherActor)) << std::endl
				//		<< "otherActor.isValidUnit: " << (otherActor.isValidUnit) << std::endl;
				//	std::string compString = "marine_armored sniper rifle major";
				//	PLOG_ERROR << "otherActor.getShortName().length: " << otherActor.getShortName().length();
				//	PLOG_ERROR << "compString.length: " << compString.length();
				//	PLOG_ERROR << "actual comparison: " << (otherActor.getShortName() == "marine_armored sniper rifle major");
				//	PLOG_ERROR << "data comparison: " << (otherActor.getShortName().data() == "marine_armored sniper rifle major");
				//	PLOG_ERROR << "contains comparison: " << (otherActor.getShortName().contains("marine_armored sniper rifle major"));

				//	//throw (
				//	//	
				//	//	"Ah.. I see the issue. The recompiled map has all the actors, but they're not all in the actor palette. We're only iterating the actor palette. Huh.
				//	//	"We should go back to throwing on having no valid rolls btw, I think"
				//	//	")

				//}


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
			PLOG_DEBUG << "rollDistribution cumulativeWeight: " << cumulativeWeight;
			actor.rollDistribution.param(std::discrete_distribution<int>::param_type(indexWeights.begin(), indexWeights.end()));
		}

	}

	// Iterate over all actors, and for each one set their pre and post-randomisation spawn multiplier
	for (auto& [mDatum, actor] : actorMap)
	{
		if (!actor.isValidUnit) continue;

		// Evaluate rules (in reverse, so rules at top of GUI overwrite ones at bottom)
		for (auto& rule : std::ranges::views::reverse(OptionsState::currentRules))
		{
			PLOG_DEBUG << "Actor checking if a rule applies to it: " << actor.getShortName();
			switch (rule.get()->getType())
			{
			case RuleType::SpawnMultiplierPreRando:
			{
				SpawnMultiplierPreRando* thisRule = dynamic_cast<SpawnMultiplierPreRando*>(rule.get());
				if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

				if (thisRule->groupSelection.isMatch(actor))
				{
					// multiply instead of set, so multiple rules applying to the same group have a multiplicative effect
					actor.spawnMultiplierPreRando *= thisRule->multiplyPercent.GetValue() / 100.f;
				}
			}
			break;

			case RuleType::SpawnMultiplierPostRando:
			{
				SpawnMultiplierPostRando* thisRule = dynamic_cast<SpawnMultiplierPostRando*>(rule.get());
				if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

				if (thisRule->groupSelection.isMatch(actor))
				{
					// multiply instead of set, so multiple rules applying to the same group have a multiplicative effect
					actor.spawnMultiplierPostRando *= thisRule->multiplyPercent.GetValue() / 100.f;
				}
			}
			break;

			default:
				break;
			}
			
		}
	}

	assert(actorDatumVector.size() == actorMap.size());
}


void EnemyRandomiser::evaluateBipeds()
{


	PLOG_DEBUG << "Evaluating bipeds.";
	bipedMap.clear();
	bipedDatumVector.clear();
	auto tagTable = instance->mapReader->getTagTable();
	constexpr auto bipdMagic = MapReader::stringToMagic("bipd");

	int unitIndex = 0;
	for (const auto& tag : tagTable)
	{
		if (tag.tagGroupMagic != bipdMagic || tag.tagDatum == nullDatum) continue;
		bipedDatumVector.emplace_back(tag.tagDatum);
		bipedMap.emplace(tag.tagDatum, readBipedInfo(tag.tagDatum));
		unitIndex++;

	}
	
	// Iterate over all bipeds, and for each one construct their rollDistribution
	for (auto& [mDatum, biped] : bipedMap)
	{
		if (!biped.isValidUnit) continue;

		EnemyGroup* rollGroup = nullptr;
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
		for (auto& [mDatum, otherBiped] : bipedMap) // Iterate over all bipeds again, checking if they're within the rollGroup
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
	assert(bipedDatumVector.size() == bipedMap.size());
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



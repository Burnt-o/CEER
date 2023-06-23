#include "pch.h"
#include "EnemyRandomiser.h"
#include "OptionsState.h"
#include "LevelLoadHook.h"
#include "EnemyRule.h"
#include "UserSeed.h"
#include "MessagesGUI.h"
EnemyRandomiser* EnemyRandomiser::instance = nullptr;


void EnemyRandomiser::lazyInit()
{
	PLOG_DEBUG << "EnemyRandomiser::lazyInit()";
	// Set up our hooks and get our pointers
	try
	{
		gameSpawnRNG = PointerManager::getMultilevelPointer("gameSpawnRNG");
		gameRNG = PointerManager::getMultilevelPointer("gameRNG");


		//fixes
		auto vehicleExitFunction = PointerManager::getMultilevelPointer("vehicleExitFunction");
		vehicleExitFunctionContext = PointerManager::getMidhookContextInterpreter("vehicleExitFunctionContext");
		vehicleExitHook = ModuleMidHook::make(L"halo1.dll", vehicleExitFunction, vehicleExitHookFunction, false);

		auto aiGoToVehicleFunction = PointerManager::getMultilevelPointer("aiGoToVehicleFunction");
		aiGoToVehicleFunctionContext = PointerManager::getMidhookContextInterpreter("aiGoToVehicleFunctionContext");
		aiGoToVehicleHook = ModuleMidHook::make(L"halo1.dll", aiGoToVehicleFunction, aiGoToVehicleHookFunction, false);

		auto aiLoadInVehicleFunction = PointerManager::getMultilevelPointer("aiLoadInVehicleFunction");
		aiLoadInVehicleHook = ModuleMidHook::make(L"halo1.dll", aiLoadInVehicleFunction, aiLoadInVehicleHookFunction, false);

		auto killVehicleOverflowFunction = PointerManager::getMultilevelPointer("killVehicleOverflowFunction");
		killVehicleOverflowFunctionContext = PointerManager::getMidhookContextInterpreter("killVehicleOverflowFunctionContext");
		killVehicleOverflowHook = ModuleMidHook::make(L"halo1.dll", killVehicleOverflowFunction, killVehicleOverflowHookFunction, false);

		// bipeds
#if bipedRandomisation == 1
		auto placeObjectFunction = PointerManager::getMultilevelPointer("placeObjectFunction");
		placeObjectHook = ModuleInlineHook::make(L"halo1.dll", placeObjectFunction, newPlaceObjectFunction, false);

		auto setBipedDatumFunction = PointerManager::getMultilevelPointer("setBipedDatumFunction");
		setBipedDatumFunctionContext = PointerManager::getMidhookContextInterpreter("setBipedDatumFunctionContext");
		setBipedDatumHook = ModuleMidHook::make(L"halo1.dll", setBipedDatumFunction, setBipedDatumHookFunction, false);

		auto fixWinstoreBipedCrashFunction = PointerManager::getMultilevelPointer("fixWinstoreBipedCrashFunction");
		auto fixWinstoreBipedCrashPatchBytes = PointerManager::getVector<byte>("fixWinstoreBipedCrashPatchBytes");

		for (auto b : fixWinstoreBipedCrashPatchBytes)
		{
			PLOG_VERBOSE << "b: " << std::hex << b;
		}
		fixWinstoreBipedCrashPatch = ModulePatch::make(L"halo1.dll", fixWinstoreBipedCrashFunction, fixWinstoreBipedCrashPatchBytes, false);
#endif
		 //actors
		auto processSquadUnitFunction = PointerManager::getMultilevelPointer("processSquadUnitFunction");
		processSquadUnitHook = ModuleInlineHook::make(L"halo1.dll", processSquadUnitFunction, newProcessSquadUnitFunction, false);

		auto setActorDatumFunction = PointerManager::getMultilevelPointer("setActorDatumFunction");
		setActorDatumFunctionContext = PointerManager::getMidhookContextInterpreter("setActorDatumFunctionContext");
		setActorDatumHook = ModuleMidHook::make(L"halo1.dll", setActorDatumFunction, setActorDatumHookFunction, false);

		auto fixUnitFactionFunction = PointerManager::getMultilevelPointer("fixUnitFactionFunction");
		fixUnitFactionFunctionContext = PointerManager::getMidhookContextInterpreter("fixUnitFactionFunctionContext");
		fixUnitFactionHook = ModuleMidHook::make(L"halo1.dll", fixUnitFactionFunction, fixUnitFactionHookFunction, false);

		auto fixMajorUpgradeFunction = PointerManager::getMultilevelPointer("fixMajorUpgradeFunction");
		fixMajorUpgradeHook = ModuleMidHook::make(L"halo1.dll", fixMajorUpgradeFunction, fixMajorUpgradeHookFunction, false);

		auto spawnPositionFuzzFunction = PointerManager::getMultilevelPointer("spawnPositionFuzzFunction");
		spawnPositionFuzzFunctionContext = PointerManager::getMidhookContextInterpreter("spawnPositionFuzzFunctionContext");
		spawnPositionFuzzHook = ModuleMidHook::make(L"halo1.dll", spawnPositionFuzzFunction, spawnPositionFuzzHookFunction, false);



	}
	catch (InitException& ex)
	{
		ex.prepend("EnemyRandomiser could not resolve hooks: ");
		throw ex;
	}
}

std::mutex enemyToggleChangeMutex;
void EnemyRandomiser::onEitherOptionChange()
{

	std::scoped_lock<std::mutex> lock(enemyToggleChangeMutex);

	// we will turn on hooks if either EnemyRandomiser OR EnemySpawnMultiplier is enabled. Off if both are off.
	bool shouldEnable = OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue();

	//lazy init (getting pointerData, creating hook objects)
		try
		{
			std::call_once(lazyInitOnceFlag, [this]() {lazyInit(); }); // flag not flipped if exception thrown
		}
		catch (InitException& ex)
		{
			RuntimeExceptionHandler::handleMessage(ex);
			OptionsState::EnemyRandomiser.GetValue() = false;
			OptionsState::EnemyRandomiser.GetValueDisplay() = false;
			OptionsState::EnemySpawnMultiplier.GetValue() = false;
			OptionsState::EnemySpawnMultiplier.GetValueDisplay() = false;
			return;
		}


	// set hook state
	vehicleExitHook.get()->setWantsToBeAttached(shouldEnable);
	aiGoToVehicleHook.get()->setWantsToBeAttached(shouldEnable);
	aiLoadInVehicleHook.get()->setWantsToBeAttached(shouldEnable);
	killVehicleOverflowHook.get()->setWantsToBeAttached(shouldEnable);
	fixUnitFactionHook.get()->setWantsToBeAttached(shouldEnable);
	setActorDatumHook.get()->setWantsToBeAttached(shouldEnable);
	processSquadUnitHook.get()->setWantsToBeAttached(shouldEnable);

	fixMajorUpgradeHook.get()->setWantsToBeAttached(shouldEnable);
	spawnPositionFuzzHook.get()->setWantsToBeAttached(shouldEnable);
#if bipedRandomisation == 1
	placeObjectHook.get()->setWantsToBeAttached(shouldEnable);
	setBipedDatumHook.get()->setWantsToBeAttached(shouldEnable);
	fixWinstoreBipedCrashPatch.get()->setWantsToBeAttached(shouldEnable);
#endif

	if (shouldEnable)
	{
		// Get rng seed
		ourSeed = UserSeed::GetCurrentSeed();

		// check if a level is already loaded, if so call onLevelLoad
		HaloLevel currentLevel;
		if (LevelLoadHook::isLevelAlreadyLoaded(currentLevel))
		{
			onLevelLoadEvent(currentLevel);
		}

	}


}


void EnemyRandomiser::onEnemyRandomiserToggleChange(bool& newValue)
{
	instance->onEitherOptionChange();
}

void EnemyRandomiser::onEnemySpawnMultiplierToggleChange(bool& newValue)
{
	instance->onEitherOptionChange();
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

#if includeFlamethrowerFloodOption == 1
	if (!OptionsState::RandomiserIncludesFlameThrowers.GetValue())
	{
		if (info.getFullName().contains("flame"))
		{
			return false;
		}
	}
#else
	if (info.getFullName().contains("flame"))
	{
		return false;
	}
#endif

	return true;
}


constexpr std::array badUnitNames = { "monitor", "captain", "engineer", "wounded", "cyborg", "cortana", "pilot", "detector", "gunner", "nopop", "cd_gun", "suicidal", "special", "greeny", "purpy", "airdef"};

UnitInfo EnemyRandomiser::readActorInfo(const datum actorDatum)
{
	UnitInfo thisActorInfo(instance->mapReader->getTagName(actorDatum));
	thisActorInfo.defaultTeam = instance->mapReader->getActorsFaction(actorDatum);
	PLOG_DEBUG << thisActorInfo.getShortName();
	thisActorInfo.isValidUnit = isValidUnit(thisActorInfo, badUnitNames);
	thisActorInfo.isSentinel = thisActorInfo.getShortName().contains("sentinel");
	return thisActorInfo;
}

UnitInfo EnemyRandomiser::readBipedInfo(const datum bipedDatum)
{
	UnitInfo thisBipedInfo(instance->mapReader->getTagName(bipedDatum));
	thisBipedInfo.defaultTeam = instance->mapReader->getBipedFaction(bipedDatum);
	PLOG_DEBUG << thisBipedInfo.getShortName();
	thisBipedInfo.isValidUnit = isValidUnit(thisBipedInfo, badUnitNames);
	thisBipedInfo.isSentinel = thisBipedInfo.getShortName().contains("sentinel");
	return thisBipedInfo;
}


void EnemyRandomiser::evaluateActors()
{
	assert(OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue());
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
	assert(actorDatumVector.size() == actorMap.size());
	PLOG_DEBUG << "Checking if randomiser enabled";
	if (OptionsState::EnemyRandomiser.GetValue())
	{
		PLOG_DEBUG << "Iterating randomisation rules";
		// Iterate over all actors, and for each one construct their rollDistribution
		for (auto& [mDatum, actor] : actorMap)
		{
			if (!actor.isValidUnit) continue;

			EnemyGroup* rollGroup = nullptr;
			// Evaluate rules (in reverse, so rules at top of GUI overwrite ones at bottom)
			for (auto& rule : std::ranges::views::reverse(OptionsState::currentRandomiserRules))
			{
				PLOG_DEBUG << "Actor checking if a rule applies to it: " << actor.getShortName();
				if (rule.get()->getType() == RuleType::RandomiseXintoY)
				{
					RandomiseXintoY* thisRule = dynamic_cast<RandomiseXintoY*>(rule.get());
					if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

					if (thisRule->randomiseGroupSelection.isMatch(actor))
					{
						PLOG_DEBUG << "Actor found a matching rule: " << actor.getShortName();
						actor.probabilityOfRandomize = thisRule->randomisePercent.GetValue() / 100.;
						rollGroup = &thisRule->rollPoolGroupSelection;
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
	}
	
	PLOG_DEBUG << "Checking if spawn multiplication enabled";
	if (OptionsState::EnemySpawnMultiplier.GetValue())
	{
		PLOG_DEBUG << "Iterating spawn multiplication rules";
		// Iterate over all actors, and for each one set their pre and post-randomisation spawn multiplier
		for (auto& [mDatum, actor] : actorMap)
		{
			if (!actor.isValidUnit) continue;

			// Evaluate rules (in reverse, so rules at top of GUI overwrite ones at bottom)
			for (auto& rule : std::ranges::views::reverse(OptionsState::currentMultiplierRules))
			{
				PLOG_DEBUG << "Actor checking if a rule applies to it: " << actor.getShortName();
				switch (rule.get()->getType())
				{
				case RuleType::SpawnMultiplierPreRando:
				{
					SpawnMultiplierBeforeRando* thisRule = dynamic_cast<SpawnMultiplierBeforeRando*>(rule.get());
					if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

					if (thisRule->groupSelection.isMatch(actor))
					{
						// multiply instead of set, so multiple rules applying to the same group have a multiplicative effect
						actor.spawnMultiplierPreRando *= thisRule->multiplier.GetValue();
					}
				}
				break;

				case RuleType::SpawnMultiplierPostRando:
				{
					SpawnMultiplierAfterRando* thisRule = dynamic_cast<SpawnMultiplierAfterRando*>(rule.get());
					if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

					if (thisRule->groupSelection.isMatch(actor))
					{
						// multiply instead of set, so multiple rules applying to the same group have a multiplicative effect
						actor.spawnMultiplierPostRando *= thisRule->multiplier.GetValue();
					}
				}
				break;

				default:
					break;
				}

			}
		}
	}
	


}


void EnemyRandomiser::evaluateBipeds()
{
	assert(OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue());
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
	assert(bipedDatumVector.size() == bipedMap.size());
	
	if (OptionsState::EnemyRandomiser.GetValue())
	{
		// Iterate over all bipeds, and for each one construct their rollDistribution
		for (auto& [mDatum, biped] : bipedMap)
		{
			if (!biped.isValidUnit) continue;

			EnemyGroup* rollGroup = nullptr;
			// Evaluate rules (in reverse, so rules at top of GUI overwrite ones at bottom)
			for (auto& rule : std::ranges::views::reverse(OptionsState::currentRandomiserRules))
			{
				if (rule.get()->getType() == RuleType::RandomiseXintoY)
				{
					RandomiseXintoY* thisRule = dynamic_cast<RandomiseXintoY*>(rule.get());
					if (thisRule == nullptr) throw CEERRuntimeException("failed to cast rule!");

					if (thisRule->randomiseGroupSelection.isMatch(biped))
					{
						biped.probabilityOfRandomize = thisRule->randomisePercent.GetValue() / 100.;
						rollGroup = &thisRule->rollPoolGroupSelection;
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
	}

	if (OptionsState::EnemySpawnMultiplier.GetValue())
	{
		//TODO (not really a big deal tho)
	}
	

}






void EnemyRandomiser::onLevelLoadEvent(HaloLevel newLevel)
{
	if (OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue())
	{

		try
		{
			MessagesGUI::addMessage("Loading unit data...");
			PLOG_DEBUG << "Loading unit data...";
			std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

			instance->mapReader->cacheTagData(newLevel);

			instance->evaluateActors();

			instance->evaluateBipeds();

			MessagesGUI::addMessage(std::format("Success! {} actor{}, {} biped{}",
				instance->actorMap.size(),
				instance->actorMap.size() == 1 ? "" : "s",
				instance->bipedMap.size(),
				instance->bipedMap.size() == 1 ? "" : "s"
			));

		}
		catch (CEERRuntimeException& ex)
		{
			PLOG_ERROR << "exception in EnemyRandomiser onLevelLoadEvent: " << ex.what();
			RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier }); // tell user, disable options
		}
		catch (...) // MCC is probably about to imminently crash, let's see if we can find out what went wrong tho
		{
			CEERRuntimeException ex(ResurrectException());
			PLOG_FATAL << "unhandled exception in enemyRandomiser onLevelLoadEvent: " << ex.what();
			RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::EnemyRandomiser, &OptionsState::EnemySpawnMultiplier }); // tell user, disable options
		}
	}
}



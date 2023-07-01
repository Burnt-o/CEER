#include "pch.h"
#include "SoundRandomiser.h"
#include "UserSeed.h"
#include "LevelLoadHook.h"
#include "MessagesGUI.h"

SoundRandomiser* SoundRandomiser::instance = nullptr;

void SoundRandomiser::lazyInit()
{
	PLOG_DEBUG << "SoundRandomiser::lazyInit()";
	// Set up our hooks and get our pointers
	try
	{
		auto loadSoundAFunction = PointerManager::getMultilevelPointer("loadSoundAFunction");
		loadSoundAFunctionContext = PointerManager::getMidhookContextInterpreter("loadSoundAFunctionContext");
		loadSoundAHook = ModuleMidHook::make(L"halo1.dll", loadSoundAFunction, loadSoundAHookFunction, false);


	}
	catch (InitException& ex)
	{
		PLOG_ERROR << "E";
		ex.prepend("SoundRandomiser could not resolve hooks: ");
		throw ex;
	}
}
std::mutex soundToggleChangeMutex;
void SoundRandomiser::onSoundRandomiserToggleChange(bool& newValue)
{

	std::scoped_lock<std::mutex> lock(soundToggleChangeMutex);
	//lazy init (getting pointerData, creating hook objects)
	try
	{
		std::call_once(instance->lazyInitOnceFlag, []() {instance->lazyInit(); }); // flag not flipped if exception thrown
	}
	catch (InitException& ex)
	{
		PLOG_ERROR << "EEEEEEE";
		RuntimeExceptionHandler::handleMessage(ex);
		OptionsState::SoundRandomiser.GetValue() = false;
		OptionsState::SoundRandomiser.GetValueDisplay() = false;
		return;
	}

	if (newValue)
	{


		// check if a level is already loaded, if so call onLevelLoad
		HaloLevel currentLevel;
		if (LevelLoadHook::isLevelAlreadyLoaded(currentLevel))
		{
			onLevelLoadEvent(currentLevel);
		}

	}

	// set hook state
	instance->loadSoundAHook.get()->setWantsToBeAttached(newValue);


}

void SoundRandomiser::onLevelLoadEvent(HaloLevel newLevel)
{
	// Get rng seed
	instance->ourSeed = UserSeed::GetCurrentSeed();

	if (OptionsState::SoundRandomiser.GetValue())
	{
		try
		{
			MessagesGUI::addMessage("Loading sound data...");
			PLOG_DEBUG << "Loading sound data...";
			std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

			instance->mapReader->cacheTagData(newLevel);

			instance->evaluateSounds();

			MessagesGUI::addMessage(std::format("Success! {} sounds{}",
				instance->soundMap.size(),
				instance->soundMap.size() == 1 ? "" : "s"
			));

		}
		catch (CEERRuntimeException& ex)
		{
			PLOG_ERROR << "exception in SoundRandomiser onLevelLoadEvent: " << ex.what();
			RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::SoundRandomiser }); // tell user, disable options
		}
		catch (...) // MCC is probably about to imminently crash, let's see if we can find out what went wrong tho
		{
			CEERRuntimeException ex(ResurrectException());
			PLOG_FATAL << "unhandled exception in SoundRandomiser onLevelLoadEvent: " << ex.what();
			RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::SoundRandomiser }); // tell user, disable options
		}
	}
}





void SoundRandomiser::evaluateSounds()
{
	assert(OptionsState::SoundRandomiser.GetValue());
	soundMap.clear();
	shuffledSounds.clear();

	auto tagTable = instance->mapReader->getTagTable();		PLOG_DEBUG << "TagTable count: " << tagTable.size();
	constexpr auto bitmMagic = MapReader::stringToMagic("snd!");

	// Get info on each of the sounds in the tag table
	for (const auto& tag : tagTable)
	{
		if (tag.tagGroupMagic != bitmMagic || tag.tagDatum == nullDatum) continue;
		auto sound = readSoundInfo(tag);
		soundMap.emplace(tag.tagDatum, sound);
	}




	// Then need to parse optionState for what sounds are in the pool
	// and what % of sounds to randomise
	// and construct a soundPool for each category




	// construct a sound pool for each category;
	std::pair<SoundCategory, std::vector<datum>> soundPool_Dialog{ SoundCategory::Dialog, {} };
	std::pair<SoundCategory, std::vector<datum>> soundPool_Music{ SoundCategory::Music, {} };
	std::pair<SoundCategory, std::vector<datum>> soundPool_Animations{ SoundCategory::Animations, {} };
	std::pair<SoundCategory, std::vector<datum>> soundPool_Effects{ SoundCategory::Effects, {} };
	std::pair<SoundCategory, std::vector<datum>>soundPool_WeapVehi{ SoundCategory::WeapVehi, {} };
	std::map<SoundCategory, std::vector<datum>> allSoundPools{ soundPool_Dialog, soundPool_Music, soundPool_Animations, soundPool_Effects, soundPool_WeapVehi };

	std::map<SoundCategory, bool> categoryIsEnabled{
		{SoundCategory::Invalid, false},
		{SoundCategory::Dialog, OptionsState::SoundIncludeDialog.GetValue()},
		{SoundCategory::Music, OptionsState::SoundIncludeMusic.GetValue()},
		{SoundCategory::Animations, OptionsState::SoundIncludeAnimations.GetValue()},
		{SoundCategory::Effects, OptionsState::SoundIncludeEffects.GetValue()},
		{SoundCategory::WeapVehi, OptionsState::SoundIncludeWeapVehi.GetValue()},
	};

	bool restrictedSounds = OptionsState::SoundRestrictToCategory.GetValue();
	// Fill up the sound pools
	for (auto& [thisSoundDatum, thissoundInfo] : soundMap)
	{

		// emplace each sound into each soundPool if it's category option is enabled. Only goes to all soundpools if restrictedSounds is off, otherwise only goes to it's matching category
		if (categoryIsEnabled.at(thissoundInfo.category)) // this check will exclude SoundCategory::Invalid from being placed into any pools
		{
			for (auto& [soundPoolCategory, soundPool] : allSoundPools)
			{
				if (restrictedSounds)
				{
					if (thissoundInfo.category == soundPoolCategory)
					{
						soundPool.emplace_back(thisSoundDatum);
						break; // don't need to check the other sound pools
					}
				}
				else
				{
					soundPool.emplace_back(thisSoundDatum); // put it in all the sound pools!
				}
			}
		}
	}

	// log how many sounds found in each category
	for (auto& [soundPoolCategory, soundPool] : allSoundPools)
	{
		PLOG_DEBUG << std::format("Sound category {} contains {} sounds", soundCategoryToString.at(soundPoolCategory), soundPool.size());
	}

	// construct roll distribution from each sound pool (makes random selection easier and less biased)
	std::pair<SoundCategory, std::discrete_distribution<int>> rollDistribution_Dialog{ SoundCategory::Dialog, {} };
	std::pair<SoundCategory, std::discrete_distribution<int>> rollDistribution_Music{ SoundCategory::Music, {} };
	std::pair<SoundCategory, std::discrete_distribution<int>> rollDistribution_Animations{ SoundCategory::Animations, {} };
	std::pair<SoundCategory, std::discrete_distribution<int>> rollDistribution_Effects{ SoundCategory::Effects, {} };
	std::pair<SoundCategory, std::discrete_distribution<int>> rollDistribution_WeapVehi{ SoundCategory::WeapVehi, {} };
	std::map<SoundCategory, std::discrete_distribution<int>> allRollDistributions{ rollDistribution_Dialog, rollDistribution_Music, rollDistribution_Animations, rollDistribution_Effects, rollDistribution_WeapVehi };
	for (auto& [soundCategory, rollDist] : allRollDistributions)
	{
		std::vector<double> weights(allSoundPools.at(soundCategory).size(), 1.0);
		rollDist.param(std::discrete_distribution<int>::param_type(weights.begin(), weights.end()));
	}

	// Setup the shuffled sounds
	SetSeed64 gen(ourSeed); // will increment this as we process each sound so we get a different random number each time
	double randomiseChance = OptionsState::SoundRandomiserPercent.GetValue() / 100.;
	for (auto& [thisSoundDatum, thisSoundInfo] : soundMap)
	{
		++gen;
		if (!categoryIsEnabled.at(thisSoundInfo.category))
		{
			shuffledSounds.emplace(thisSoundDatum, thisSoundDatum); // sound is mapped to itself, ie no randomisation
		}
		else
		{
			if (zeroToOne(gen) <= randomiseChance) // roll randomise chance
			{ // roll success!
				// Now to decide which sound

				// use roll Distribution to get which index of the soundPool to get the datum from
				int randomSoundIndex = allRollDistributions.at(thisSoundInfo.category)(gen);
				// get the datum and emplace it into shuffled sounds
				shuffledSounds.emplace(thisSoundDatum, allSoundPools.at(thisSoundInfo.category).at(randomSoundIndex));
			}
			else
			{ // roll failure, no randomisation
				shuffledSounds.emplace(thisSoundDatum, thisSoundDatum); // sound is mapped to itself, ie no randomisation
			}

		}
	}

	assert(shuffledSounds.size() == soundMap.size());

}


SoundInfo SoundRandomiser::readSoundInfo(const tagElement& soundTag)
{
	SoundInfo thisSoundInfo(instance->mapReader->getTagName(soundTag.tagDatum));
	thisSoundInfo.dat = soundTag.tagDatum;
	thisSoundInfo.memoryOffset = soundTag.offset;
	thisSoundInfo.category = sortIntoCategory(thisSoundInfo);
	thisSoundInfo.nameMemoryOffset = soundTag.nameOffset;
	return thisSoundInfo;
}



SoundCategory SoundRandomiser::sortIntoCategory(const SoundInfo& soundInfo)
{
	PLOG_VERBOSE << soundInfo.getFullName();

	if (soundInfo.getFullName().contains("\\dialog\\")
		) return SoundCategory::Dialog;


	if (soundInfo.getFullName().contains("\\music\\")
		) return SoundCategory::Music;


	if (soundInfo.getFullName().contains("vehicles")
		|| soundInfo.getFullName().contains("weapon")
		|| soundInfo.getFullName().contains("melee")
		|| soundInfo.getFullName().contains("impacts")
		|| soundInfo.getFullName().contains("shellcasings")
		) return SoundCategory::WeapVehi;

	if (soundInfo.getFullName().contains("\\ui\\")
		|| soundInfo.getFullName().contains("\\ambience\\")
		|| soundInfo.getFullName().contains("\\panel\\")
		|| soundInfo.getFullName().contains("\\coolant\\")
		|| soundInfo.getFullName().contains("\\door")
		|| soundInfo.getFullName().contains("\\sinomatix")
		|| soundInfo.getFullName().contains("\\glass")
		) return SoundCategory::Effects;


	if (soundInfo.getFullName().contains("\\impulse\\") // some of the impulse stuff gets caught by weapVehi and effects. Important that this check is after those.
		) return SoundCategory::Animations;



	// fall-through
	return SoundCategory::Invalid;
}
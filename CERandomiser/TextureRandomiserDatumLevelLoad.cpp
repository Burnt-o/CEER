#include "pch.h"
#include "TextureRandomiserDatum.h"
#include "UserSeed.h"
#include "LevelLoadHook.h"
#include "MessagesGUI.h"

TextureRandomiserDatum* TextureRandomiserDatum::instance = nullptr;

bool TextureRandomiserDatum::SMseizureModeEnabled = false;
std::map<TextureCategory, bool> TextureRandomiserDatum::SMcategoryIsEnabled{};
std::map<TextureCategory, std::vector<datum>> TextureRandomiserDatum::SMallTexturePools{};
std::mt19937 TextureRandomiserDatum::SMseizureRNG; // generator needed to grab numbers from the int distributions
std::uniform_int_distribution<> TextureRandomiserDatum::SMseizureWillRandomiseRNG{ 0, 100 }; // used to determine whether a texture will be re-randomised on any given frame
std::uniform_int_distribution<> TextureRandomiserDatum::SMseizureWhatRandomiseRNG{ 0, 100 }; // if re-randomised, used to select which texture to rando into 


void TextureRandomiserDatum::lazyInit()
{
	PLOG_DEBUG << "TextureRandomiser::lazyInit()";
	// Set up our hooks and get our pointers
	try
	{
		auto loadRegularTextureDatumFunction = PointerManager::getMultilevelPointer("loadRegularTextureDatumFunction");
		loadRegularTextureDatumFunctionContext = PointerManager::getMidhookContextInterpreter("loadRegularTextureDatumFunctionContext");
		loadRegularTextureDatumHook = ModuleMidHook::make(L"halo1.dll", loadRegularTextureDatumFunction, loadRegularTextureDatumHookFunction, false);

		auto loadDecalTextureDatumFunction = PointerManager::getMultilevelPointer("loadDecalTextureDatumFunction");
		loadDecalTextureDatumFunctionContext = PointerManager::getMidhookContextInterpreter("loadDecalTextureDatumFunctionContext");
		loadDecalTextureDatumHook = ModuleMidHook::make(L"halo1.dll", loadDecalTextureDatumFunction, loadDecalTextureDatumHookFunction, false);

		auto loadParticleTextureDatumFunction = PointerManager::getMultilevelPointer("loadParticleTextureDatumFunction");
		loadParticleTextureDatumFunctionContext = PointerManager::getMidhookContextInterpreter("loadParticleTextureDatumFunctionContext");
		loadParticleTextureDatumHook = ModuleMidHook::make(L"halo1.dll", loadParticleTextureDatumFunction, loadParticleTextureDatumHookFunction, false);

		auto loadContrailTextureDatumFunction = PointerManager::getMultilevelPointer("loadContrailTextureDatumFunction");
		loadContrailTextureDatumFunctionContext = PointerManager::getMidhookContextInterpreter("loadContrailTextureDatumFunctionContext");
		loadContrailTextureDatumHook = ModuleMidHook::make(L"halo1.dll", loadContrailTextureDatumFunction, loadContrailTextureDatumHookFunction, false);

		auto loadLensTextureDatumFunction = PointerManager::getMultilevelPointer("loadLensTextureDatumFunction");
		loadLensTextureDatumFunctionContext = PointerManager::getMidhookContextInterpreter("loadLensTextureDatumFunctionContext");
		loadLensTextureDatumHook = ModuleMidHook::make(L"halo1.dll", loadLensTextureDatumFunction, loadLensTextureDatumHookFunction, false);

		try
		{
			auto loadHudTextureDatumFunction = PointerManager::getMultilevelPointer("loadHudTextureDatumFunction");
			loadHudTextureDatumFunctionContext = PointerManager::getMidhookContextInterpreter("loadHudTextureDatumFunctionContext");
			loadHudTextureDatumHook = ModuleMidHook::make(L"halo1.dll", loadLensTextureDatumFunction, loadHudTextureDatumHookFunction, false);
		}
		catch (InitException& ex)
		{
			PLOG_ERROR << "Could not find loadHudTexture hooks";
			hudTexAvailable = false;
		}

	}
	catch (InitException& ex)
	{
		PLOG_ERROR << "E";
		ex.prepend("TextureRandomiser could not resolve hooks: ");
		throw ex;
	}
}
std::mutex textureDatumToggleChangeMutex;
void TextureRandomiserDatum::onTextureRandomiserToggleChange(bool& newValue)
{
	

	std::scoped_lock<std::mutex> lock(textureDatumToggleChangeMutex);
	//lazy init (getting pointerData, creating hook objects)
	try
	{
		std::call_once(instance->lazyInitOnceFlag, []() {instance->lazyInit(); }); // flag not flipped if exception thrown
	}
	catch (InitException& ex)
	{
		PLOG_ERROR << "EEEEEEE";
		RuntimeExceptionHandler::handleMessage(ex);
		OptionsState::TextureRandomiser.GetValue() = false;
		OptionsState::TextureRandomiser.GetValueDisplay() = false;
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

	PLOG_DEBUG << "setting hook state to " << newValue;

	// set hook state
	instance->loadRegularTextureDatumHook.get()->setWantsToBeAttached(newValue);
	instance->loadDecalTextureDatumHook.get()->setWantsToBeAttached(newValue);
	instance->loadParticleTextureDatumHook.get()->setWantsToBeAttached(newValue);
	instance->loadContrailTextureDatumHook.get()->setWantsToBeAttached(newValue);
	instance->loadLensTextureDatumHook.get()->setWantsToBeAttached(newValue);
	if (instance->hudTexAvailable)
	{
		PLOG_DEBUG << "hudTextAvailable, setting " << newValue;
		instance->loadHudTextureDatumHook.get()->setWantsToBeAttached(newValue);
	}
	else
	{
		PLOG_DEBUG << "hudTextAvailable is FALSE, setting " << newValue;
	}


}

void TextureRandomiserDatum::onLevelLoadEvent(HaloLevel newLevel)
{
	// Get rng seed
	instance->ourSeed = UserSeed::GetCurrentSeed();

	if (OptionsState::TextureRandomiser.GetValue())
	{
		try
		{
			MessagesGUI::addMessage("Loading texture data...");
			PLOG_DEBUG << "Loading texture data...";
			PLOG_VERBOSE << "locking TextureRandomiser::mDestructionGuard";
			std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

			instance->mapReader->cacheTagData(newLevel);

			instance->evaluateTextures();

			MessagesGUI::addMessage(std::format("Success! {} textures{}",
				instance->textureMap.size(),
				instance->textureMap.size() == 1 ? "" : "s"
			));

		}
		catch (CEERRuntimeException& ex)
		{
			PLOG_ERROR << "exception in TextureRandomiser onLevelLoadEvent: " << ex.what();
			RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::TextureRandomiser}); // tell user, disable options
		}
		catch (...) // MCC is probably about to imminently crash, let's see if we can find out what went wrong tho
		{
			CEERRuntimeException ex(ResurrectException());
			PLOG_FATAL << "unhandled exception in TextureRandomiser onLevelLoadEvent: " << ex.what();
			RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::TextureRandomiser }); // tell user, disable options
		}
	}
	PLOG_VERBOSE << "unlocking TextureRandomiser::mDestructionGuard";
}





void TextureRandomiserDatum::evaluateTextures()
{
	assert(OptionsState::TextureRandomiser.GetValue());
	textureMap.clear();
	shuffledTextures.clear();
	seizureModeVector.clear();

	auto tagTable = instance->mapReader->getTagTable();		PLOG_DEBUG << "TagTable count: " << tagTable.size();
	constexpr auto bitmMagic = MapReader::stringToMagic("bitm");

	// Get info on each of the textures in the tag table
	for (const auto& tag : tagTable)
	{
		if (tag.tagGroupMagic != bitmMagic || tag.tagDatum == nullDatum) continue;
		auto texture = readTextureInfo(tag);
		textureMap.emplace(tag.tagDatum, texture);
		seizureModeVector.emplace_back(tag.tagDatum);

	}

	// Then need to parse optionState for what textures are in the pool
	// and what % of textures to randomise
	// and construct a texturePool for each category




	// construct a texture pool for each category;
	std::pair<TextureCategory, std::vector<datum>> texturePool_Character{ TextureCategory::Character, {} };
	std::pair<TextureCategory, std::vector<datum>> texturePool_WeapVehi{ TextureCategory::WeapVehi, {} };
	std::pair<TextureCategory, std::vector<datum>> texturePool_Effect{ TextureCategory::Effect, {} };
	std::pair<TextureCategory, std::vector<datum>> texturePool_Level{ TextureCategory::Level, {} };
	std::pair<TextureCategory, std::vector<datum>>texturePool_UI{ TextureCategory::UI, {} };
	std::map<TextureCategory, std::vector<datum>> allTexturePools{ texturePool_Character, texturePool_WeapVehi, texturePool_Effect, texturePool_Level, texturePool_UI };

	std::map<TextureCategory, bool> categoryIsEnabled{ 
		{TextureCategory::Invalid, false},
		{TextureCategory::Character, OptionsState::TextureIncludeCharacter.GetValue()}, 
		{TextureCategory::WeapVehi, OptionsState::TextureIncludeWeapVehi.GetValue()},
		{TextureCategory::Effect, OptionsState::TextureIncludeEffect.GetValue()},
		{TextureCategory::Level, OptionsState::TextureIncludeLevel.GetValue()},
		{TextureCategory::UI, hudTexAvailable == false ? false : OptionsState::TextureIncludeUI.GetValue()},
	};

	bool restrictedTextures = OptionsState::TextureRestrictToCategory.GetValue();
	// Fill up the texture pools
	for (auto& [thistextureDatum, thistextureInfo] : textureMap)
	{

		// emplace each texture into each texturePool if it's category option is enabled. Only goes to all texturepools if restrictedTextures is off, otherwise only goes to it's matching category
		if (categoryIsEnabled.at(thistextureInfo.category)) // this check will exclude TextureCategory::Invalid from being placed into any pools
		{
			for (auto& [texturePoolCategory, texturePool] : allTexturePools)
			{
				if (restrictedTextures)
				{
					if (thistextureInfo.category == texturePoolCategory)
					{
						texturePool.emplace_back(thistextureDatum);
						break; // don't need to check the other texture pools
					}
				}
				else
				{
					texturePool.emplace_back(thistextureDatum); // put it in all the texture pools!
				}
			}
		}	
	}

	// log how many textures found in each category
	size_t largestTexturePoolSize = 0; // this value is also used by seizureMode
	for (auto& [texturePoolCategory, texturePool] : allTexturePools)
	{

		largestTexturePoolSize = std::max(largestTexturePoolSize, texturePool.size());
		PLOG_DEBUG << std::format("Texture category {} contains {} textures", textureCategoryToString.at(texturePoolCategory), texturePool.size());
	}

	// construct roll distribution from each texture pool (makes random selection easier and less biased)
	std::pair<TextureCategory, std::discrete_distribution<int>> rollDistribution_Character{ TextureCategory::Character, {} };
	std::pair<TextureCategory, std::discrete_distribution<int>> rollDistribution_WeapVehi{ TextureCategory::WeapVehi, {} };
	std::pair<TextureCategory, std::discrete_distribution<int>> rollDistribution_Effect{ TextureCategory::Effect, {} };
	std::pair<TextureCategory, std::discrete_distribution<int>> rollDistribution_Level{ TextureCategory::Level, {} };
	std::pair<TextureCategory, std::discrete_distribution<int>> rollDistribution_UI{ TextureCategory::UI, {} };
	std::map<TextureCategory, std::discrete_distribution<int>> allRollDistributions{ rollDistribution_Character, rollDistribution_WeapVehi, rollDistribution_Effect, rollDistribution_Level, rollDistribution_UI };
	for (auto& [textureCategory, rollDist] : allRollDistributions)
	{
		std::vector<double> weights(allTexturePools.at(textureCategory).size(), 1.0);
		rollDist.param(std::discrete_distribution<int>::param_type(weights.begin(), weights.end()));
	}

	// Setup the shuffled textures
	SetSeed64 gen(ourSeed); // will increment this as we process each texture so we get a different random number each time
	std::uniform_real_distribution<double> zeroToOne{ 0.0, 1.0 }; // to roll against % of textures to randomise
	double randomiseChance = OptionsState::TextureRandomiserPercent.GetValue() / 100.;
	for (auto& [thisTextureDatum, thisTextureInfo] : textureMap)
	{
		++gen;
		if (!categoryIsEnabled.at(thisTextureInfo.category))
		{
			shuffledTextures.emplace(thisTextureDatum, thisTextureDatum); // texture is mapped to itself, ie no randomisation
		}
		else
		{
			if (zeroToOne(gen) <= randomiseChance) // roll randomise chance
			{ // roll success!
				// Now to decide which texture

				// use roll Distribution to get which index of the texturePool to get the memoffset from
				int randomTextureIndex = allRollDistributions.at(thisTextureInfo.category)(gen);
				// get the memOffset and emplace it into shuffled textures
				shuffledTextures.emplace(thisTextureDatum, allTexturePools.at(thisTextureInfo.category).at(randomTextureIndex));
			}
			else
			{ // roll failure, no randomisation
				shuffledTextures.emplace(thisTextureDatum, thisTextureDatum); // texture is mapped to itself, ie no randomisation
			}

		}
	}

	assert(shuffledTextures.size() == textureMap.size());

	// setup seizureMode stuff

	if (OptionsState::TextureSeizureMode.GetValue())
	{
		PLOG_VERBOSE << "Updating SeizureSettings";
		SMcategoryIsEnabled = categoryIsEnabled;
		SMallTexturePools = allTexturePools;


		using param_t = std::uniform_int_distribution<>::param_type;
		SMseizureWillRandomiseRNG.param(param_t(0, OptionsState::TextureFramesBetweenSeizures.GetValue() - 1));
		SMseizureWhatRandomiseRNG.param(param_t(0, (int)largestTexturePoolSize - 1));

	}
	SMseizureModeEnabled = OptionsState::TextureSeizureMode.GetValue(); // important that this is set AFTER settings set, so hooks don't access that data before it's ready
	PLOG_DEBUG << "SeizureSettings::seizureModeEnabled " << SMseizureModeEnabled;
	PLOG_DEBUG << "OptionsState::TextureSeizureMode.GetValue() " << OptionsState::TextureSeizureMode.GetValue();
}


TextureInfo TextureRandomiserDatum::readTextureInfo(const tagElement& textureTag)
{
	TextureInfo thisTextureInfo(instance->mapReader->getTagName(textureTag.tagDatum));
	thisTextureInfo.dat = textureTag.tagDatum;
	thisTextureInfo.memoryOffset = textureTag.offset;
	thisTextureInfo.category = sortIntoCategory(thisTextureInfo);
	return thisTextureInfo;
}



TextureCategory TextureRandomiserDatum::sortIntoCategory(const TextureInfo& textureInfo)
{
	PLOG_VERBOSE << textureInfo.getFullName();
	if (textureInfo.getFullName().starts_with("rasterizer")
		|| textureInfo.getFullName().contains("angle_ticks_black") // sniper hud element that looks dumb if rando'd
		) return TextureCategory::Invalid;
		

	if (textureInfo.getFullName().starts_with("character")
		) return TextureCategory::Character;
		

	if (textureInfo.getFullName().starts_with("weapons")
		|| textureInfo.getFullName().starts_with("powerups")
		|| textureInfo.getFullName().starts_with("vehicles")
		) return TextureCategory::WeapVehi;
		

	if (textureInfo.getFullName().starts_with("effects")
		) return TextureCategory::Effect;
		

	if (textureInfo.getFullName().starts_with("scenery")
		|| textureInfo.getFullName().starts_with("sky")
		|| textureInfo.getFullName().starts_with("levels")
		) return TextureCategory::Level;
		

	if (textureInfo.getFullName().starts_with("ui")
		) return TextureCategory::UI;
		
	// fall-through
	return TextureCategory::Invalid;
}
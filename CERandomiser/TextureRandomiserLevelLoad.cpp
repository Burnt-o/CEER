#include "pch.h"
#include "TextureRandomiser.h"
#include "UserSeed.h"
#include "LevelLoadHook.h"
#include "MessagesGUI.h"

TextureRandomiser* TextureRandomiser::instance = nullptr;

void TextureRandomiser::lazyInit()
{
	PLOG_DEBUG << "TextureRandomiser::lazyInit()";
	// Set up our hooks and get our pointers
	try
	{
		auto loadRegularTextureFunction = PointerManager::getMultilevelPointer("loadRegularTextureFunction");
		loadRegularTextureFunctionContext = PointerManager::getMidhookContextInterpreter("loadRegularTextureFunctionContext");
		loadRegularTextureHook = ModuleMidHook::make(L"halo1.dll", loadRegularTextureFunction, loadRegularTextureHookFunction, false);

		auto loadDecalTextureFunction = PointerManager::getMultilevelPointer("loadDecalTextureFunction");
		loadDecalTextureFunctionContext = PointerManager::getMidhookContextInterpreter("loadDecalTextureFunctionContext");
		loadDecalTextureHook = ModuleMidHook::make(L"halo1.dll", loadDecalTextureFunction, loadDecalTextureHookFunction, false);

		auto loadParticleTextureFunction = PointerManager::getMultilevelPointer("loadParticleTextureFunction");
		loadParticleTextureFunctionContext = PointerManager::getMidhookContextInterpreter("loadParticleTextureFunctionContext");
		loadParticleTextureHook = ModuleMidHook::make(L"halo1.dll", loadParticleTextureFunction, loadParticleTextureHookFunction, false);

		auto loadContrailTextureFunction = PointerManager::getMultilevelPointer("loadContrailTextureFunction");
		loadContrailTextureFunctionContext = PointerManager::getMidhookContextInterpreter("loadContrailTextureFunctionContext");
		loadContrailTextureHook = ModuleMidHook::make(L"halo1.dll", loadContrailTextureFunction, loadContrailTextureHookFunction, false);


		auto loadLensTextureFunction = PointerManager::getMultilevelPointer("loadLensTextureFunction");
		loadLensTextureFunctionContext = PointerManager::getMidhookContextInterpreter("loadLensTextureFunctionContext");
		loadLensTextureHook = ModuleMidHook::make(L"halo1.dll", loadLensTextureFunction, loadLensTextureHookFunction, false);

	}
	catch (InitException& ex)
	{
		PLOG_ERROR << "E";
		ex.prepend("TextureRandomiser could not resolve hooks: ");
		throw ex;
	}
}

void TextureRandomiser::onTextureRandomiserToggleChange(bool& newValue)
{
	//lazy init (getting pointerData, creating hook objects)
	try
	{
		std::call_once(instance->lazyInitOnceFlag, []() {instance->lazyInit(); }); // flag not flipped if exception thrown
	}
	catch (InitException& ex)
	{
		PLOG_ERROR << "EEEEEEE";
		RuntimeExceptionHandler::handleMessage(ex, { &OptionsState::TextureRandomiser });
		return;
	}

	if (newValue)
	{
		// Get rng seed
		instance->ourSeed = UserSeed::GetCurrentSeed();

		// check if a level is already loaded, if so call onLevelLoad
		HaloLevel currentLevel;
		if (LevelLoadHook::isLevelAlreadyLoaded(currentLevel))
		{
			onLevelLoadEvent(currentLevel);
		}

	}

	// set hook state
	instance->loadRegularTextureHook.get()->setWantsToBeAttached(newValue);
	instance->loadDecalTextureHook.get()->setWantsToBeAttached(newValue);

	instance->loadParticleTextureHook.get()->setWantsToBeAttached(newValue);
	instance->loadContrailTextureHook.get()->setWantsToBeAttached(newValue);
	instance->loadLensTextureHook.get()->setWantsToBeAttached(newValue);
}

void TextureRandomiser::onLevelLoadEvent(HaloLevel newLevel)
{
	if (OptionsState::TextureRandomiser.GetValue())
	{
		try
		{
			MessagesGUI::addMessage("Loading texture data...");
			PLOG_DEBUG << "Loading texture data...";
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
}





void TextureRandomiser::evaluateTextures()
{
	assert(OptionsState::TextureRandomiser.GetValue());
	textureMap.clear();
	textureVector.clear();
	shuffledTextures.clear();

	auto tagTable = instance->mapReader->getTagTable();		PLOG_DEBUG << "TagTable count: " << tagTable.size();
	constexpr auto bitmMagic = MapReader::stringToMagic("bitm");

	// Get info on each of the textures in the tag table
	for (const auto& tag : tagTable)
	{
		if (tag.tagGroupMagic != bitmMagic || tag.tagDatum == nullDatum) continue;
		auto texture = readTextureInfo(tag);
		textureMap.emplace(tag.offset, texture);
		textureVector.emplace_back(texture);
	}
	assert(textureMap.size() == textureVector.size());

	// Then need to parse optionState for what textures are in the pool
	// and what % of textures to randomise
	// and construct a texturePool for each category




	// construct a texture pool for each category;
	std::pair<TextureCategory, std::vector<MemOffset>> texturePool_Character{ TextureCategory::Character, {} };
	std::pair<TextureCategory, std::vector<MemOffset>> texturePool_WeapVehi{ TextureCategory::WeapVehi, {} };
	std::pair<TextureCategory, std::vector<MemOffset>> texturePool_Effect{ TextureCategory::Effect, {} };
	std::pair<TextureCategory, std::vector<MemOffset>> texturePool_Level{ TextureCategory::Level, {} };
	std::pair<TextureCategory, std::vector<MemOffset>>texturePool_UI{ TextureCategory::UI, {} };
	std::map<TextureCategory, std::vector<MemOffset>> allTexturePools{ texturePool_Character, texturePool_WeapVehi, texturePool_Effect, texturePool_Level, texturePool_UI };

	std::map<TextureCategory, bool> categoryIsEnabled{ 
		{TextureCategory::Invalid, false},
		{TextureCategory::Character, OptionsState::TextureIncludeCharacter.GetValue()}, 
		{TextureCategory::WeapVehi, OptionsState::TextureIncludeWeapVehi.GetValue()},
		{TextureCategory::Effect, OptionsState::TextureIncludeEffect.GetValue()},
		{TextureCategory::Level, OptionsState::TextureIncludeLevel.GetValue()},
		{TextureCategory::UI, OptionsState::TextureIncludeUI.GetValue()},
	};

	bool restrictedTextures = OptionsState::TextureRestrictToCategory.GetValue();
	// Fill up the texture pools
	for (auto& [thistextureOffset, thistextureInfo] : textureMap)
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
						texturePool.emplace_back(thistextureOffset);
						break; // don't need to check the other texture pools
					}
				}
				else
				{
					texturePool.emplace_back(thistextureOffset); // put it in all the texture pools!
				}
			}
		}	
	}

	// log how many textures found in each category
	for (auto& [texturePoolCategory, texturePool] : allTexturePools)
	{
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
	double randomiseChance = OptionsState::TextureRandomiserPercent.GetValue();
	for (auto& [thisTextureOffset, thisTextureInfo] : textureMap)
	{
		++gen;
		if (!categoryIsEnabled.at(thisTextureInfo.category))
		{
			shuffledTextures.emplace(thisTextureOffset, thisTextureOffset); // texture is mapped to itself, ie no randomisation
		}
		else
		{
			if (zeroToOne(gen) <= randomiseChance) // roll randomise chance
			{ // roll success!
				// Now to decide which texture

				// use roll Distribution to get which index of the texturePool to get the memoffset from
				int randomTextureIndex = allRollDistributions.at(thisTextureInfo.category)(gen);
				// get the memOffset and emplace it into shuffled textures
				shuffledTextures.emplace(thisTextureOffset, allTexturePools.at(thisTextureInfo.category).at(randomTextureIndex));
			}
			else
			{ // roll failure, no randomisation
				shuffledTextures.emplace(thisTextureOffset, thisTextureOffset); // texture is mapped to itself, ie no randomisation
			}

		}
	}

	assert(shuffledTextures.size() == textureMap.size());

}


TextureInfo TextureRandomiser::readTextureInfo(const tagElement& textureTag)
{
	TextureInfo thisTextureInfo(instance->mapReader->getTagName(textureTag.tagDatum));
	thisTextureInfo.dat = textureTag.tagDatum;
	thisTextureInfo.memoryOffset = textureTag.offset;
	thisTextureInfo.category = sortIntoCategory(thisTextureInfo);
	return thisTextureInfo;
}



TextureCategory TextureRandomiser::sortIntoCategory(const TextureInfo& textureInfo)
{
	PLOG_VERBOSE << textureInfo.getFullName();
	if (textureInfo.getFullName().starts_with("rasterizer")
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
#include "pch.h"
#include "TextureRandomiserDatum.h"
#include "MessagesGUI.h"

#if CEER_DEBUG
TextureInfo* lastOriginalTextureDatum = nullptr;
TextureInfo* lastReplacedTextureDatum = nullptr;
void TextureRandomiserDatum::DebugLastTextureDatum()
{
	if (lastOriginalTextureDatum && lastReplacedTextureDatum)
	{
		std::string message = std::format("og last: {}\n replaced with: {}", lastOriginalTextureDatum->getFullName(), lastReplacedTextureDatum->getFullName());
		PLOG_VERBOSE << message;
	}
	else
	{
		PLOG_VERBOSE << "null textures";
	}


	//MessagesGUI::addMessage(message);
}
#endif

uintptr_t someSafeTexture = 0;

// re-randomises textures
void TextureRandomiserDatum::induceSeizure(datum* datumRef)
{

	TextureCategory currentTextureCategory = instance->textureMap.at(*datumRef).category;
	if (SMcategoryIsEnabled.at(currentTextureCategory) == false) // check if current texture has it's category enabled
	{
		return;
	}

	// check if current texture already maps to itself (ie should not be randomised)
	if (instance->shuffledTextures.at(*datumRef) == *datumRef)
	{
		return;
	}

	std::vector<datum>& texturePool = SMallTexturePools.at(currentTextureCategory);

	// decide what to randomise into
	datum newTexture;
	{
		int choiceIndex = SMseizureWhatRandomiseRNG(SMseizureRNG) % (texturePool.size());
		newTexture = texturePool.at(choiceIndex);

	} while (newTexture == *datumRef); // We don't want to randomise back into the original texture because of a check that's done in the hooks for that case

	// write the new texture to the shuffledTextures map
	instance->shuffledTextures[*datumRef] = newTexture;
}



void TextureRandomiserDatum::loadRegularTextureDatumHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		textureDatum,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadRegularTextureDatumFunctionContext.get();

	auto datumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::textureDatum);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(datumRef);
	}

#if CEER_DEBUG

	if (instance->shuffledTextures.contains(*datumRef))
	{
	}
	else
	{
		PLOG_DEBUG << "yuh oh";
		return;
	}

	if (!instance->textureMap.contains(instance->shuffledTextures.at(*datumRef)))
	{
		PLOG_DEBUG << "yowchies";
			return;
	}

	lastOriginalTextureDatum = &instance->textureMap.at(*datumRef);
	lastReplacedTextureDatum = &instance->textureMap.at(instance->shuffledTextures.at(*datumRef));
#endif

	* datumRef = instance->shuffledTextures.at(*datumRef);

}

void TextureRandomiserDatum::loadDecalTextureDatumHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		textureDatum,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadDecalTextureDatumFunctionContext.get();

	auto datumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::textureDatum);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(datumRef);
	}

#if CEER_DEBUG

	if (instance->shuffledTextures.contains(*datumRef))
	{
	}
	else
	{
		PLOG_DEBUG << "yuh oh";
		return;
	}

	if (!instance->textureMap.contains(instance->shuffledTextures.at(*datumRef)))
	{
		PLOG_DEBUG << "yowchies";
		return;
	}

	lastOriginalTextureDatum = &instance->textureMap.at(*datumRef);
	lastReplacedTextureDatum = &instance->textureMap.at(instance->shuffledTextures.at(*datumRef));
#endif

	* datumRef = instance->shuffledTextures.at(*datumRef);

}

void TextureRandomiserDatum::loadParticleTextureDatumHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		textureDatum,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadParticleTextureDatumFunctionContext.get();

	auto datumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::textureDatum);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(datumRef);
	}

#if CEER_DEBUG

	if (instance->shuffledTextures.contains(*datumRef))
	{
	}
	else
	{
		PLOG_DEBUG << "yuh oh";
		return;
	}

	if (!instance->textureMap.contains(instance->shuffledTextures.at(*datumRef)))
	{
		PLOG_DEBUG << "yowchies";
		return;
	}

	lastOriginalTextureDatum = &instance->textureMap.at(*datumRef);
	lastReplacedTextureDatum = &instance->textureMap.at(instance->shuffledTextures.at(*datumRef));
#endif

	* datumRef = instance->shuffledTextures.at(*datumRef);

}

void TextureRandomiserDatum::loadContrailTextureDatumHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		textureDatum,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadContrailTextureDatumFunctionContext.get();

	auto datumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::textureDatum);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(datumRef);
	}

#if CEER_DEBUG

	if (instance->shuffledTextures.contains(*datumRef))
	{
	}
	else
	{
		PLOG_DEBUG << "yuh oh";
		return;
	}

	if (!instance->textureMap.contains(instance->shuffledTextures.at(*datumRef)))
	{
		PLOG_DEBUG << "yowchies";
		return;
	}

	lastOriginalTextureDatum = &instance->textureMap.at(*datumRef);
	lastReplacedTextureDatum = &instance->textureMap.at(instance->shuffledTextures.at(*datumRef));
#endif

	* datumRef = instance->shuffledTextures.at(*datumRef);

}

void TextureRandomiserDatum::loadLensTextureDatumHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		textureDatum,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadLensTextureDatumFunctionContext.get();

	auto datumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::textureDatum);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(datumRef);
	}

#if CEER_DEBUG

	if (instance->shuffledTextures.contains(*datumRef))
	{
	}
	else
	{
		PLOG_DEBUG << "yuh oh";
		return;
	}

	if (!instance->textureMap.contains(instance->shuffledTextures.at(*datumRef)))
	{
		PLOG_DEBUG << "yowchies";
		return;
	}

	lastOriginalTextureDatum = &instance->textureMap.at(*datumRef);
	lastReplacedTextureDatum = &instance->textureMap.at(instance->shuffledTextures.at(*datumRef));
#endif

	* datumRef = instance->shuffledTextures.at(*datumRef);

}

void TextureRandomiserDatum::loadHudTextureDatumHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		textureDatum,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadHudTextureDatumFunctionContext.get();

	auto datumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::textureDatum);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(datumRef);
	}

#if CEER_DEBUG

	if (instance->shuffledTextures.contains(*datumRef))
	{
	}
	else
	{
		PLOG_DEBUG << "yuh oh";
		return;
	}

	if (!instance->textureMap.contains(instance->shuffledTextures.at(*datumRef)))
	{
		PLOG_DEBUG << "yowchies";
		return;
	}

	lastOriginalTextureDatum = &instance->textureMap.at(*datumRef);
	lastReplacedTextureDatum = &instance->textureMap.at(instance->shuffledTextures.at(*datumRef));
#endif

	* datumRef = instance->shuffledTextures.at(*datumRef);

}

void TextureRandomiserDatum::regularTextureDatumCrashFixHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		nullBad,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->regularTextureDatumCrashFixFunctionContext.get();

	auto checkRef = ctxInterpreter->getParameterRef(ctx, (int)param::nullBad);

	if (*checkRef != 0)
	{
		someSafeTexture = *checkRef;
		return;
	}
	else
	{
		if (someSafeTexture == 0)
		{
			PLOG_FATAL << "fuck";
		}
		*checkRef = someSafeTexture;

#ifdef CEER_DEBUG
		PLOG_VERBOSE << "problem solved :)";
#endif
	}





}
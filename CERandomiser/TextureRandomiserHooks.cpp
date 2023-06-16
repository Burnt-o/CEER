#include "pch.h"
#include "TextureRandomiser.h"
#include "MessagesGUI.h"

#if CEER_DEBUG
TextureInfo* lastOriginalTexture = nullptr;
TextureInfo* lastReplacedTexture = nullptr;
void TextureRandomiser::DebugLastTexture()
{
	MessagesGUI::addMessage(std::format("og last: {}\n replaced with: {}", lastOriginalTexture->getFullName(), lastReplacedTexture->getFullName()));
}
#endif



// re-randomises textures
void TextureRandomiser::induceSeizure(MemOffset* offsetRef)
{

	TextureCategory currentTextureCategory = instance->textureMap.at(*offsetRef).category;
	if (SMcategoryIsEnabled.at(currentTextureCategory) == false) // check if current texture has it's category enabled
	{
		return;
	}

	// check if current texture already maps to itself (ie should not be randomised)
	if (instance->shuffledTextures.at(*offsetRef) == *offsetRef)
	{
		return;
	}

	std::vector<MemOffset>& texturePool = SMallTexturePools.at(currentTextureCategory);

	// decide what to randomise into
	MemOffset newTexture = 0;
	do
	{
		int choiceIndex = SMseizureWhatRandomiseRNG(SMseizureRNG) % (texturePool.size());
		newTexture = texturePool.at(choiceIndex);

	} while (newTexture == *offsetRef); // We don't want to randomise back into the original texture because of a check that's done in the hooks for that case

	// write the new texture to the shuffledTextures map
	instance->shuffledTextures[*offsetRef] = newTexture;
}



void TextureRandomiser::loadRegularTextureHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		memoryOffset,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadRegularTextureFunctionContext.get();

	auto offsetRef = (MemOffset*)ctxInterpreter->getParameterRef(ctx, (int)param::memoryOffset);


	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{

		induceSeizure(offsetRef);
	}


#if CEER_DEBUG
	lastOriginalTexture = &instance->textureMap.at(*offsetRef);
	lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

	*offsetRef = instance->shuffledTextures.at(*offsetRef);

}



void TextureRandomiser::loadDecalTextureHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		memoryOffset,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadDecalTextureFunctionContext.get();

	auto offsetRef = (MemOffset*)ctxInterpreter->getParameterRef(ctx, (int)param::memoryOffset);

	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(offsetRef);
	}


#if CEER_DEBUG
	lastOriginalTexture = &instance->textureMap.at(*offsetRef);
	lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

	* offsetRef = instance->shuffledTextures.at(*offsetRef);
	

}




void TextureRandomiser::loadParticleTextureHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		memoryOffset,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadParticleTextureFunctionContext.get();

	auto offsetRef = (MemOffset*)ctxInterpreter->getParameterRef(ctx, (int)param::memoryOffset);

	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(offsetRef);
	}


#if CEER_DEBUG
	lastOriginalTexture = &instance->textureMap.at(*offsetRef);
	lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

	* offsetRef = instance->shuffledTextures.at(*offsetRef);

}


void TextureRandomiser::loadContrailTextureHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		memoryOffset,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadContrailTextureFunctionContext.get();

	auto offsetRef = (MemOffset*)ctxInterpreter->getParameterRef(ctx, (int)param::memoryOffset);

	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(offsetRef);
	}


#if CEER_DEBUG
	lastOriginalTexture = &instance->textureMap.at(*offsetRef);
	lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

	* offsetRef = instance->shuffledTextures.at(*offsetRef);

}


void TextureRandomiser::loadLensTextureHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		memoryOffset,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadLensTextureFunctionContext.get();

	auto offsetRef = (MemOffset*)ctxInterpreter->getParameterRef(ctx, (int)param::memoryOffset);

	if (SMseizureModeEnabled && SMseizureWillRandomiseRNG(SMseizureRNG) == 0)
	{
		induceSeizure(offsetRef);
	}


#if CEER_DEBUG
	lastOriginalTexture = &instance->textureMap.at(*offsetRef);
	lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

	* offsetRef = instance->shuffledTextures.at(*offsetRef);

}

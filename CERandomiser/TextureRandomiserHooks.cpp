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

	if (instance->shuffledTextures.contains(*offsetRef))
	{
	#if CEER_DEBUG
		lastOriginalTexture = &instance->textureMap.at(*offsetRef);
		lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
	#endif

		*offsetRef = instance->shuffledTextures.at(*offsetRef);
	}

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

	if (instance->shuffledTextures.contains(*offsetRef))
	{
#if CEER_DEBUG
		lastOriginalTexture = &instance->textureMap.at(*offsetRef);
		lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

		* offsetRef = instance->shuffledTextures.at(*offsetRef);
	}

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

	if (instance->shuffledTextures.contains(*offsetRef))
	{
#if CEER_DEBUG
		lastOriginalTexture = &instance->textureMap.at(*offsetRef);
		lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

		* offsetRef = instance->shuffledTextures.at(*offsetRef);
	}

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

	if (instance->shuffledTextures.contains(*offsetRef))
	{
#if CEER_DEBUG
		lastOriginalTexture = &instance->textureMap.at(*offsetRef);
		lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

		* offsetRef = instance->shuffledTextures.at(*offsetRef);
	}

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

	if (instance->shuffledTextures.contains(*offsetRef))
	{
#if CEER_DEBUG
		lastOriginalTexture = &instance->textureMap.at(*offsetRef);
		lastReplacedTexture = &instance->textureMap.at(instance->shuffledTextures.at(*offsetRef));
#endif

		* offsetRef = instance->shuffledTextures.at(*offsetRef);
	}

}

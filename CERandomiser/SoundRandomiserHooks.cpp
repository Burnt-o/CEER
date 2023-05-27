#include "pch.h"
#include "SoundRandomiser.h"
#include "MessagesGUI.h"


#if CEER_DEBUG
SoundInfo* lastOriginalSound = nullptr;
SoundInfo* lastReplacedSound = nullptr;
void SoundRandomiser::DebugLastSound()
{
	MessagesGUI::addMessage(std::format("og last: {}\n replaced with: {}", lastOriginalSound->getFullName(), lastReplacedSound->getFullName()));
}
#endif

void SoundRandomiser::loadSoundAHookFunction(SafetyHookContext& ctx)
{
	if (!instance) { PLOG_ERROR << "null instance!"; return; }
	//std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
	enum class param
	{
		soundDatumParam,
	};

	static MidhookContextInterpreter* ctxInterpreter = instance->loadSoundAFunctionContext.get();

	auto soundDatumRef = (datum*)ctxInterpreter->getParameterRef(ctx, (int)param::soundDatumParam);

	if (instance->shuffledSounds.contains(*soundDatumRef))
	{
#if CEER_DEBUG
		lastOriginalSound = &instance->soundMap.at(*soundDatumRef);
		lastReplacedSound = &instance->soundMap.at(instance->shuffledSounds.at(*soundDatumRef));
		if (lastOriginalSound != lastReplacedSound) PLOG_DEBUG << std::format("og last: {}\n replaced with: {}", lastOriginalSound->getFullName(), lastReplacedSound->getFullName());
#endif

		* soundDatumRef = instance->shuffledSounds.at(*soundDatumRef);
	}

}

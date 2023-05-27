
#pragma once
#include "HaloEnums.h"
#include "ModuleHook.h"
#include "PointerManager.h"
#include "MapReader.h"
#include "SetSeed.h"
#include "OptionsState.h"
#include "SoundInfo.h"



class SoundRandomiser
{
private:
	static SoundRandomiser* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing

	// ref to MapReader for reading map stuff like actorPalettes and etc
	MapReader* mapReader;


	// handle to our callback of OptionsState::SoundRandomiser so we can remove it in destructor
	eventpp::CallbackList<void(bool&)>::Handle mSoundRandomiserToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mSoundRandomiserToggleEvent;

	// handle to our callback of LevelLoadHook so we can remove it in destructor
	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	// What we run when OptionsState::SoundRandomiser changes
	static void onSoundRandomiserToggleChange(bool& newValue);

	// What we run when new level is loaded changes
	// can also be invoked by options being enabled when a level is already loaded
	static void onLevelLoadEvent(HaloLevel newLevel);


	// main hooks
	std::shared_ptr<ModuleMidHook> loadSoundAHook;
	static void loadSoundAHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadSoundAFunctionContext;



	std::map<datum, SoundInfo> soundMap;
	std::map<datum, datum> shuffledSounds;

	std::once_flag lazyInitOnceFlag;
	void lazyInit();

	uint64_t ourSeed = 0x12355678;

	void evaluateSounds();
	SoundInfo readSoundInfo(const tagElement& soundTag);
	SoundCategory sortIntoCategory(const SoundInfo& soundInfo);


public:
	explicit SoundRandomiser(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent, MapReader* mapR)
		: mLevelLoadEvent(levelLoadEvent), mapReader(mapR), mSoundRandomiserToggleEvent(OptionsState::SoundRandomiser.valueChangedEvent)
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one EnemyRandomiser");
		}
		instance = this;

		// Listen to the events we care about
		mSoundRandomiserToggleCallbackHandle = mSoundRandomiserToggleEvent.append(&onSoundRandomiserToggleChange);
		mLevelLoadCallbackHandle = levelLoadEvent.append(&onLevelLoadEvent);

	}

	~SoundRandomiser()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		// Unsubscribe events
		mSoundRandomiserToggleEvent.remove(mSoundRandomiserToggleCallbackHandle);
		mLevelLoadEvent.remove(mLevelLoadCallbackHandle);

		//destroy hooks
#define safe_destroy_hook(x) if (x.get()) x->setWantsToBeAttached(false)
		safe_destroy_hook(loadSoundAHook);




		instance = nullptr;
	}


#if CEER_DEBUG
	static void DebugLastSound();
#endif

};


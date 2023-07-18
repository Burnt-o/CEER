#pragma once
#include "HaloEnums.h"
#include "ModuleHook.h"
#include "PointerManager.h"
#include "MapReader.h"
#include "SetSeed.h"
#include "OptionsState.h"
#include "TextureInfo.h"


class TextureRandomiserDatum
{
private:
	static TextureRandomiserDatum* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing

	// ref to MapReader for reading map stuff like actorPalettes and etc
	MapReader* mapReader;


	// handle to our callback of OptionsState::TextureRandomiser so we can remove it in destructor
	eventpp::CallbackList<void(bool&)>::Handle mTextureRandomiserToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mTextureRandomiserToggleEvent;

	// handle to our callback of LevelLoadHook so we can remove it in destructor
	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	// What we run when OptionsState::TextureRandomiser changes
	static void onTextureRandomiserToggleChange(bool& newValue);

	// What we run when new level is loaded changes
	// can also be invoked by options being enabled when a level is already loaded
	static void onLevelLoadEvent(HaloLevel newLevel);


	// main hooks
	std::shared_ptr<ModuleMidHook> loadRegularTextureDatumHook;
	static void loadRegularTextureDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadRegularTextureDatumFunctionContext;

	std::shared_ptr<ModuleMidHook> loadDecalTextureDatumHook;
	static void loadDecalTextureDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadDecalTextureDatumFunctionContext;

	std::shared_ptr<ModuleMidHook> loadParticleTextureDatumHook;
	static void loadParticleTextureDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadParticleTextureDatumFunctionContext;

	std::shared_ptr<ModuleMidHook> loadContrailTextureDatumHook;
	static void loadContrailTextureDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadContrailTextureDatumFunctionContext;

	std::shared_ptr<ModuleMidHook> loadLensTextureDatumHook;
	static void loadLensTextureDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadLensTextureDatumFunctionContext;

	std::shared_ptr<ModuleMidHook> loadHudTextureDatumHook;
	static void loadHudTextureDatumHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> loadHudTextureDatumFunctionContext;

	std::shared_ptr<ModuleMidHook> regularTextureDatumCrashFixHook;
	static void regularTextureDatumCrashFixHookFunction(SafetyHookContext& ctx);
	std::shared_ptr<MidhookContextInterpreter> regularTextureDatumCrashFixFunctionContext;


	std::vector<datum> seizureModeVector;
	std::map<datum, TextureInfo> textureMap;
	std::map<datum, datum> shuffledTextures;

	std::once_flag lazyInitOnceFlag;
	void lazyInit();

	uint64_t ourSeed = 0x12355678;

	void evaluateTextures();
	TextureInfo readTextureInfo(const tagElement& textureTag);
	TextureCategory sortIntoCategory(const TextureInfo& textureInfo);

	//seizure stuff
	static void induceSeizure(datum* offsetRef);
	static bool SMseizureModeEnabled;
	static std::map<TextureCategory, bool> SMcategoryIsEnabled;;
	static std::map<TextureCategory, std::vector<datum>> SMallTexturePools;
	static std::mt19937 SMseizureRNG; // generator needed to grab numbers from the int distributions
	static std::uniform_int_distribution<> SMseizureWillRandomiseRNG; // used to determine whether a texture will be re-randomised on any given frame
	static std::uniform_int_distribution<> SMseizureWhatRandomiseRNG; // if re-randomised, used to select which texture to rando into 



	bool hudTexAvailable = true;


public:
	explicit TextureRandomiserDatum(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent, MapReader* mapR)
		: mLevelLoadEvent(levelLoadEvent), mapReader(mapR), mTextureRandomiserToggleEvent(OptionsState::TextureRandomiser.valueChangedEvent)
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one EnemyRandomiser");
		}
		instance = this;

		// Listen to the events we care about
		mTextureRandomiserToggleCallbackHandle = mTextureRandomiserToggleEvent.append(&onTextureRandomiserToggleChange);
		mLevelLoadCallbackHandle = levelLoadEvent.append(&onLevelLoadEvent);


	}

	~TextureRandomiserDatum()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		// Unsubscribe events
		mTextureRandomiserToggleEvent.remove(mTextureRandomiserToggleCallbackHandle);
		mLevelLoadEvent.remove(mLevelLoadCallbackHandle);

		//destroy hooks
#define safe_destroy_hook(x) if (x.get()) x->setWantsToBeAttached(false)
		safe_destroy_hook(loadRegularTextureDatumHook);
		safe_destroy_hook(loadDecalTextureDatumHook);
		safe_destroy_hook(loadParticleTextureDatumHook);
		safe_destroy_hook(loadContrailTextureDatumHook);
		safe_destroy_hook(loadLensTextureDatumHook);
		safe_destroy_hook(loadHudTextureDatumHook);

		
		instance = nullptr;
	}


#if CEER_DEBUG
	static void DebugLastTextureDatum();
#endif

};


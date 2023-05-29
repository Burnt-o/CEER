#pragma once
#include "HaloEnums.h"
#include "OptionsState.h"

class CEERStateLogger
{
private:
	std::mutex mDestructionGuard; 

	eventpp::CallbackList<void(bool&)>::Handle mEnemyRandomiserToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mEnemyRandomiserToggleEvent;

	eventpp::CallbackList<void(bool&)>::Handle mEnemySpawnMultiplierToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mEnemySpawnMultiplierToggleEvent;

	eventpp::CallbackList<void(bool&)>::Handle mTextureRandomiserToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mTextureRandomiserToggleEvent;

	eventpp::CallbackList<void(bool&)>::Handle mSoundRandomiserToggleCallbackHandle = {};
	eventpp::CallbackList<void(bool&)>& mSoundRandomiserToggleEvent;

	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	void logOnToggleChange(bool& newValue, std::string optionName);
	void logOnLevelChange(HaloLevel newLevel);

public:

	explicit CEERStateLogger(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent)
		: mLevelLoadEvent(levelLoadEvent), mEnemyRandomiserToggleEvent(OptionsState::EnemyRandomiser.valueChangedEvent),
		mEnemySpawnMultiplierToggleEvent(OptionsState::EnemySpawnMultiplier.valueChangedEvent),
		mTextureRandomiserToggleEvent(OptionsState::TextureRandomiser.valueChangedEvent),
		mSoundRandomiserToggleEvent(OptionsState::SoundRandomiser.valueChangedEvent)
	{
		mEnemyRandomiserToggleCallbackHandle = mEnemyRandomiserToggleEvent.append([this](bool& newValue) {logOnToggleChange(newValue, "EnemyRandomiser"); });
		mEnemySpawnMultiplierToggleCallbackHandle = mEnemySpawnMultiplierToggleEvent.append([this](bool& newValue) {logOnToggleChange(newValue, "EnemySpawnMultiplier"); });
		mTextureRandomiserToggleCallbackHandle = mTextureRandomiserToggleEvent.append([this](bool& newValue) {logOnToggleChange(newValue, "TextureRandomiser"); });
		mSoundRandomiserToggleCallbackHandle = mSoundRandomiserToggleEvent.append([this](bool& newValue) {logOnToggleChange(newValue, "SoundRandomiser"); });
		mLevelLoadCallbackHandle = levelLoadEvent.append([this](HaloLevel newLevel) {logOnLevelChange(newLevel); });
	}

	~CEERStateLogger()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		mEnemyRandomiserToggleEvent.remove(mEnemyRandomiserToggleCallbackHandle);
		mEnemySpawnMultiplierToggleEvent.remove(mEnemySpawnMultiplierToggleCallbackHandle);
		mTextureRandomiserToggleEvent.remove(mTextureRandomiserToggleCallbackHandle);
		mSoundRandomiserToggleEvent.remove(mSoundRandomiserToggleCallbackHandle);
		mLevelLoadEvent.remove(mLevelLoadCallbackHandle);
	}

};


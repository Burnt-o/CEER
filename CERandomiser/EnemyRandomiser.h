#pragma once
#include "HaloEnums.h"


class EnemyRandomiser
{
private:
	static EnemyRandomiser* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing

	// handle to our callback of OptionsState::EnemyRandomiserEnabled so we can remove it in destructor
	eventpp::CallbackList<void(bool&, bool&)>::Handle mEnabledCallbackHandle = {};
	eventpp::CallbackList<void(bool&, bool&)>& mEnabledEvent;
	// handle to our callback of LevelLoadHook so we can remove it in destructor
	eventpp::CallbackList<void(HaloLevel)>::Handle mLevelLoadCallbackHandle = {};
	eventpp::CallbackList<void(HaloLevel)>& mLevelLoadEvent;

	// What we run when EnemyRandomiserEnabled changes
	static void onEnemyRandomiserEnabledChanged(bool& newValue, bool& oldValue);

	// What we run when new level is loaded changes
	static void onLevelLoadEvent(HaloLevel newLevel);

	// Used to know if we need to read thru the enemy data and setup the probability table
	// Is set true by onEnemyRandomiserEnabledChanged w/ true value 
		// and by levelLoadCallback
	// Hooks check this and run init if true
	bool needToLoadGameData = false;

public:
	explicit EnemyRandomiser(eventpp::CallbackList<void(bool&, bool&)>& enabledEvent, eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent)
		: mEnabledEvent(enabledEvent), mLevelLoadEvent(levelLoadEvent)
	{
		if (instance != nullptr)
		{
			throw expected_exception("Cannot have more than one EnemyRandomiser");
		}
		instance = this;

		// Listen to the events we care about
		mEnabledCallbackHandle = enabledEvent.append(&onEnemyRandomiserEnabledChanged);
		mLevelLoadCallbackHandle = levelLoadEvent.append(&onLevelLoadEvent);

		// Set up our hooks
		// TODO

	}

	~EnemyRandomiser()
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard);
		// Unsubscribe events
		mEnabledEvent.remove(mEnabledCallbackHandle);
		mLevelLoadEvent.remove(mLevelLoadCallbackHandle);

		//TODO: destroy hooks

		instance = nullptr;
	}

};


#pragma once
#include "ModuleHookManager.h"
#include "MultilevelPointer.h"
#include "HaloEnums.h"
#include "PointerManager.h"
// Hooks H1 Level loading and fires an event when it happens
class LevelLoadHook
{
private:
	static LevelLoadHook* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing

	// Function we run when hook runs
	static void levelLoadHookFunction(SafetyHookContext ctx)
	{
		PLOG_VERBOSE << "levelLoadHookFunction running";
		std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

		// Find the current level
		char currentLevel[4];
		if (!instance->currentLevelName->readArrayData(&currentLevel, 4))
		{
			PLOG_ERROR << "Failed to read current level: " << MultilevelPointer::GetLastError();
			return;
		}

		// Set the null terminator at the end
		currentLevel[3] = '\0';

		auto itr = std::ranges::find(instance->mapNames, currentLevel);
		if (itr == instance->mapNames.end())
		{
			PLOG_ERROR << "Current level string didn't appear to be valid: " << currentLevel;
			return;
		}

		// Get index of HaloLevel enum
		int index = std::distance(instance->mapNames.begin(), itr);
			
		// Call event
		instance->levelLoadEvent(HaloLevel(index));


	}

	// Hook that invokes event
	std::unique_ptr<ModuleMidHook> levelLoadHook;


	std::vector<std::string> mapNames = { "a10", "a30", "a50", "b30", "b40", "c10", "c20", "c40", "d20", "d40"};
	std::shared_ptr<MultilevelPointer> currentLevelName;
	

public:

	// Event we fire on level load
	eventpp::CallbackList<void(HaloLevel)> levelLoadEvent;


	explicit LevelLoadHook()
	{
		if (instance != nullptr)
		{
			throw ExpectedException("Cannot have more than one LevelLoadHook");
		}
		instance = this;

		
		// Get pointers
		try
		{
			currentLevelName = PointerManager::getMultilevelPointer("currentLevelName");
			auto levelLoadFunction = PointerManager::getMultilevelPointer("levelLoadFunction");

			// Set up the hook 
			levelLoadHook = ModuleMidHook::make(
				L"halo1.dll",
				levelLoadFunction,
				(safetyhook::MidHookFn)&levelLoadHookFunction,
				true);
		}
		catch (ExpectedException& ex)
		{
			ex.prepend("LevelLoadHook could not resolve hooks: ");
			throw ex;
		}




	
	}

	~LevelLoadHook()
	{
		std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);
		instance = nullptr;
	}

};


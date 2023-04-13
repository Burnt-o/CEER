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
	static void hookFunction(SafetyHookContext ctx)
	{
		PLOG_VERBOSE << "levelLoadHookFunction running";
		std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

		// Find the current level
		char currentLevel[4];
		if (!instance->MLP_currentLevelName->readArrayData(&currentLevel, 4))
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
	std::shared_ptr<ModuleMidHook> levelLoadHook;


	std::vector<std::string> mapNames = { "a10", "a30", "a50", "b30", "b40", "c10", "c20", "c40", "d20", "d40"};
	std::shared_ptr<MultilevelPointer> MLP_currentLevelName;
	std::shared_ptr<MultilevelPointer> MLP_levelLoadHook;
	//std::shared_ptr<MultilevelPointer> pCurrentLevelName = MultilevelPointer::make(L"halo1.dll", { 0x2AF8288 });

public:

	// Event we fire on level load
	eventpp::CallbackList<void(HaloLevel)> levelLoadEvent;


	explicit LevelLoadHook()
	{
		if (instance != nullptr)
		{
			throw expected_exception("Cannot have more than one LevelLoadHook");
		}
		instance = this;

		
		// Get pointers
		try
		{
			MLP_currentLevelName = PointerManager::getMultilevelPointer("MLP_currentLevelName");
			MLP_levelLoadHook = PointerManager::getMultilevelPointer("MLP_levelLoadHook");
		}
		catch (expected_exception& ex)
		{
			ex.prepend("LevelLoadHook could not resolve pointers: ");
			throw ex;
		}



		// Set up the hook 
		
		levelLoadHook = std::make_shared<ModuleMidHook>(
			L"levelLoadHook", 
			MLP_levelLoadHook,
			(safetyhook::MidHookFn)&hookFunction,
			true);
		
		// Add to the moduleHookManager so it is appopiately loaded/unloaded on dll load/unload
		ModuleHookManager::addHook(L"halo1.dll", levelLoadHook);
		// Try attaching it right now in case halo1.dll already loaded
		levelLoadHook.get()->attach();
	}

	~LevelLoadHook()
	{
		std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

		ModuleHookManager::removeHook(L"levelLoadHook");

		instance = nullptr;
	}

};


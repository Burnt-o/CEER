#pragma once
#include "ModuleHookManager.h"
#include "MultilevelPointer.h"
#include "HaloEnums.h"
#include "PointerManager.h"
#include "OptionsState.h"
// Hooks H1 Level loading and fires an event when it happens
class LevelLoadHook
{
private:
	static LevelLoadHook* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing



	static HaloLevel getCurrentLevel() 
	{
		// Find the current level
		char currentLevel[4];
		if (!instance->currentLevelName->readArrayData(&currentLevel, 4))
		{
			throw CEERRuntimeException(std::format("Failed to read current level: {}", MultilevelPointer::GetLastError()));
		}

		// Set the null terminator at the end
		currentLevel[3] = '\0';

		auto itr = std::ranges::find(instance->mapNames, currentLevel);
		if (itr == instance->mapNames.end())
		{
			throw CEERRuntimeException(std::format("Failed to read current level: {}", MultilevelPointer::GetLastError()));
		}

		// Get index of HaloLevel enum
		int index = std::distance(instance->mapNames.begin(), itr);

		return (HaloLevel)index;
	}


	// Function we run when hook runs
	static void levelLoadHookFunction(SafetyHookContext ctx)
	{
		PLOG_VERBOSE << "levelLoadHookFunction running";
		std::scoped_lock<std::mutex> lock(instance->mDestructionGuard);

		try
		{
			HaloLevel currentLevel = getCurrentLevel();
			fireLevelLoadEvent(currentLevel);
		}
		catch (CEERRuntimeException& ex)
		{
			RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
		}
			



	}

	// Hook that invokes event
	std::unique_ptr<ModuleMidHook> levelLoadHook;


	std::vector<std::string> mapNames = { "a10", "a30", "a50", "b30", "b40", "c10", "c20", "c40", "d20", "d40"};
	std::shared_ptr<MultilevelPointer> currentLevelName;

	

	
	 static void fireLevelLoadEvent(HaloLevel currentLevel)
	 {
		 try
		 {
			 // Call event
			 instance->levelLoadEvent(currentLevel);
		 }
		 catch (CEERRuntimeException& ex)
		 { 
			// Let runtime handler handle any exceptions thrown by listeners
			 PLOG_ERROR << "levelLoadEvent exception caught " << ex.what();
			 RuntimeExceptionHandler::handle(ex, &OptionsState::MasterToggle);
		 }
	 }

public:
	static bool isLevelAlreadyLoaded(HaloLevel& outCurrentLevel);
	// Event we fire on level load
	eventpp::CallbackList<void(HaloLevel)> levelLoadEvent;


	explicit LevelLoadHook()
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one LevelLoadHook");
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
		catch (InitException& ex)
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


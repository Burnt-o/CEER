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
		char curLevel[4];
		if (!instance->currentLevelName->readArrayData(&curLevel, 4))
		{
			throw CEERRuntimeException(std::format("Failed to read current level: {}", MultilevelPointer::GetLastError()));
		}

		// Set the null terminator at the end
		curLevel[3] = '\0';
		std::string currentLevel{ curLevel }; // convert to string

		if (codeStringToLevel.contains(currentLevel))
		{
			PLOG_VERBOSE << std::format("Read current level {} from codeString {}", levelToString.at(codeStringToLevel.at(currentLevel)), currentLevel);
			return codeStringToLevel.at(currentLevel);
		}
		else
		{
			PLOG_INFO << std::format("Failed to read current level: {}", curLevel);
			return HaloLevel::UNK;
		}

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
			RuntimeExceptionHandler::handleMessage(ex);
		}
			



	}

	// Hook that invokes event
	std::unique_ptr<ModuleMidHook> levelLoadHook;

	std::shared_ptr<MultilevelPointer> currentLevelName;
	std::shared_ptr<MultilevelPointer> currentCacheAddress;
	std::shared_ptr<MultilevelPointer> stateIndicator;
	

	
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
			 RuntimeExceptionHandler::handleMessage(ex);
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
			stateIndicator = PointerManager::getMultilevelPointer("stateIndicator");
			currentLevelName = PointerManager::getMultilevelPointer("currentLevelName");
			currentCacheAddress = PointerManager::getMultilevelPointer("currentCacheAddress");
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


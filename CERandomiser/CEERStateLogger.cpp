#include "pch.h"
#include "CEERStateLogger.h"
#include "OptionSerialisation.h"
#include "SetSeed.h"
#include "MessagesGUI.h"
#include "LevelLoadHook.h"
void optionStateSnapshot()
{
	try
	{
		std::stringstream ss;
		OptionSerialisation::serialiseAllOptions().print(ss);
		PLOG_INFO << "OptionsState snapshot: \n" << ss.str();
	}
	catch (SerialisationException& ex)
	{
		RuntimeExceptionHandler::handleSilent(ex);
	}
}

// create a "signature" of the settings and print it to the screen
void printConfigSignature()
{
	try
	{
		std::stringstream ss;
		OptionSerialisation::serialiseAllOptions(true).print(ss);
		std::string optionsString = ss.str();
		SetSeed64 twister(0);
		for (char c : optionsString)
		{
			auto val = twister();
			val += c;
			twister(val);
		}
		MessagesGUI::addMessage(std::format("Config Signature: {:X}", twister()));
	}
	catch (SerialisationException& ex)
	{
		RuntimeExceptionHandler::handleSilent(ex);
	}

}


void CEERStateLogger::logOnToggleChange(bool& newValue, std::string optionName)
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard);
	PLOG_INFO << optionName << " was " << (newValue ? "enabled!" : "disabled!");
	optionStateSnapshot();
	HaloLevel currentLevel = HaloLevel::UNK;
	bool inLevel = LevelLoadHook::isLevelAlreadyLoaded(currentLevel);


#if printConfigSignatureOnToggleChange == 1
	if (inLevel &&
		OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue() || OptionsState::TextureRandomiser.GetValue() || OptionsState::SoundRandomiser.GetValue())
	{
		printConfigSignature();
	}
#endif
	if (inLevel)
	{
		PLOG_INFO << "Current level: " << levelToString.at(currentLevel);
	}
	else
	{
		PLOG_INFO << "User not currently in a level";
	}

}
void CEERStateLogger::logOnLevelChange(HaloLevel newLevel)
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard);
	PLOG_INFO << "New level loaded: " << levelToString.at(newLevel);

	if (OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue() || OptionsState::TextureRandomiser.GetValue() || OptionsState::SoundRandomiser.GetValue())
	{
		printConfigSignature();
	}
}
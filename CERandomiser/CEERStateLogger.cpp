#include "pch.h"
#include "CEERStateLogger.h"
#include "OptionSerialisation.h"

void optionStateSnapshot()
{
	try
	{
		PLOG_INFO << "OptionsState snapshot: \n" << OptionSerialisation::serialiseAll();
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

}
void CEERStateLogger::logOnLevelChange(HaloLevel newLevel)
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard);
	PLOG_INFO << "New level loaded: " << levelToString.at(newLevel);

}
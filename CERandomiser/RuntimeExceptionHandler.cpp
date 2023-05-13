#include "pch.h"
#include "RuntimeExceptionHandler.h"

#include "Option.h"
//#include "OptionsGUI.h"
#include "MessagesGUI.h"
RuntimeExceptionHandler* RuntimeExceptionHandler::instance = nullptr;


// Shows error message to user, and disables related toggle options if any were passed to it (optional)
void RuntimeExceptionHandler::handle(CEERRuntimeException& ex, std::vector<Option<bool>*> optionsToDisable)
{
	// Log to console
	PLOG_ERROR << "EXCEPTION: " << std::string(ex.what()) << std::endl << std::string(ex.source()) << std::endl << std::string(ex.trace());

	std::string optionsDisabledString; // left empty if no options are being disabled
	if (optionsToDisable.size() > 0)
	{
		optionsDisabledString += "\nThe following options were disabled:";
		for (auto option : optionsToDisable)
		{
			if (!option) continue; // Don't know how a nullptr would get in here, but good to double check
			// check if the option was already disabled, we can skip if so
			if (option->GetValue() == false) continue;
			option->resetToDefaultValue();
			optionsDisabledString += "\n * " + option->getOptionName();
		}
	}

	MessagesGUI::addMessage(
		std::format(
			"An error occured:\n{}{}\nSee CEER_logging.txt for more info, and send it to Burnt so he can fix it.", 
			ex.what(), optionsDisabledString));

}



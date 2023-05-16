#pragma once

#include "Option.h"


class RuntimeExceptionHandler
{
private:
	// private singleton
	static RuntimeExceptionHandler* instance;

	eventpp::CallbackList<void()>& renderEventRef;


public:

	// Shows error message to user, and disables related toggle options if any were passed to it (optional)
	static void handleSilent(CEERExceptionBase& ex, std::vector<Option<bool>*> optionsToDisable = {});
	static void handleMessage(CEERExceptionBase& ex, std::vector<Option<bool>*> optionsToDisable = {});
	static void handlePopup(CEERExceptionBase& ex, std::vector<Option<bool>*> optionsToDisable = {});
	
	explicit RuntimeExceptionHandler(eventpp::CallbackList<void()>& renderEvent) : renderEventRef(renderEvent) 
	{
		if (instance) throw InitException("Cannot have more than one RuntimeExceptionHandler");
		instance = this;
	}

	~RuntimeExceptionHandler() { instance = nullptr; }


};


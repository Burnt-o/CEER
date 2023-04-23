#pragma once

#include "Option.h"

#define throw_from_hook(x, y)		CEERRuntimeException exception(x);\
									RuntimeExceptionHandler::handle(exception, y);\
									return;\


class RuntimeExceptionHandler
{
private:
	// private singleton
	static RuntimeExceptionHandler* instance;

	eventpp::CallbackList<void()>& renderEventRef;

	static void disableOption(Option<bool>* optionToDisable);
	static void showErrorMessage(CEERRuntimeException& ex);

public:
	static void handle(CEERRuntimeException& ex);

	static void handle(CEERRuntimeException& ex, Option<bool>* optionToDisable);

	explicit RuntimeExceptionHandler(eventpp::CallbackList<void()>& renderEvent) : renderEventRef(renderEvent) 
	{
		if (instance) throw InitException("Cannot have more than one RuntimeExceptionHandler");
		instance = this;
	}

	~RuntimeExceptionHandler() { instance = nullptr; }


};


#include "pch.h"
#include "RuntimeExceptionHandler.h"

#include "ImGuiDialogBox.h"
#include "Option.h"

RuntimeExceptionHandler* RuntimeExceptionHandler::instance = nullptr;

void RuntimeExceptionHandler::handle(CEERRuntimeException& ex) 
{
	showErrorMessage(ex);
}

void RuntimeExceptionHandler::handle(CEERRuntimeException& ex, Option<bool>* optionToDisable)
{
	disableOption(optionToDisable);
	showErrorMessage(ex);
}


void RuntimeExceptionHandler::disableOption(Option<bool>* optionToDisable)
{
	optionToDisable->resetToDefaultValue();
}



void RuntimeExceptionHandler::showErrorMessage(CEERRuntimeException& ex)
{
	PLOG_ERROR << "EXCEPTION: " << std::string(ex.what()) << std::endl << std::string(ex.source()) << std::endl << std::string(ex.trace());
	auto errorDialog = new ErrorMessageBox(instance->renderEventRef, std::string(ex.what()), std::string(ex.trace()));
}


#include "pch.h"
#include "MidhookContextInterpreter.h"
#include "OptionsState.h"
// return ref to the ctx register we want


uintptr_t* MidhookContextInterpreter::getParameterRef(SafetyHookContext& ctx, int parameterIndex)
{

	if (parameterIndex > mParameterRegisterIndices.size())
	{
		CEERRuntimeException exception(std::format("MidhookContextInterpreter had invalid access of parameter! paramArray.size(): {}, accessed parameter index: {}", mParameterRegisterIndices.size(), parameterIndex)); 
		RuntimeExceptionHandler::handle(exception, &OptionsState::MasterToggle); 
		return nullptr;
	}

	uintptr_t* ctxArray = reinterpret_cast<uintptr_t*>(&ctx);
	return &ctxArray[mParameterRegisterIndices.at(parameterIndex)];

}
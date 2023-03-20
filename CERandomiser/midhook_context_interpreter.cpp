#include "pch.h"
#include "midhook_context_interpreter.h"




midhook_context_interpreter::midhook_context_interpreter(std::vector<int> cfp)
	{
		context_from_parameter = cfp;
	}

uintptr_t midhook_context_interpreter::GetParameterValue(SafetyHookContext& ctx, int parameter_index)
{
	uintptr_t* ctxArray = reinterpret_cast<uintptr_t*>(&ctx);
	return ctxArray[context_from_parameter[parameter_index]];
}

void midhook_context_interpreter::SetParameterValue(SafetyHookContext& ctx, int parameter_index, uintptr_t value)
{
	uintptr_t* ctxArray = reinterpret_cast<uintptr_t*>(&ctx);
	ctxArray[context_from_parameter[parameter_index]] = value;
}


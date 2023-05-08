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

	ParameterLocation& thisParameter = mParameterRegisterIndices.at(parameterIndex);
	uintptr_t* ctxArray = reinterpret_cast<uintptr_t*>(&ctx);

	if (thisParameter.getOffsets().empty())
	{
		return &ctxArray[thisParameter.getRegIndex()];
	}
	else
	{
		uintptr_t ptr = ctxArray[thisParameter.getRegIndex()];
		auto &offsets = thisParameter.getOffsets();



		for (int i = 0; i < offsets.size(); i++)
		{
			int offset = offsets[i];
			uintptr_t follow = (uintptr_t)(ptr + offset);
			if (IsBadReadPtr((void*)follow, 8))
			{
				CEERRuntimeException ex(std::format("bad read in midhookcontextinterpreter! next: {}, ptr: {}, offset: {}", follow, ptr, offset));
				RuntimeExceptionHandler::handle(ex);
				return nullptr;
			}

			if (i == 0)
			{
				ptr = follow;
			}
			else
			{
				ptr = *(uintptr_t*)follow; // dereference
			}
		}

		return (uintptr_t*)ptr;
	}
}


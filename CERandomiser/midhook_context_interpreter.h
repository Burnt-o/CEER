#pragma once

// This is the order safetyhook stores registers in a SafetyHookContext
enum class Register : unsigned char {
	rflags, r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rdx, rcx, rbx, rax, rbp, rsp
};


// One of these interpreter objects will be associated with each MidHook callback function
// This allows us to figure out at runtime which registers contains the parameters that the callback wants to manipulate
// eg depending what version of MCC we've been injected into
class MidhookContextInterpreter
{
private:
	std::vector<Register> mParameterRegisterIndices; //ie index 0 contains the RegisterIndex of the FIRST parameter that the midhook callback function cares about
	std::vector<safetyhook::Context64> mpri;
public:
	explicit MidhookContextInterpreter(std::vector<Register> parameterRegisterIndices) : mParameterRegisterIndices(parameterRegisterIndices) {}

	// return ref to the ctx register we want
	uintptr_t* getParameterRef(SafetyHookContext& ctx, int parameterIndex)
	{
		uintptr_t* ctxArray = reinterpret_cast<uintptr_t*>(&ctx); 
		return &ctxArray[std::to_underlying(mParameterRegisterIndices.at(parameterIndex))];

	}

};

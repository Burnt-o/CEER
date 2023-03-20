#pragma once

enum reg_index {
	rflags, r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rdx, rcx, rbx, rax, rbp, rsp
};


class midhook_context_interpreter
{
private:
public:
	std::vector<int> context_from_parameter;

	midhook_context_interpreter(std::vector<int> cfp);

	uintptr_t GetParameterValue(SafetyHookContext& ctx, int parameter_index);
	void SetParameterValue(SafetyHookContext& ctx, int parameter_index, uintptr_t value);

};

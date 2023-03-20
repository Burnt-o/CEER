#pragma once
#include "multilevel_pointer.h"



// Base class with management stuff
class Hook {
protected:
	std::wstring mHookName;
	std::wstring mAssociatedModule; // used to know if a hook should be reattached or attached on dll load/unload
	void* mInstalledAt = nullptr;
	bool mShouldBeEnabled = false;
	bool mInstalled = false;
public:
	// Contracts - must be overridden
	virtual void attach() = 0;
	virtual void detach() = 0;

	Hook(const std::wstring& hook_name, bool startEnabled, const std::wstring& associatedModule)
		: mHookName(hook_name), mAssociatedModule(associatedModule), mShouldBeEnabled(startEnabled)
	{}

	// common members
	void* installed_at();
	const std::wstring& hook_name();
	const std::wstring& associated_module();
	bool get_WantsToBeEnabled();
	void set_WantsToBeEnabled(bool value);
	void update_state();
};

class InlineHook : public Hook {
private:
	MultilevelPointer* mOriginalFunction = nullptr;
	void* mHookFunction;
	safetyhook::InlineHook mInlineHook;

public:
	InlineHook(const std::wstring& hook_name, MultilevelPointer* original_func, void* new_func, bool startEnabled = false, std::wstring associatedModule = L"")
		: Hook(hook_name, startEnabled, associatedModule), mOriginalFunction(original_func), mHookFunction(new_func)
	{}
	
	~InlineHook() = default;

	void attach() final;
	void detach() final;

	safetyhook::InlineHook& getInlineHook();


private:
	void hook_install(void* old_func, void* new_func);
};




class MidHook : public Hook {
private:
	MultilevelPointer* mOriginalFunction = nullptr;
	safetyhook::MidHookFn mHookFunction;
	safetyhook::MidHook mMidHook;

public:
	// TODO: replace MidHookFn type with a template or std::function
	MidHook(const std::wstring& hook_name, MultilevelPointer* original_func, safetyhook::MidHookFn new_func, bool startEnabled = false, std::wstring associatedModule = L"")
		: Hook(hook_name, startEnabled, associatedModule), mOriginalFunction(original_func), mHookFunction(new_func)
	{}

	~MidHook() = default;

	void attach() final;
	void detach() final;


private:
	void hook_install(void* old_func, safetyhook::MidHookFn new_func);
};

#pragma once

//#include <cinttypes>
//#include "pointer.h"
#include "multilevel_pointer.h"
class IHook {
protected:
	std::wstring mHookName;
	std::wstring mAssociatedModule; // used to know if a hook should be reattached or attached on dll load/unload
	void* mInstalledAt = nullptr;
	bool mShouldBeEnabled = false;
	bool mInstalled = false;
public:
	// Contracts
	virtual void attach() { PLOG_DEBUG << "this should always be overridden"; };
	virtual void detach() { PLOG_DEBUG << "this should always be overridden"; };

	IHook(const std::wstring& hook_name, bool startEnabled, std::wstring associatedModule)
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

class inline_hook : public IHook {
private:

	MultilevelPointer* mOriginalFunction = nullptr;
	void* mHookFunction;
	safetyhook::InlineHook mInlineHook;



public:
	inline_hook(const std::wstring& hook_name, multilevel_pointer* original_func, void* new_func, bool startEnabled = false, std::wstring associatedModule = L"")
		: IHook(hook_name, startEnabled, associatedModule), mOriginalFunction(original_func), mHookFunction(new_func)
	{}
	
	~inline_hook() = default;

	void attach();
	void detach();

	safetyhook::InlineHook& getInlineHook();


private:
	void hook_install(void* old_func, void* new_func);
};




class mid_hook : public IHook {
private:


	MultilevelPointer* mOriginalFunction = nullptr;
	safetyhook::MidHookFn mHookFunction;
	safetyhook::MidHook mMidHook;



public:

	mid_hook(const std::wstring& hook_name, multilevel_pointer* original_func, safetyhook::MidHookFn new_func, bool startEnabled = false, std::wstring associatedModule = L"")
		: IHook(hook_name, startEnabled, associatedModule), mOriginalFunction(original_func), mHookFunction(new_func)
	{}

	~mid_hook() = default;

	void attach();
	void detach();


private:
	void hook_install(void* old_func, safetyhook::MidHookFn new_func);
};

#pragma once
#include "multilevel_pointer.h"



// Base class with management stuff
class Hook {
private:
	std::wstring mHookName;
	std::wstring mAssociatedModule; // used to know if a hook should be reattached or attached on dll load/unload
	bool mShouldBeEnabled = false;
	bool mInstalled = false;


protected:
	Hook(const std::wstring& hook_name, bool startEnabled, const std::wstring& associatedModule)
		: mHookName(hook_name), mAssociatedModule(associatedModule), mShouldBeEnabled(startEnabled)
	{}

	virtual ~Hook() = default; // child classes will override to detach their hook objects

	bool getInstalled() const { return mInstalled; }
	void setInstalled(bool val) { mInstalled = val; }

public:
	// Contracts - must be overridden
	virtual void attach() = 0;
	virtual void detach() = 0;
	virtual void* getHookInstallLocation() const = 0;

	// Common functions
	const std::wstring& getHookName();
	const std::wstring& getAssociatedModule();
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
	
	 //~InlineHook() final { detach(); }
	 InlineHook(const InlineHook&) = delete;
	 InlineHook& operator=(const InlineHook&) = delete;
	void attach() final;
	void detach() final;


	safetyhook::InlineHook& getInlineHook();

	void* getHookInstallLocation() const final;

private:
	void hook_install(void* old_func, void* new_func);
};




class MidHook : public Hook {

private:
	MultilevelPointer* mOriginalFunction = nullptr;
	safetyhook::MidHookFn mHookFunction;
	safetyhook::MidHook mMidHook;

public:
	MidHook(const std::wstring& hook_name, MultilevelPointer* original_func, safetyhook::MidHookFn new_func, bool startEnabled = false, std::wstring associatedModule = L"")
		: Hook(hook_name, startEnabled, associatedModule), mOriginalFunction(original_func), mHookFunction(new_func)
	{}

	//~MidHook() final { detach(); }
	MidHook(const MidHook&) = delete;
	MidHook& operator=(const MidHook&) = delete;

	void attach() final;
	void detach() final;

	void* installed_at();

	void* getHookInstallLocation() const final;

private:
	void hook_install(void* old_func, safetyhook::MidHookFn new_func);
};

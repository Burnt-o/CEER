#include "pch.h"
#include "hook.h"
//#include "pointer.h"
#include "dll_cache.h"





const std::wstring& IHook::hook_name()
{
	return this->mHookName;
}


const std::wstring& IHook::associated_module()
{
	return this->mAssociatedModule;
}




void inline_hook::hook_install(void* old_func, void* new_func)
{
	// Acquire the factory's builder which will freeze all threads and give
	// us access to the hook creation methods.
	auto builder = SafetyHookFactory::acquire();

	this->mInlineHook = builder.create_inline(old_func, new_func);

	// Once we leave this scope, builder will unfreeze all threads and our
	// factory will be kept alive by member mInlineHook.
}

void mid_hook::hook_install(void* old_func, safetyhook::MidHookFn new_func)
{
	auto builder = SafetyHookFactory::acquire();
	this->mMidHook = builder.create_mid(old_func, new_func);
}


void inline_hook::attach()
{
	PLOG_DEBUG << "inline_hook attempting attach: " << this->mHookName;
	PLOG_VERBOSE << "hookFunc " << this->mHookFunction;
	if (this->mInstalled) {
		PLOG_DEBUG << "attach failed: hook already installed";
		return;
	}

	if (this->associated_module() != L"" && !dll_cache::module_in_cache(this->associated_module()))
	{
		PLOG_DEBUG << "attach failed: associated module wasn't in cache, " << this->associated_module();
		return;
	}

	if (this->mOriginalFunction == nullptr)
	{
		PLOG_DEBUG << "attach failed: no pointer to original function";
		return;
	}


	void* pOriginalFunction;
	if (!this->mOriginalFunction->resolve(&pOriginalFunction))
	{
		PLOG_ERROR << "attach failed: pOriginalFunction pointer failed to resolve";
		return;
	}

	PLOG_VERBOSE << "pOriginalFunction " << pOriginalFunction;

	hook_install(pOriginalFunction, this->mHookFunction); // need to pass latter by ref?
	this->mInstalledAt = pOriginalFunction;
	this->mInstalled = true;
	PLOG_DEBUG << "inline_hook successfully attached: " << this->mHookName;


}

void mid_hook::attach()
{
	PLOG_DEBUG << "mid_hook attempting attach: " << this->mHookName;
	if (this->mInstalled) {
		PLOG_DEBUG << "attach failed: hook already installed";
		return;
	}

	if (this->associated_module() != L"" && !dll_cache::module_in_cache(this->associated_module()))
	{
		PLOG_DEBUG << "attach failed: associated module wasn't in cache, " << this->associated_module();
		return;
	}


	if (this->mOriginalFunction == nullptr)
	{
		PLOG_DEBUG << "attach failed: no pointer to original function";
		return;
	}

	void* pOriginalFunction;
	if (!this->mOriginalFunction->resolve(&pOriginalFunction))
	{
		PLOG_ERROR << "attach failed: pOriginalFunction pointer failed to resolve";
		return;
	}

	PLOG_VERBOSE << "pOriginalFunction " << pOriginalFunction;

	hook_install(pOriginalFunction, this->mHookFunction);
	this->mInstalledAt = pOriginalFunction;
	this->mInstalled = true;

	PLOG_DEBUG << "mid_hook successfully attached: " << this->mHookName;
	PLOG_VERBOSE << "originalFunc " << this->mInstalledAt;
	PLOG_VERBOSE << "replacedFunc " << *this->mHookFunction;
}

void inline_hook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->mHookName;
	if (!this->mInstalled) {
		PLOG_DEBUG << "already detached";
		return;
	}

	// Should I add a check for pOriginalFunction being valid here?

	this->mInstalledAt = nullptr;
	this->mInstalled = false;
	this->mInlineHook = {};
}

void mid_hook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->mHookName;
	if (!this->mInstalled) {
		PLOG_DEBUG << "already detached";
		return;
	}

	// Should I add a check for pOriginalFunction being valid here?
	this->mInstalledAt = nullptr;
	this->mInstalled = false;
	this->mMidHook = {};
	PLOG_DEBUG << "successfully detached " << this->mHookName;
}





void* IHook::installed_at()
{
	return this->mInstalledAt;
}



void IHook::update_state()
{
	if (this->mShouldBeEnabled)
	{
		this->attach();
	}
	else
	{
		this->detach();
	}
}



void IHook::set_WantsToBeEnabled(bool value)
{
	if (this->mShouldBeEnabled != value)
	{
		this->mShouldBeEnabled = value;
		this->update_state();
	}
}




bool IHook::get_WantsToBeEnabled()
{
	return this->mShouldBeEnabled;
}


safetyhook::InlineHook& inline_hook::getInlineHook()
{
	return this->mInlineHook;
}
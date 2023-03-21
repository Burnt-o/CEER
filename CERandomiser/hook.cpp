#include "pch.h"
#include "Hook.h"
//#include "pointer.h"
#include "ModuleCache.h"





const std::wstring& Hook::getHookName()
{
	return this->mHookName;
}


const std::wstring& Hook::getAssociatedModule()
{
	return this->mAssociatedModule;
}




void InlineHook::hook_install(void* old_func, void* new_func)
{
	// Acquire the factory's builder which will freeze all threads and give
	// us access to the hook creation methods.
	auto builder = SafetyHookFactory::acquire();

	this->mInlineHook = builder.create_inline(old_func, new_func);

	// Once we leave this scope, builder will unfreeze all threads and our
	// factory will be kept alive by member mInlineHook.
}

void MidHook::hook_install(void* old_func, safetyhook::MidHookFn new_func)
{
	auto builder = SafetyHookFactory::acquire();
	this->mMidHook = builder.create_mid(old_func, new_func);
}


void InlineHook::attach()
{
	PLOG_DEBUG << "inline_hook attempting attach: " << getHookName();
	PLOG_VERBOSE << "hookFunc " << this->mHookFunction;
	if (this->getInstalled()) {
		PLOG_DEBUG << "attach failed: hook already installed";
		return;
	}

	if (this->getAssociatedModule() != L"" && !ModuleCache::isModuleInCache(this->getAssociatedModule()))
	{
		PLOG_DEBUG << "attach failed: associated module wasn't in cache, " << this->getAssociatedModule();
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

	this->setInstalled(true);
	PLOG_DEBUG << "inline_hook successfully attached: " << this->getHookName();


}

void MidHook::attach()
{
	PLOG_DEBUG << "mid_hook attempting attach: " << this->getHookName();
	if (this->getInstalled()) {
		PLOG_DEBUG << "attach failed: hook already installed";
		return;
	}

	if (this->getAssociatedModule() != L"" && !ModuleCache::isModuleInCache(this->getAssociatedModule()))
	{
		PLOG_DEBUG << "attach failed: associated module wasn't in cache, " << this->getAssociatedModule();
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
	this->setInstalled(true);

	PLOG_DEBUG << "mid_hook successfully attached: " << this->getHookName();
	PLOG_VERBOSE << "originalFunc " << pOriginalFunction;
	PLOG_VERBOSE << "replacedFunc " << *this->mHookFunction;
}

void InlineHook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->getHookName();
	if (!this->getInstalled()) {
		PLOG_DEBUG << "already detached";
		return;
	}

	// Should I add a check for pOriginalFunction being valid here?


	this->setInstalled(false);
	this->mInlineHook = {};
}

void MidHook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->getHookName();
	if (!this->getInstalled()) {
		PLOG_DEBUG << "already detached";
		return;
	}

	// Should I add a check for pOriginalFunction being valid here?

	this->setInstalled(false);
	this->mMidHook = {};
	PLOG_DEBUG << "successfully detached " << this->getHookName();
}



void* InlineHook::getHookInstallLocation() const
{
	if (!getInstalled()) return nullptr;
	return (void*)this->mInlineHook.target();
}

void* MidHook::getHookInstallLocation() const
{
	if (!getInstalled()) return nullptr;
	return (void*)this->mMidHook.target();
}



void Hook::update_state()
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



void Hook::set_WantsToBeEnabled(bool value)
{
	if (this->mShouldBeEnabled != value)
	{
		this->mShouldBeEnabled = value;
		this->update_state();
	}
}




bool Hook::get_WantsToBeEnabled()
{
	return this->mShouldBeEnabled;
}


safetyhook::InlineHook& InlineHook::getInlineHook() 
{
	return this->mInlineHook;
}
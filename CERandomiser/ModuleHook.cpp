#include "pch.h"
#include "ModuleHook.h"
//#include "pointer.h"
#include "ModuleCache.h"




// The hook name is just used for logging (ie logging attach/detach events)
const std::wstring& ModuleHookBase::getHookName() const
{
	return this->mHookName;
}




void ModuleInlineHook::attach()
{
	PLOG_DEBUG << "inline_hook attempting attach: " << getHookName();
	PLOG_VERBOSE << "hookFunc " << this->mHookFunction;
	if (this->isHookInstalled()) {
		PLOG_DEBUG << "attach failed: hook already installed";
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

	this->mInlineHook = safetyhook::create_inline(pOriginalFunction, this->mHookFunction);

	PLOG_DEBUG << "inline_hook successfully attached: " << this->getHookName();

}

void ModuleMidHook::attach()
{
	PLOG_DEBUG << "mid_hook attempting attach: " << this->getHookName();
	if (this->isHookInstalled()) {
		PLOG_DEBUG << "attach failed: hook already installed";
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

	this->mMidHook = safetyhook::create_mid(pOriginalFunction, this->mHookFunction);

	PLOG_DEBUG << "mid_hook successfully attached: " << this->getHookName();
}

void ModuleInlineHook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->getHookName();
	if (!this->isHookInstalled()) {
		PLOG_DEBUG << "already detached";
		return;
	}

	this->mInlineHook = {};
	PLOG_DEBUG << "successfully detached " << this->getHookName();
}

void ModuleMidHook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->getHookName();
	if (!this->isHookInstalled()) {
		PLOG_DEBUG << "already detached";
		return;
	}

	this->mMidHook = {};
	PLOG_DEBUG << "successfully detached " << this->getHookName();
}





void ModuleHookBase::updateHookState()
{
	if (this->mWantsToBeAttached)
	{
		this->attach();
	}
	else
	{
		this->detach();
	}
}


// Getter/setter for mWantsToBeAttached
void ModuleHookBase::setWantsToBeAttached(bool value)
{
	if (this->mWantsToBeAttached != value)
	{
		this->mWantsToBeAttached = value;
		this->updateHookState();
	}
}


bool ModuleHookBase::getWantsToBeAttached() const
{
	return this->mWantsToBeAttached;
}

// Gets a ref to the safetyhook object, mainly used for calling the original function from within the new function
safetyhook::InlineHook& ModuleInlineHook::getInlineHook() 
{
	return this->mInlineHook;
}



bool ModuleInlineHook::isHookInstalled() const
{
	return this->mInlineHook.operator bool();
}

bool ModuleMidHook::isHookInstalled() const
{
	return this->mMidHook.operator bool();
}
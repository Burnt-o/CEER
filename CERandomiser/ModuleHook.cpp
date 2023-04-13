#include "pch.h"
#include "ModuleHook.h"
//#include "pointer.h"
#include "ModuleCache.h"
#include "ModuleHookManager.h"

std::unique_ptr<ModuleInlineHook> ModuleInlineHook::make(const std::wstring associatedModule, std::shared_ptr<MultilevelPointer> original_func, void* new_func, bool startEnabled)
{
	auto ptr = std::unique_ptr< ModuleInlineHook>(new ModuleInlineHook(associatedModule, original_func, new_func, startEnabled));
	ModuleHookManager::addHook(associatedModule, ptr.get());
	return ptr;

}

std::unique_ptr<ModuleMidHook> ModuleMidHook::make(const std::wstring associatedModule, std::shared_ptr<MultilevelPointer> original_func, safetyhook::MidHookFn new_func, bool startEnabled)
{
	auto ptr = std::unique_ptr< ModuleMidHook>(new ModuleMidHook(associatedModule, original_func, new_func, startEnabled));
	ModuleHookManager::addHook(associatedModule, ptr.get());
	return ptr;
}


const std::wstring& ModuleHookBase::getAssociatedModule() const
{
	return this->mAssociatedModule;
}

ModuleInlineHook::~ModuleInlineHook()
{
	detach();
	ModuleHookManager::removeHook(getAssociatedModule(), this);

}

ModuleMidHook::~ModuleMidHook()
{
	detach();
	ModuleHookManager::removeHook(getAssociatedModule(), this);
}


void ModuleInlineHook::attach()
{
	PLOG_DEBUG << "inline_hook attempting attach: " << getAssociatedModule();
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

	PLOG_DEBUG << "inline_hook successfully attached: " << this->getAssociatedModule();

}

void ModuleMidHook::attach()
{
	PLOG_DEBUG << "mid_hook attempting attach: " << this->getAssociatedModule();
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

	PLOG_DEBUG << "mid_hook successfully attached: " << this->getAssociatedModule();
}

void ModuleInlineHook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->getAssociatedModule();
	if (!this->isHookInstalled()) {
		PLOG_DEBUG << "already detached";
		return;
	}

	this->mInlineHook = {};
	PLOG_DEBUG << "successfully detached " << this->getAssociatedModule();
}

void ModuleMidHook::detach()
{
	PLOG_DEBUG << "detaching hook: " << this->getAssociatedModule();
	if (!this->isHookInstalled()) {
		PLOG_DEBUG << "already detached";
		return;
	}

	this->mMidHook = {};
	PLOG_DEBUG << "successfully detached " << this->getAssociatedModule();
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
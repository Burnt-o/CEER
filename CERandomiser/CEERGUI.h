#pragma once
#include "ImGuiManager.h"
class CEERGUI
{
private:
	// constructor and destructor private
	explicit CEERGUI() = default;
	~CEERGUI() = default;
	// singleton accessor
	static CEERGUI& get() {
		static CEERGUI instance;
		return instance;
	}

	static inline std::mutex mDestructionGuard; // Protects against CEERGUI singleton destruction while callbacks are executing

	static void onImGuiRenderCallback();
	bool m_OptionsGUIinitialized = false;
	static void initializeCEERGUI();
	static eventpp::CallbackList<void()>::Handle mCallbackHandle;


	ImGuiWindowFlags windowFlags;



	bool m_WindowOpen = true;


public:
	static void initialize() // Listen to ImGuiRenderCallback
	{
		mCallbackHandle = ImGuiManager::get().ImGuiRenderCallback.append(onImGuiRenderCallback);
	}
	static void release() // release resources
	{
		if (mCallbackHandle)
		{
			ImGuiManager::get().ImGuiRenderCallback.remove(mCallbackHandle);
			mCallbackHandle = {};
		}
	}
	static void destroy() // destroys the singleton
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard); // onPresentHookCallback also locks this
		release();
		get().~CEERGUI();
	}
};


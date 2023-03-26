#pragma once
#include "ImGuiManager.h"
class OptionsGUI
{
private:
	// constructor and destructor private
	explicit OptionsGUI() = default;
	~OptionsGUI() = default;
	// singleton accessor
	static OptionsGUI& get() {
		static OptionsGUI instance;
		return instance;
	}

	static inline std::mutex mDestructionGuard; // Protects against OptionsGUI singleton destruction while callbacks are executing

	static void onImGuiRenderCallback();
	bool m_OptionsGUIinitialized = false;
	static void initializeOptionsGUI();

	ImGuiWindowFlags windowFlags;

public:
	static void initialize(ImGuiManager& im) // Listen to ImGuiRenderCallback
	{
		im.ImGuiRenderCallback.append(onImGuiRenderCallback);
	}
	static void release(); // release resources
	static void destroy() // destroys the singleton
	{
		std::scoped_lock<std::mutex> lock(mDestructionGuard); // onPresentHookCallback also locks this
		release();
		get().~OptionsGUI();
	}
};


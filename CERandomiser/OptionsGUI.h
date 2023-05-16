#pragma once
#include "ImGuiManager.h"

// Singleton: on construction we subscribe to the ImGuiRenderEvent.
// In onImGuiRenderEvent we draw our options GUI.
// destructor just unsubscribes from ImGuiRenderEvent.
class OptionsGUI
{
private:
	static OptionsGUI* instance; // Private Singleton instance so static hooks/callbacks can access
	std::mutex mDestructionGuard; // Protects against Singleton destruction while callbacks are executing

	// ImGuiManager Event reference and our handle to the append so we can remove it in destructor
	eventpp::CallbackList<void()>& pImGuiRenderEvent;
	eventpp::CallbackList<void()>::Handle mCallbackHandle = {};

	// What we run when ImGuiManager ImGuiRenderEvent is invoked
	static void onImGuiRenderEvent();

	// initialize resources in the first onImGuiRenderEvent
	void initializeCEERGUI();
	bool m_OptionsGUIinitialized = false;


	void renderOptionsGUI(); // Primary render function

	// sub-render functions to help my sanity
	std::vector<CEERExceptionBase> errorsToDisplay;
	void renderErrorDialog();
	void renderAboutWindow();


	void renderAddSpawnMultiplierRulePopup();

	void renderManageCustomGroupsDialog();
	void renderEnemyRandomiserRules();
	void renderEnemySpawnMultiplierRules();
	

	// GUI data
	ImGuiWindowFlags windowFlags;
	static bool m_WindowOpen;

public:

	// Gets passed ImGuiManager ImGuiRenderEvent reference so we can subscribe and unsubscribe
	explicit OptionsGUI(eventpp::CallbackList<void()>& pEvent) : pImGuiRenderEvent(pEvent)//
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one OptionsGUI");
		}
		instance = this;

		mCallbackHandle = pImGuiRenderEvent.append(onImGuiRenderEvent);
	}

	~OptionsGUI(); // release resources

	static void addError(CEERExceptionBase error) { if (instance->errorsToDisplay.size() < 5) instance->errorsToDisplay.push_back(error); }
	static bool isWindowOpen() { return m_WindowOpen; };
};


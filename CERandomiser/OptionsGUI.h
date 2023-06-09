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
	eventpp::CallbackList<void()>::Handle mImGuiRenderCallbackHandle = {};

	// resize Window event and handle
	eventpp::CallbackList<void(ImVec2)>& pWindowResizeEvent;
	eventpp::CallbackList<void(ImVec2)>::Handle mWindowResizeCallbackHandle = {};

	// What we run when ImGuiManager ImGuiRenderEvent is invoked
	static void onImGuiRenderEvent();
	static void onWindowResizeEvent(ImVec2 newScreenSize);

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

	void renderTextureSeizureWarning();
	void renderHighMultiplierWarning();
	void renderEmptySeedWarning();
	void renderMissingRulesWarning();

	// GUI data
	ImGuiWindowFlags windowFlags;
	static bool m_WindowOpen;
	ImVec2 mWindowSize{ 500, 500 };
	ImVec2 mWindowPos{ 10, 10 };

public:

	// Gets passed ImGuiManager ImGuiRenderEvent reference so we can subscribe and unsubscribe
	explicit OptionsGUI(eventpp::CallbackList<void()>& pRenderEvent, eventpp::CallbackList<void(ImVec2)>& pResizeEvent) : pImGuiRenderEvent(pRenderEvent), pWindowResizeEvent(pResizeEvent)
	{
		if (instance != nullptr)
		{
			throw InitException("Cannot have more than one OptionsGUI");
		}
		instance = this;

		mImGuiRenderCallbackHandle = pImGuiRenderEvent.append(onImGuiRenderEvent);
		mWindowResizeCallbackHandle = pWindowResizeEvent.append(onWindowResizeEvent);
	}

	~OptionsGUI(); // release resources

	static void addError(CEERExceptionBase error) { if (instance->errorsToDisplay.size() < 5) instance->errorsToDisplay.push_back(error); }
	static bool isWindowOpen() { return m_WindowOpen; };

	static float getOptionsGUIHeight() { return instance->mWindowSize.y; };
};


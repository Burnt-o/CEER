#include "pch.h"
#include "OptionsGUI.h"
#include "GlobalKill.h"
#include "OptionsState.h"
OptionsGUI* OptionsGUI::instance = nullptr;


OptionsGUI::~OptionsGUI()
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard); // onPresentHookCallback also locks this
	if (mCallbackHandle && pImGuiRenderEvent)
	{
		pImGuiRenderEvent.remove(mCallbackHandle);
		mCallbackHandle = {};
	}

	instance = nullptr;
}




void OptionsGUI::initializeCEERGUI()
{
	ImGui::SetNextWindowCollapsed(false);
	windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
}


void OptionsGUI::onImGuiRenderEvent()
{
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard); 

#pragma region init
	//PLOG_VERBOSE << "CEERGUI::onImGuiRenderCallback()";
	if (!instance->m_OptionsGUIinitialized)
	{
		PLOG_INFO << "Initializing OptionsGUI";
		try
		{
			instance->initializeCEERGUI();
			instance->m_OptionsGUIinitialized = true;
		}
		catch (InitException& ex)
		{
			PLOG_FATAL << "Failed to initialize OptionsGUI, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			GlobalKill::killMe();
			return;
		}
	}
#pragma endregion init

	instance->renderOptionsGUI();

}


void OptionsGUI::renderOptionsGUI()
{

	ImGui::SetNextWindowSize(ImVec2(300, 300));
	ImGui::SetNextWindowPos(ImVec2(10, 10));

	if (ImGui::Begin("CE Enemy Randomiser!", NULL, windowFlags))  // Create window, only bother rendering children if it's not collapsed
	{
		ImGui::Text("This is some useful text.");


		if (ImGui::Checkbox("Master Toggle", &OptionsState::MasterToggle.GetValueDisplay()))
		{
			OptionsState::MasterToggle.UpdateValueWithInput();
		}

		if (ImGui::Checkbox("Enable Enemy Randomiser", &OptionsState::EnemyRandomiser.GetValueDisplay()))
		{
			OptionsState::EnemyRandomiser.UpdateValueWithInput();
		}

	}



	ImGui::End();
}
#include "pch.h"
#include "OptionsGUI.h"
#include "global_kill.h"

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
	ImGui::SetWindowSize(ImVec2(300, 300));
	windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
}


void OptionsGUI::onImGuiRenderCallback()
{
	std::scoped_lock<std::mutex> lock(instance->mDestructionGuard); 

#pragma region init
	//PLOG_VERBOSE << "CEERGUI::onImGuiRenderCallback()";
	if (instance->m_OptionsGUIinitialized)
	{
		PLOG_INFO << "Initializing OptionsGUI";
		try
		{
			instance->initializeCEERGUI();
			instance->m_OptionsGUIinitialized = true;
		}
		catch (expected_exception& ex)
		{
			PLOG_FATAL << "Failed to initialize OptionsGUI, info: " << std::endl
				<< ex.what() << std::endl
				<< "CEER will now automatically close down";
			global_kill::kill_me();
			return;
		}
	}
#pragma endregion init

	instance->renderOptionsGUI();

}


bool test_checkbox = false;
float test_slider = 0.3f;

struct rgb {
	float r, g, b;
};

int test_buttonPressCount = 0;

rgb test_coloredit;


void OptionsGUI::renderOptionsGUI()
{

	ImGui::SetNextWindowSize(ImVec2(300, 300));
	bool windowOpen = ImGui::Begin("CE Enemy Randomiser!", NULL, windowFlags);                          // Create a window called "Hello, world!" and append into it.



	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &test_checkbox);      // Edit bools storing our window open/close state


	ImGui::SliderFloat("float", &test_slider, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&test_coloredit); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		test_buttonPressCount++;

	ImGui::SameLine();
	ImGui::Text("test_buttonPressCount = %d", test_buttonPressCount);


	ImGui::End();
}
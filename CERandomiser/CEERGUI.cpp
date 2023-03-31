#include "pch.h"
#include "CEERGUI.h"
#include "global_kill.h"

bool test_checkbox = false;
float test_slider = 0.3f;
eventpp::CallbackList<void()>::Handle CEERGUI::mCallbackHandle = {};

struct rgb {
	float r, g, b;
};

int test_buttonPressCount = 0;

rgb test_coloredit;
void CEERGUI::initializeCEERGUI()
{

	//ImGui::SetNextWindowCollapsed(false);
	ImGui::SetWindowSize(ImVec2(300, 300));
	get().windowFlags =  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
}


void CEERGUI::onImGuiRenderCallback()
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard); // CEERGUI::destroy also locks this
	//PLOG_VERBOSE << "CEERGUI::onImGuiRenderCallback()";
	if (!get().m_OptionsGUIinitialized)
	{
		PLOG_INFO << "Initializing OptionsGUI";
		try
		{
			initializeCEERGUI();
			get().m_OptionsGUIinitialized = true;
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






	ImGui::SetNextWindowSize(ImVec2(300, 300));
	bool windowOpen = ImGui::Begin("CE Enemy Randomiser!", NULL, get().windowFlags);                          // Create a window called "Hello, world!" and append into it.



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
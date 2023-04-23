#include "pch.h"
#include "ImGuiDialogBox.h"




void ErrorMessageBox::onImGuiRenderEvent()
{
	ImVec2 windowSize(400, 400);
	ImGui::SetNextWindowSize(windowSize);

	ImVec2 renderPosition = (ImGuiManager::getScreenSize() / 2.f) - (windowSize / 2.f);

	ImGui::SetNextWindowPos(renderPosition); // Center screen
	ImGui::Begin("Error!", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);  // Create window, only bother rendering children if it's not collapsed
	{
		ImGui::Text(mErrorMessage.c_str());

		if (ImGui::Button("OK"))
		{
			delete this;
		}

	}
	ImGui::End();
}


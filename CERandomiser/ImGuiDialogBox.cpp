#include "pch.h"
#include "ImGuiDialogBox.h"




void ErrorMessageBox::onImGuiRenderEvent()
{
	ImGui::SetNextWindowSize(ImVec2(300, 300));

	ImGui::Begin("Error!", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);  // Create window, only bother rendering children if it's not collapsed
	{
		ImGui::Text(mErrorMessage.c_str());

		if (ImGui::Button("OK"))
		{
			delete this;
		}

	}
}


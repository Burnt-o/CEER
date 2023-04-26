#include "pch.h"
#include "OptionsGUI.h"
#include "GlobalKill.h"
#include "OptionsState.h"

#include "EnemyRandomiserRule.h"
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


static bool errorDialogShowing = false;
void OptionsGUI::renderErrorDialog()
{
	if (errorsToDisplay.empty())
	{
		return;
	}

	if (!errorDialogShowing)
	{
		ImGui::OpenPopup("Error!");
		errorDialogShowing = true;
	}

	ImVec2 size = ImVec2(300, 0); 
	ImVec2 pos = ImVec2(ImGuiManager::getScreenSize() / 2.f) - (size / 2.f);

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(pos);

	if (ImGui::BeginPopupModal("Error!", NULL, ImGuiWindowFlags_NoResize))
	{

		ImGui::TextWrapped("CEER encountered an error!");
		ImGui::TextWrapped(errorsToDisplay.front().what().c_str());
		ImGui::TextWrapped("");
		ImGui::TextWrapped("Nerdy info for Burnt: ");

			ImGui::BeginChild("trace", ImVec2(ImGui::GetWindowSize().x - 15, 150), NULL);
			ImGui::TextWrapped("The error occured here:");
			ImGui::TextWrapped(errorsToDisplay.front().source().c_str());
			ImGui::TextWrapped("");
			ImGui::TextWrapped("Full stack trace:");
			ImGui::TextWrapped(errorsToDisplay.front().trace().c_str());
			ImGui::EndChild();

		if (ImGui::Button("OK"))
		{
			errorDialogShowing = false;
			errorsToDisplay.erase(errorsToDisplay.begin());
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Copy to clipboard"))
		{
			std::string copyText = errorsToDisplay.front().what() + "\n\n" + errorsToDisplay.front().source() + "\n\n" + errorsToDisplay.front().trace();
			ImGui::SetClipboardText(copyText.c_str());
		}
		ImGui::EndPopup();
	}

}

void OptionsGUI::renderAddRuleDialog()
{
	if (ImGui::BeginPopupModal("AddRuleDialog", NULL, ImGuiWindowFlags_NoResize))
	{
		if (ImGui::Button("Add default test rule"))
		{
			OptionsState::currentRules.emplace_back(new RandomiseXintoY());
			PLOG_DEBUG << "added default test rule, currentRuleList size: " << OptionsState::currentRules.size();
			ImGui::CloseCurrentPopup();
		}


		ImGui::EndPopup();
	}
}

void OptionsGUI::renderManageCustomGroupsDialog()
{
	if (ImGui::BeginPopupModal("ManageCustomGroupsDialog", NULL, ImGuiWindowFlags_NoResize))
	{
		ImGui::EndPopup();
	}
}


void OptionsGUI::renderOptionsGUI()
{
	// Dialogs
	renderErrorDialog();




#pragma region MainWindow
	// Main window
	ImGui::SetNextWindowSize(ImVec2(500, 500));
	ImGui::SetNextWindowPos(ImVec2(10, 10));

	if (ImGui::Begin("CE Enemy Randomiser!", NULL, windowFlags))  // Create window, only bother rendering children if it's not collapsed
	{
#pragma region Seed

		if (OptionsState::AutoGenerateSeed.GetValue())
			ImGui::BeginDisabled();

		if (ImGui::InputText("Seed", &OptionsState::SeedString.GetValueDisplay()))
		{
			OptionsState::SeedString.UpdateValueWithInput();
		}

		if (OptionsState::AutoGenerateSeed.GetValue())
			ImGui::EndDisabled();

		ImGui::SameLine();

		if (ImGui::Checkbox("Set Seed", &OptionsState::AutoGenerateSeed.GetValueDisplay()))
		{
			OptionsState::AutoGenerateSeed.UpdateValueWithInput();
		}
#pragma endregion Seed


		if (ImGui::Checkbox("Master Toggle", &OptionsState::MasterToggle.GetValueDisplay()))
		{
			OptionsState::MasterToggle.UpdateValueWithInput();
		}

#pragma region EnemyRandomiser
		if (ImGui::Checkbox("Enable Enemy Randomiser", &OptionsState::EnemyRandomiser.GetValueDisplay()))
		{
			OptionsState::EnemyRandomiser.UpdateValueWithInput();
		}

		if (ImGui::CollapsingHeader("Enemy Randomiser Settings", ImGui::IsItemHovered()))
		{
			if (ImGui::Button("Add Rule"))
			{
				ImGui::OpenPopup("AddRuleDialog");
			}
			renderAddRuleDialog();
			ImGui::SameLine();
			if (ImGui::Button("Manage Custom Groups"))
			{
				ImGui::OpenPopup("ManageCustomGroupsDialog");
			}
			renderManageCustomGroupsDialog();

			//ImVec4 mask{0.1f,0.1f,0.1f,0.0f};
			//ImVec4 newCol = ImGui::GetStyle().Colors[ImGuiCol_WindowBg] + mask;
			//ImGui::PushStyleColor(ImGuiCol_WindowBg, newCol);
			ImGui::SetNextWindowBgAlpha(-0.2f);
			ImGui::BeginChild("rules", ImVec2(0, 0), true, NULL);
			//for ( std::unique_ptr<EnemyRandomiserRule>& rule : OptionsState::currentRules) // render rules
			for (auto it = OptionsState::currentRules.begin(); auto& rule: OptionsState::currentRules) // render rules
			{
				
				ImGui::BeginChild(ImGui::GetID(rule.get()), ImVec2(0, 60), true, NULL);
				

				// TODO: implement delete/moveup/movedown buttons
				if (ImGui::Button("Delete"))
				{
					OptionsState::currentRules.erase(it);
					ImGui::EndChild();
						break;
				}	ImGui::SameLine();

				if (it == OptionsState::currentRules.begin()) ImGui::BeginDisabled(); // if at start, disable move up button

				if (ImGui::Button("Move Up"))
				{
					std::swap(*it, *(it - 1));
					ImGui::EndChild();
					break;
				}	ImGui::SameLine();
				if (it == OptionsState::currentRules.begin()) ImGui::EndDisabled(); // if at start, disable move up button


				if (it == OptionsState::currentRules.end() - 1) ImGui::BeginDisabled(); // if at end, disable move down button
				if (ImGui::Button("Move Down"))
				{
					std::swap(*it, *(it + 1));
					ImGui::EndChild();
					break;
				}
				if (it == OptionsState::currentRules.end() - 1) ImGui::EndDisabled(); // if at end, disable move down button

				switch (rule->getType())
				{
				case RuleType::RandomiseXintoY:
				{
					RandomiseXintoY* thisRule = dynamic_cast<RandomiseXintoY*>(rule.get());
					assert(thisRule != nullptr);
					ImGui::Text("Turn "); ImGui::SameLine();

					ImGui::SetNextItemWidth(150);
					if (ImGui::BeginCombo("##randomiseGroup", builtInGroups::builtInGroups.at(thisRule->randomiseGroupSelection).getName().data()))
					{
						for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
						{
							const bool is_selected = thisRule->randomiseGroupSelection == n;
							if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
							{
								thisRule->randomiseGroupSelection = n;
							}

							if (is_selected)
							{
								ImGui::SetItemDefaultFocus();
							}

						}
						ImGui::EndCombo();
					} ImGui::SameLine();

					ImGui::Text(" into "); ImGui::SameLine();

					ImGui::SetNextItemWidth(150);
					if (ImGui::BeginCombo("##rollGroup", builtInGroups::builtInGroups.at(thisRule->rollGroupSelection).getName().data()))
					{
						for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
						{
							const bool is_selected = thisRule->rollGroupSelection == n;
							if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
							{
								thisRule->rollGroupSelection = n;
							}

							if (is_selected)
							{
								ImGui::SetItemDefaultFocus();
							}

						}
						ImGui::EndCombo();
					} 
				}
				default:
					break;
				}

				ImGui::EndChild();
				it++;
			}
			ImGui::EndChild();

			static bool blah = false;
			ImGui::Checkbox("blah", &blah);

		} // End enemy randomiser settings

		ImGui::Text("Texture rando todo");

#pragma endregion EnemyRandomiser
	}
	ImGui::End(); // end main window

#pragma endregion MainWindow




}
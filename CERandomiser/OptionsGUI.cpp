#include "pch.h"
#include "OptionsGUI.h"
#include "GlobalKill.h"
#include "OptionsState.h"

#include "EnemyRule.h"
#include "OptionSerialisation.h"

bool OptionsGUI::m_WindowOpen = true;
OptionsGUI* OptionsGUI::instance = nullptr;


std::map<RuleType, float> ruleTypeToPixelHeight
{
	{RuleType::RandomiseXintoY, 125.f},
	{RuleType::SpawnMultiplierPreRando, 115.f},
		{RuleType::SpawnMultiplierPostRando, 115.f}
};

std::map<RuleType, std::string> ruleTypeToRuleTypeName
{
	{RuleType::RandomiseXintoY, "RandomiseXintoY"},
	{RuleType::SpawnMultiplierPreRando, "Spawn Rate (pre-rando)"},
		{RuleType::SpawnMultiplierPostRando, "Spawn Rate (post-rando)"}
};

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


void OptionsGUI::renderAboutWindow()
{
	ImVec2 size = ImVec2(300, 0);
	ImVec2 pos = ImVec2(ImGuiManager::getScreenSize() / 2.f) - (size / 2.f);

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(pos);

	if (ImGui::BeginPopupModal("AboutWindow", NULL, ImGuiWindowFlags_NoResize))
	{
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::TextWrapped("I'll put some text here later");




		ImGui::EndPopup();
	}
}




void OptionsGUI::renderAddSpawnMultiplierRulePopup()
{
	if (ImGui::BeginPopup("AddSpawnMultiplierRulePopup"))
	{
		if (ImGui::Selectable("Spawn Multiplier Pre-Randomisation"))
		{
			OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierPreRando());
		}
		if (ImGui::Selectable("Spawn Multiplier Post-Randomisation"))
		{
			OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierPostRando());
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

void OptionsGUI::renderEnemySpawnMultiplierRules()
{
	ImGui::SetNextWindowBgAlpha(0.1f);

	float rulesWindowHeight = 12.f;
	for (auto& rule : OptionsState::currentMultiplierRules)
	{
		rulesWindowHeight += ruleTypeToPixelHeight[rule.get()->getType()] + 5.f;
	}

	ImGui::BeginChild("MultiplierRules", ImVec2(0, rulesWindowHeight), true, NULL);
	//for ( std::unique_ptr<EnemyRandomiserRule>& rule : OptionsState::currentRules) // render rules
	for (auto it = OptionsState::currentMultiplierRules.begin(); auto & rule: OptionsState::currentMultiplierRules) // render rules
	{

		ImGui::BeginChild(ImGui::GetID(rule.get()), ImVec2(0.f, ruleTypeToPixelHeight[rule.get()->getType()]), true, NULL);

		if (ImGui::Button("Delete"))
		{
			OptionsState::currentMultiplierRules.erase(it);
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();

		if (it == OptionsState::currentMultiplierRules.begin()) ImGui::BeginDisabled(); // if at start, disable move up button

		if (ImGui::Button("Move Up"))
		{
			std::swap(*it, *(it - 1));
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();
		if (it == OptionsState::currentMultiplierRules.begin()) ImGui::EndDisabled(); // if at start, disable move up button


		if (it == OptionsState::currentMultiplierRules.end() - 1) ImGui::BeginDisabled(); // if at end, disable move down button
		if (ImGui::Button("Move Down"))
		{
			std::swap(*it, *(it + 1));
			ImGui::EndChild();
			break;
		} ImGui::SameLine();
		if (it == OptionsState::currentMultiplierRules.end() - 1) ImGui::EndDisabled(); // if at end, disable move down button
		ImGui::Text(" Rule type:"); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), ruleTypeToRuleTypeName[rule.get()->getType()].c_str());



		switch (rule->getType())
		{
		case RuleType::SpawnMultiplierPreRando:
		{
			SpawnMultiplierPreRando* thisRule = dynamic_cast<SpawnMultiplierPreRando*>(rule.get());
			assert(thisRule != nullptr);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Multiply pre-randomisation spawn rate of");

			if (ImGui::BeginCombo("##groupSelection", thisRule->groupSelection.getName().data()))
			{

				ImGui::SeparatorText("General: ");
				for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
				{
					if (n == 3) ImGui::SeparatorText("Faction: ");
					const bool is_selected = &thisRule->groupSelection == &builtInGroups::builtInGroups.at(n);
					if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
					{
						thisRule->groupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
			ImGui::AlignTextToFramePadding();
			ImGui::Text("by:"); ImGui::SameLine();

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputDouble("##multiplyPercent", &thisRule->multiplier.GetValueDisplay(), 1.0, 10.0, "%.0f"))
			{
				thisRule->multiplier.UpdateValueWithInput();
			}
			ImGui::SameLine(); ImGui::Text("x");

		}
		break;

		case RuleType::SpawnMultiplierPostRando:
		{
			SpawnMultiplierPostRando* thisRule = dynamic_cast<SpawnMultiplierPostRando*>(rule.get());
			assert(thisRule != nullptr);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Multiply post-randomisation spawn rate of");

			if (ImGui::BeginCombo("##groupSelection", thisRule->groupSelection.getName().data()))
			{

				ImGui::SeparatorText("General: ");
				for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
				{
					if (n == 3) ImGui::SeparatorText("Faction: ");
					const bool is_selected = &thisRule->groupSelection == &builtInGroups::builtInGroups.at(n);
					if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
					{
						thisRule->groupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
			ImGui::AlignTextToFramePadding();
			ImGui::Text("by:"); ImGui::SameLine();

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputDouble("##multiplyPercent", &thisRule->multiplier.GetValueDisplay(), 1.0, 10.0, "%.0f"))
			{
				thisRule->multiplier.UpdateValueWithInput();
			}
			ImGui::SameLine(); ImGui::Text("x");

		}
		break;
		default:
			break;
		}

		ImGui::EndChild();
		it++;
	}
	ImGui::EndChild();
}

void OptionsGUI::renderEnemyRandomiserRules()
{

	ImGui::SetNextWindowBgAlpha(0.1f);

	float rulesWindowHeight = 12.f;
	for (auto& rule : OptionsState::currentRandomiserRules)
	{
		rulesWindowHeight += ruleTypeToPixelHeight[rule.get()->getType()] + 5.f;
	}

	ImGui::BeginChild("RandomisationRules", ImVec2(0, rulesWindowHeight), true, NULL);
	//for ( std::unique_ptr<EnemyRandomiserRule>& rule : OptionsState::currentRules) // render rules
	for (auto it = OptionsState::currentRandomiserRules.begin(); auto & rule: OptionsState::currentRandomiserRules) // render rules
	{

		ImGui::BeginChild(ImGui::GetID(rule.get()), ImVec2(0.f, ruleTypeToPixelHeight[rule.get()->getType()]), true, NULL);

		if (ImGui::Button("Delete"))
		{
			OptionsState::currentRandomiserRules.erase(it);
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();

		if (it == OptionsState::currentRandomiserRules.begin()) ImGui::BeginDisabled(); // if at start, disable move up button

		if (ImGui::Button("Move Up"))
		{
			std::swap(*it, *(it - 1));
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();
		if (it == OptionsState::currentRandomiserRules.begin()) ImGui::EndDisabled(); // if at start, disable move up button


		if (it == OptionsState::currentRandomiserRules.end() - 1) ImGui::BeginDisabled(); // if at end, disable move down button
		if (ImGui::Button("Move Down"))
		{
			std::swap(*it, *(it + 1));
			ImGui::EndChild();
			break;
		} ImGui::SameLine();
		if (it == OptionsState::currentRandomiserRules.end() - 1) ImGui::EndDisabled(); // if at end, disable move down button
		ImGui::Text(" Rule type:"); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), ruleTypeToRuleTypeName[rule.get()->getType()].c_str());



		switch (rule->getType())
		{
		case RuleType::RandomiseXintoY:
		{
			RandomiseXintoY* thisRule = dynamic_cast<RandomiseXintoY*>(rule.get());
			assert(thisRule != nullptr);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Randomise"); ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputDouble("##randomisePercent", &thisRule->randomisePercent.GetValueDisplay(), 1.0, 10.0, "%.0f"))
			{
				thisRule->randomisePercent.UpdateValueWithInput();
			} ImGui::SameLine();
			ImGui::Text("percent of:");

			//ImGui::SetNextItemWidth(150);
			if (ImGui::BeginCombo("##randomiseGroup", thisRule->randomiseGroupSelection.getName().data()))
			{
				// If we split this into the different kind of vectors we can easily use seperators, I think
				// plus Custom groups of course

				ImGui::SeparatorText("General: ");
				for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
				{
					if (n == 3) ImGui::SeparatorText("Faction: ");
					const bool is_selected = &thisRule->randomiseGroupSelection == &builtInGroups::builtInGroups.at(n);
					if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
					{
						thisRule->randomiseGroupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}

			ImGui::Text("Into:");

			if (ImGui::BeginCombo("##rollGroup", thisRule->rollGroupSelection.getName().data()))
			{
				for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
				{
					const bool is_selected = &thisRule->rollGroupSelection == &builtInGroups::builtInGroups.at(n);
					if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
					{
						thisRule->rollGroupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
		}
		break;
		
		default:
			break;
		}

		ImGui::EndChild();
		it++;
	}
	ImGui::EndChild();
}


void OptionsGUI::renderOptionsGUI()
{
	// Dialogs
	renderErrorDialog();




#pragma region MainWindow
	// Main window
	ImGui::SetNextWindowSize(ImVec2(500, 500));
	ImGui::SetNextWindowPos(ImVec2(10, 10));

	m_WindowOpen = ImGui::Begin("CE Enemy Randomiser!", nullptr, windowFlags); // Create window

	if (m_WindowOpen)  //only bother rendering children if it's not collapsed
	{

#pragma region Settings/About


		if (ImGui::Button("Copy Settings"))
		{
			try
			{
				auto doc = OptionSerialisation::serialiseAll();
				std::ostringstream oss;
				doc.print(oss);
				std::string str = oss.str();
				ImGui::SetClipboardText(str.c_str());
			}
			catch (SerialisationException& ex)
			{
				RuntimeExceptionHandler::handlePopup(ex);
			}

		} ImGui::SameLine();

		if (ImGui::Button("Paste Settings"))
		{
			try
			{
				auto clipboard = ImGui::GetClipboardText();
				OptionSerialisation::deserialiseAll(clipboard);
			}
			catch (SerialisationException& ex)
			{
				RuntimeExceptionHandler::handlePopup(ex);
			}

		} ImGui::SameLine();

		if (ImGui::Button("About"))
		{
			// modal popup
			ImGui::OpenPopup("AboutWindow");
		}
		renderAboutWindow();

#pragma endregion



#pragma region Seed

		if (ImGui::InputText("Seed", &OptionsState::SeedString.GetValueDisplay()))
		{
			OptionsState::SeedString.UpdateValueWithInput();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Seed determines the outcome of randomisation. Leave blank to have a seed auto-generated for you.");


#pragma endregion Seed



		renderManageCustomGroupsDialog();

#pragma region EnemyRandomiser
		if (ImGui::Checkbox("Enable Enemy Randomiser", &OptionsState::EnemyRandomiser.GetValueDisplay()))
		{
			OptionsState::EnemyRandomiser.UpdateValueWithInput();
		}

		if (ImGui::CollapsingHeader("Enemy Randomiser Settings", ImGui::IsItemHovered()))
		{
			if (ImGui::Button("Add Randomisation Rule"))
			{
				OptionsState::currentRandomiserRules.emplace_back(new RandomiseXintoY());
			}
			ImGui::SameLine();
			if (ImGui::Button("Manage Custom Groups"))
			{
				ImGui::OpenPopup("ManageCustomGroupsDialog");
			}

			renderEnemyRandomiserRules();
		} 
#pragma endregion EnemyRandomiser
#pragma region EnemySpawnMultiplier
		if (ImGui::Checkbox("Enable Enemy Spawn Multiplier", &OptionsState::EnemySpawnMultiplier.GetValueDisplay()))
		{
			OptionsState::EnemySpawnMultiplier.UpdateValueWithInput();
		}

		if (ImGui::CollapsingHeader("Enemy Spawn Multiplier Settings", ImGui::IsItemHovered()))
		{
			if (ImGui::Button("Add Multiplier Rule"))
			{
				ImGui::OpenPopup("AddSpawnMultiplierRulePopup");
			}
			renderAddSpawnMultiplierRulePopup();
			ImGui::SameLine();
			if (ImGui::Button("Manage Custom Groups"))
			{
				ImGui::OpenPopup("ManageCustomGroupsDialog");
			}



			renderEnemySpawnMultiplierRules();



		} 
#pragma endregion EnemySpawnMultiplier
		ImGui::Text("Texture rando todo");


	}
	ImGui::End(); // end main window

#pragma endregion MainWindow




}
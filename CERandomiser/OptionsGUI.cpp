#include "pch.h"
#include "OptionsGUI.h"
#include "GlobalKill.h"
#include "OptionsState.h"

#include "EnemyRule.h"
#include "OptionSerialisation.h"
#include <shellapi.h>


#define addTooltip(x) if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip(x)


bool OptionsGUI::m_WindowOpen = true;
OptionsGUI* OptionsGUI::instance = nullptr;


static bool changesPending_Seed = false;
static bool changesPending_Rand = false;
static bool changesPending_Mult = false;
static bool changesPending_Text = false;
static bool changesPending_Sound = false;


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

std::map<RuleType, std::string> ruleTypeToRuleTypeToolTip
{
	{RuleType::RandomiseXintoY, "Randomises one category of enemies (X) into another category of enemies (Y)"},
	{RuleType::SpawnMultiplierPreRando, "Multiplies the spawn rate of the selected group (before any randomisation)"},
		{RuleType::SpawnMultiplierPostRando, "Multiplies the spawn rate of the selected group (after any randomisation)"}
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

static ImGuiStyle* mainStyle = nullptr;
static ImGuiStyle* highlightStyle = nullptr;





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

void copySettings()
{
	try
	{
		auto doc = OptionSerialisation::serialiseAll();
		std::ostringstream oss;
		doc.print(oss);
		std::string str = "CEERsettings[" + oss.str() + "]";
		std::erase_if(str, [](char x) { return x == '\n' || x == '\t'; });
		ImGui::SetClipboardText(str.c_str());
	}
	catch (SerialisationException& ex)
	{
		RuntimeExceptionHandler::handlePopup(ex);
	}
}

void pasteSettings()
{
	try
	{
		std::string clipboard = ImGui::GetClipboardText();
		if (clipboard.starts_with("CEERsettings[")) clipboard.erase(0, 12);
		if (clipboard.starts_with("[")) clipboard.erase(clipboard.begin());
		if (clipboard.ends_with("]")) clipboard.erase(std::prev(clipboard.end()));

		OptionSerialisation::deserialiseAll(clipboard);
	}
	catch (SerialisationException& ex)
	{
		RuntimeExceptionHandler::handlePopup(ex);
	}
}

void OptionsGUI::renderTextureSeizureWarning()
{

	ImVec2 size = ImVec2(300, 0);
	ImVec2 pos = ImVec2(ImGuiManager::getScreenSize() / 2.f) - (size / 2.f);

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(pos);

	if (ImGui::BeginPopupModal("Seizure Warning", NULL, ImGuiWindowFlags_NoResize))
	{

		ImGui::TextWrapped("Warning! Setting texture re-randomisation to a low (fast) value may potentially trigger seizures for people with photosensitive epilepsy.");
		ImGui::TextWrapped("Most people are unaware that they have this disorder until it strikes.");
		ImGui::TextWrapped("If you experience dizziness, altered vision, eye or muscle twitches, loss of awareness, diorientation, any involuntary movement, or convulsions while using this feature, immediately stop and consult a physician.");

		if (ImGui::Button("Continue"))
		{
			OptionsState::TextureRandomiser.UpdateValueWithInput();
			changesPending_Text = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop"))
		{
			OptionsState::TextureRandomiser.GetValueDisplay() = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void OptionsGUI::renderEmptySeedWarning()
{

	ImVec2 size = ImVec2(300, 0);
	ImVec2 pos = ImVec2(ImGuiManager::getScreenSize() / 2.f) - (size / 2.f);

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(pos);

	if (ImGui::BeginPopupModal("Empty Seed", NULL, ImGuiWindowFlags_NoResize))
	{

		ImGui::TextWrapped("Warning! Not setting a seed means a random one will be generated.");
		ImGui::TextWrapped("This is fine for solo play, but for Co-op playthroughs both players must use the same seed, or else the game will desync.");
	

		if (ImGui::Button("Ok"))
		{
			copySettings();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


void OptionsGUI::renderHighMultiplierWarning()
{

	ImVec2 size = ImVec2(300, 0);
	ImVec2 pos = ImVec2(ImGuiManager::getScreenSize() / 2.f) - (size / 2.f);

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(pos);

	if (ImGui::BeginPopupModal("High Multiplier Warning", NULL, ImGuiWindowFlags_NoResize))
	{

		ImGui::TextWrapped("Warning! Setting enemy multiplication to a high value (>3x) is likely to crash the game due to engine limitations.");
		if (ImGui::Button("Continue"))
		{
			OptionsState::EnemySpawnMultiplier.UpdateValueWithInput();
			changesPending_Mult = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop"))
		{
			OptionsState::EnemySpawnMultiplier.GetValueDisplay() = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
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


bool debugme = true;
void renderHyperLinkText(std::string text, std::string url)
{
	if (ImGui::Selectable(text.c_str(), true, ImGuiSelectableFlags_DontClosePopups, ImVec2(text.length() * 7, 15)))
	{
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}

}



bool clickedLastFrame = false;
void OptionsGUI::renderAboutWindow()
{
	ImVec2 size = ImVec2(300, 0);
	ImVec2 pos = ImVec2(ImGuiManager::getScreenSize() / 2.f) - (size / 2.f);

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(pos);

	if (ImGui::BeginPopupModal("AboutWindow", NULL, ImGuiWindowFlags_NoResize))
	{
		ImGui::TextWrapped("Created by Burnt with much help from Scales");
		
		renderHyperLinkText("Source code", "https://github.com/Burnt-o/CEER");
		renderHyperLinkText("Latest release", "https://github.com/Burnt-o/CEER/releases");

		ImGui::Text("TODO: add more here");

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}




	


		ImGui::EndPopup();
	}
}




void OptionsGUI::renderAddSpawnMultiplierRulePopup()
{
	if (ImGui::BeginPopup("AddSpawnMultiplierRulePopup"))
	{
		if (ImGui::Selectable("Spawn Multiplier Pre-Randomisation"))
		{
			changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
			OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierBeforeRando());
		}
		addTooltip("Add a spawn multiplier rule that occurs before any randomisation.");
		if (ImGui::Selectable("Spawn Multiplier Post-Randomisation"))
		{
			changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
			OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierAfterRando());
		}
		addTooltip("Add a spawn multiplier rule that occurs after any randomisation.");
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

	//for ( std::unique_ptr<EnemyRandomiserRule>& rule : OptionsState::currentRules) // render rules
	for (auto it = OptionsState::currentMultiplierRules.begin(); auto & rule: OptionsState::currentMultiplierRules) // render rules
	{

		ImGui::BeginChild(ImGui::GetID(rule.get()), ImVec2(0.f, ruleTypeToPixelHeight[rule.get()->getType()]), true, NULL);

		if (ImGui::Button("Delete"))
		{
			changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
			OptionsState::currentMultiplierRules.erase(it);
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();
		addTooltip("Delete this rule.");

		if (it == OptionsState::currentMultiplierRules.begin()) ImGui::BeginDisabled(); // if at start, disable move up button
		if (ImGui::Button("Move Up"))
		{
			changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
			std::swap(*it, *(it - 1));
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();
		addTooltip("Move this rule up (rules at the top take precedence if a conflict occurs).");
		if (it == OptionsState::currentMultiplierRules.begin()) ImGui::EndDisabled(); // if at start, disable move up button


		if (it == OptionsState::currentMultiplierRules.end() - 1) ImGui::BeginDisabled(); // if at end, disable move down button
		if (ImGui::Button("Move Down"))
		{
			changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
			std::swap(*it, *(it + 1));
			ImGui::EndChild();
			break;
		} ImGui::SameLine();
		addTooltip("Move this rule down (rules at the top take precedence if a conflict occurs).");
		if (it == OptionsState::currentMultiplierRules.end() - 1) ImGui::EndDisabled(); // if at end, disable move down button
		ImGui::Text(" Rule type:"); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), ruleTypeToRuleTypeName[rule.get()->getType()].c_str());
		addTooltip(ruleTypeToRuleTypeToolTip[rule.get()->getType()].c_str());


		switch (rule->getType())
		{
		case RuleType::SpawnMultiplierPreRando:
		{
			SpawnMultiplierBeforeRando* thisRule = dynamic_cast<SpawnMultiplierBeforeRando*>(rule.get());
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
						changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
						thisRule->groupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
			addTooltip("The category of enemies which will have their spawn rate modified");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("by:"); ImGui::SameLine();

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputDouble("##multiplyPercent", &thisRule->multiplier.GetValueDisplay(), 1.0, 10.0, "%.2f"))
			{
				changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
				thisRule->multiplier.UpdateValueWithInput();
			}
			addTooltip("The spawn rate multiplier. Decimal numbers are allowed.");
			ImGui::SameLine(); ImGui::Text("x");

		}
		break;

		case RuleType::SpawnMultiplierPostRando:
		{
			SpawnMultiplierAfterRando* thisRule = dynamic_cast<SpawnMultiplierAfterRando*>(rule.get());
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
						changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
						thisRule->groupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
			addTooltip("The category of enemies which will have their spawn rate modified");
			ImGui::AlignTextToFramePadding();
			ImGui::Text("by:"); ImGui::SameLine();

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputDouble("##multiplyPercent", &thisRule->multiplier.GetValueDisplay(), 1.0, 10.0, "%.2f"))
			{
				changesPending_Mult = OptionsState::EnemySpawnMultiplier.GetValue();
				thisRule->multiplier.UpdateValueWithInput();
			}
			addTooltip("The spawn rate multiplier. Decimal numbers are allowed.");
			ImGui::SameLine(); ImGui::Text("x");

		}
		break;
		default:
			break;
		}

		ImGui::EndChild();
		it++;
	}


}

void OptionsGUI::renderEnemyRandomiserRules()
{

	//for ( std::unique_ptr<EnemyRandomiserRule>& rule : OptionsState::currentRules) // render rules
	for (auto it = OptionsState::currentRandomiserRules.begin(); auto & rule: OptionsState::currentRandomiserRules) // render rules
	{

		ImGui::BeginChild(ImGui::GetID(rule.get()), ImVec2(0.f, ruleTypeToPixelHeight[rule.get()->getType()]), true, NULL);

		if (ImGui::Button("Delete"))
		{
			changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
			OptionsState::currentRandomiserRules.erase(it);
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();
		addTooltip("Delete this rule.");

		if (it == OptionsState::currentRandomiserRules.begin()) ImGui::BeginDisabled(); // if at start, disable move up button

		if (ImGui::Button("Move Up"))
		{
			changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
			std::swap(*it, *(it - 1));
			ImGui::EndChild();
			break;
		}	ImGui::SameLine();
		addTooltip("Move this rule up (rules at the top take precedence if a conflict occurs).");
		if (it == OptionsState::currentRandomiserRules.begin()) ImGui::EndDisabled(); // if at start, disable move up button


		if (it == OptionsState::currentRandomiserRules.end() - 1) ImGui::BeginDisabled(); // if at end, disable move down button
		if (ImGui::Button("Move Down"))
		{
			changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
			std::swap(*it, *(it + 1));
			ImGui::EndChild();
			break;
		} ImGui::SameLine();
		addTooltip("Move this rule down (rules at the top take precedence if a conflict occurs).");
		if (it == OptionsState::currentRandomiserRules.end() - 1) ImGui::EndDisabled(); // if at end, disable move down button
		ImGui::Text(" Rule type:"); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), ruleTypeToRuleTypeName[rule.get()->getType()].c_str());
		addTooltip(ruleTypeToRuleTypeToolTip[rule.get()->getType()].c_str());


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
				changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
				thisRule->randomisePercent.UpdateValueWithInput();
			} ImGui::SameLine();
			addTooltip("Percentage of enemies in the selected group that will be randomised.");
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
						changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
						thisRule->randomiseGroupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
			addTooltip("The category of enemies that will be randomised into something else.");

			ImGui::Text("Into:");

			if (ImGui::BeginCombo("##rollGroup", thisRule->rollPoolGroupSelection.getName().data()))
			{
				for (int n = 0; n < builtInGroups::builtInGroups.size(); n++)
				{
					const bool is_selected = &thisRule->rollPoolGroupSelection == &builtInGroups::builtInGroups.at(n);
					if (ImGui::Selectable(builtInGroups::builtInGroups[n].getName().data(), is_selected))
					{
						changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
						thisRule->rollPoolGroupSelection = builtInGroups::builtInGroups.at(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}
			addTooltip("The category of enemies that they will be randomised into.");
		}
		break;
		
		default:
			break;
		}

		ImGui::EndChild();
		it++;
	}

}



bool IsEnemyMultiplierTooHigh()
{
	SpawnMultiplierBeforeRando* thisRulePre = nullptr;
	SpawnMultiplierAfterRando* thisRulePost = nullptr;
	for (auto& rule : OptionsState::currentMultiplierRules)
	{
		switch (rule->getType())
		{
		case RuleType::SpawnMultiplierPreRando:

			thisRulePre = dynamic_cast<SpawnMultiplierBeforeRando*>(rule.get());
			assert(thisRulePre != nullptr);
			if (thisRulePre->multiplier.GetValue() > 3.f)
			{
				return true;
			}
			break;

		case RuleType::SpawnMultiplierPostRando:

			thisRulePost = dynamic_cast<SpawnMultiplierAfterRando*>(rule.get());
			assert(thisRulePost != nullptr);
			if (thisRulePost->multiplier.GetValue() > 3.f)
			{
				return true;
			}
			break;
		}
	}
	return false;
}




void OptionsGUI::renderOptionsGUI()
{
	
	// Dialogs
	renderErrorDialog();




#pragma region MainWindow
	// Main window
	ImGui::SetNextWindowSize(ImVec2(500, 500));
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	
	m_WindowOpen = ImGui::Begin(m_WindowOpen ? "CE Enemy Randomiser###CEER" : "CEER###CEER", nullptr, windowFlags); // Create window

	if (m_WindowOpen)  //only bother rendering children if it's not collapsed
	{

#pragma region Settings/About

		ImGui::Dummy((ImVec2(0, 2)));
		if (ImGui::Button("Copy Settings"))
		{
			if (OptionsState::SeedString.GetValue().empty())
			{
				ImGui::OpenPopup("Empty Seed");
			}
			else
			{
				copySettings();
			}

		} ImGui::SameLine();
		addTooltip("Copies all CEER settings to your clipboard, so you can ctrl+v it to a mate over discord to do a coop run.");
		renderEmptySeedWarning();

		if (ImGui::Button("Paste Settings"))
		{
			pasteSettings();

		} ImGui::SameLine();
		addTooltip("Paste CEER settings from your clipboard.");

		if (ImGui::Button("About"))
		{
			// modal popup
			ImGui::OpenPopup("AboutWindow");
		}
		renderAboutWindow();


		ImGui::SameLine();
		if (ImGui::Button("Shutdown"))
		{
			GlobalKill::killMe();
		}
		addTooltip("Closes CEER.");

#pragma endregion


		ImGui::Dummy((ImVec2(0, 2)));
#pragma region Seed
		ImGui::SetNextItemWidth(200);
		if (ImGui::InputText("Seed", &OptionsState::SeedString.GetValueDisplay()))
		{
			OptionsState::SeedString.UpdateValueWithInput();

			changesPending_Seed = (OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue());
		}
			addTooltip("Seed determines the outcome of randomisation. Leave blank to have a seed auto-generated for you.");

		if (changesPending_Seed && (OptionsState::EnemyRandomiser.GetValue() || OptionsState::EnemySpawnMultiplier.GetValue() || OptionsState::TextureRandomiser.GetValue() || OptionsState::SoundRandomiser.GetValue()))
		{
			ImGui::SameLine();
			if (ImGui::Button("Apply Pending Changes"))
			{
				changesPending_Seed = false;
				// deduplicate firing the enemyRandomsier/multiplier events. 
				OptionsState::EnemyRandomiser.UpdateValueWithInput(); // Since rando and mult both proc the same stuff we only need to fire one
				OptionsState::TextureRandomiser.UpdateValueWithInput();
				OptionsState::SoundRandomiser.UpdateValueWithInput();
			}
			addTooltip("Update CEERs internal state with your inputted changed settings.");
		}

#pragma endregion Seed






		renderManageCustomGroupsDialog();

#pragma region EnemyRandomiser
		ImGui::SeparatorText("");

		if (ImGui::Checkbox("Enable Enemy Randomiser", &OptionsState::EnemyRandomiser.GetValueDisplay()))
		{
			OptionsState::EnemyRandomiser.UpdateValueWithInput();
			changesPending_Rand = false;
		}
		addTooltip("Enables enemy randomisation: requires a valid rule first, such as turning \"everything\" into \"everything\".");

		if (changesPending_Rand && (OptionsState::EnemyRandomiser.GetValue()))
		{
			ImGui::SameLine();
			if (ImGui::Button("Apply Pending Changes"))
			{
				changesPending_Rand = false;
				OptionsState::EnemyRandomiser.UpdateValueWithInput();
			}
			addTooltip("Update CEERs internal state with your inputted changed settings.");
		}


		//ImGui::SetNextWindowBgAlpha(0.5f);


		ImGui::Indent();
		ImGui::Dummy((ImVec2(0, 2)));
		if (ImGui::CollapsingHeader("Enemy Randomiser Settings", ImGui::IsItemHovered()))
		{
			float rulesWindowHeight = 12.f;
			for (auto& rule : OptionsState::currentRandomiserRules)
			{
				rulesWindowHeight += ruleTypeToPixelHeight[rule.get()->getType()] + 5.f;
			}
			ImGui::BeginChild("enemyRandoSettings", ImVec2(0, rulesWindowHeight + 50.f), true, 0);

			if (ImGui::Checkbox("Include Flamethrower Flood", &OptionsState::RandomiserIncludesFlameThrowers.GetValueDisplay()))
			{
				OptionsState::RandomiserIncludesFlameThrowers.UpdateValueWithInput();
				changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
			}
			addTooltip("Adds flamethrower-wielding flood (a cut enemy) to the appropiate enemy category pools (eg \"everything\", \"flood\", \"combat forms\".");

			if (ImGui::Button("Add Randomisation Rule"))
			{
				OptionsState::currentRandomiserRules.emplace_back(new RandomiseXintoY());
				changesPending_Rand = OptionsState::EnemyRandomiser.GetValue();
			}
#if CEER_CUSTOM_GROUPS == 1
			ImGui::SameLine();
			if (ImGui::Button("Manage Custom Groups"))
			{
				ImGui::OpenPopup("ManageCustomGroupsDialog");
			}
#endif

			renderEnemyRandomiserRules();
			ImGui::EndChild();
		} 
		else
		{
			addTooltip("Click to expand Enemy Randomiser settings.");
		}
		ImGui::Unindent();
		
#pragma endregion EnemyRandomiser
#pragma region EnemySpawnMultiplier
		ImGui::SeparatorText("");

	
		if (ImGui::Checkbox("Enable Enemy Spawn Multiplier", &OptionsState::EnemySpawnMultiplier.GetValueDisplay()))
		{
			if (OptionsState::EnemySpawnMultiplier.GetValueDisplay() == true && IsEnemyMultiplierTooHigh())
			{
				ImGui::OpenPopup("High Multiplier Warning");
			}
			else
			{
				OptionsState::EnemySpawnMultiplier.UpdateValueWithInput();
				changesPending_Mult = false;
			}

		}
		addTooltip("Enables enemy spawn multiplication: requires a valid rule first, such as doubling the spawnrate of \"everything\".");

		if (changesPending_Mult && (OptionsState::EnemySpawnMultiplier.GetValue()))
		{
			ImGui::SameLine();
			if (ImGui::Button("Apply Pending Changes"))
			{
				if (IsEnemyMultiplierTooHigh())
				{
					ImGui::OpenPopup("High Multiplier Warning");
				}
				else
				{
					changesPending_Mult = false;
					OptionsState::EnemySpawnMultiplier.UpdateValueWithInput();
				}

			}
			addTooltip("Update CEERs internal state with your inputted changed settings.");
		}
		renderHighMultiplierWarning();

	

		ImGui::Indent();
		ImGui::Dummy((ImVec2(0, 2)));
		if (ImGui::CollapsingHeader("Enemy Spawn Multiplier Settings", ImGui::IsItemHovered()))
		{

			float rulesWindowHeight = 12.f;
			for (auto& rule : OptionsState::currentMultiplierRules)
			{
				rulesWindowHeight += ruleTypeToPixelHeight[rule.get()->getType()] + 5.f;
			}

			ImGui::BeginChild("enemyMultiplierSettings", ImVec2(0, rulesWindowHeight + 30.f), true, 0);

			if (ImGui::Button("Add Multiplier Rule"))
			{
				ImGui::OpenPopup("AddSpawnMultiplierRulePopup");
			}
			renderAddSpawnMultiplierRulePopup();
#if CEER_CUSTOM_GROUPS == 1
			ImGui::SameLine();
			if (ImGui::Button("Manage Custom Groups"))
			{
				ImGui::OpenPopup("ManageCustomGroupsDialog");
			}
#endif



			renderEnemySpawnMultiplierRules();
			ImGui::EndChild();


		}
		else
		{
			addTooltip("Click to expand Enemy Spawn Multiplier settings.");
		}

		ImGui::Unindent();

#pragma endregion EnemySpawnMultiplier
#pragma region TextureRandomiser
		ImGui::SeparatorText("");


		constexpr int warnSeizureIfFramesLessThan = 1999;

		if (ImGui::Checkbox("Enable Texture Randomiser", &OptionsState::TextureRandomiser.GetValueDisplay()))
		{
			// Need to add a check for if the SeizureMode has been set to a really fast setting, and warn the user 
			if (OptionsState::TextureRandomiser.GetValueDisplay() == true && OptionsState::TextureFramesBetweenSeizures.GetValue() < warnSeizureIfFramesLessThan)
			{
				ImGui::OpenPopup("Seizure Warning");
			}
			else
			{
				OptionsState::TextureRandomiser.UpdateValueWithInput();
				changesPending_Text = false;
			}
		}
		addTooltip("Enables texture randomisation. Only supports classic graphics.");

		if (changesPending_Text && (OptionsState::TextureRandomiser.GetValue() == true))
		{
			ImGui::SameLine();
			if (ImGui::Button("Apply Pending Changes"))
			{
				if (OptionsState::TextureFramesBetweenSeizures.GetValue() < warnSeizureIfFramesLessThan)
				{
					ImGui::OpenPopup("Seizure Warning");
				}
				else
				{
					OptionsState::TextureRandomiser.UpdateValueWithInput();
					changesPending_Text = false;
				}
			}
			addTooltip("Update CEERs internal state with your inputted changed settings.");
		}
		renderTextureSeizureWarning();


		ImGui::Indent();
		ImGui::Dummy((ImVec2(0, 2)));
		if (ImGui::CollapsingHeader("Texture Randomiser Settings", ImGui::IsItemHovered()))
		{
			ImGui::BeginChild("texSettings", ImVec2(0, 260), true, 0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Randomise");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.f);
			if (ImGui::InputDouble("Percent of Textures", &OptionsState::TextureRandomiserPercent.GetValueDisplay(), 0, 0, "%.0f"))
			{
				OptionsState::TextureRandomiserPercent.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("The percent of textures that should be randomised.");

			if (ImGui::Checkbox("Restrict randomisation to like categories", &OptionsState::TextureRestrictToCategory.GetValueDisplay()))
			{
				OptionsState::TextureRestrictToCategory.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("If enabled, then, for example, weapon textures will only get randomised into other weapon textures. When disabled, anything can become anything.");

			ImGui::SeparatorText("Texture Categories");
			if (ImGui::Checkbox("Characters", &OptionsState::TextureIncludeCharacter.GetValueDisplay()))
			{
				OptionsState::TextureIncludeCharacter.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("Enables randomisation of textures associated with NPC's and the player.");
			if (ImGui::Checkbox("Weapons and vehicles", &OptionsState::TextureIncludeWeapVehi.GetValueDisplay()))
			{
				OptionsState::TextureIncludeWeapVehi.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("Enables randomisation of textures associated with vehicles and weapons.");
			if (ImGui::Checkbox("Effects and particles", &OptionsState::TextureIncludeEffect.GetValueDisplay()))
			{
				OptionsState::TextureIncludeEffect.UpdateValueWithInput();
				changesPending_Text = true;
			}
			if (ImGui::Checkbox("Level geometry", &OptionsState::TextureIncludeLevel.GetValueDisplay()))
			{
				OptionsState::TextureIncludeLevel.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("Enables randomisation of textures associated with the environment / level geometry.");
			if (ImGui::Checkbox("User Interface", &OptionsState::TextureIncludeUI.GetValueDisplay()))
			{
				OptionsState::TextureIncludeUI.UpdateValueWithInput();
				changesPending_Text = true;
			}

			ImGui::SeparatorText("Seizure Mode");
			if (ImGui::Checkbox("Enable Texture Mutation", &OptionsState::TextureSeizureMode.GetValueDisplay()))
			{
				OptionsState::TextureSeizureMode.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("When enabled, textures will be constantly mutated into other textures.");
			ImGui::AlignTextToFramePadding();
			ImGui::Text("every");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputInt("frames", &OptionsState::TextureFramesBetweenSeizures.GetValueDisplay()))
			{
				OptionsState::TextureFramesBetweenSeizures.UpdateValueWithInput();
				changesPending_Text = true;
			}
			addTooltip("How many frames, on average, it takes for a texture to mutate into another texture.");



			ImGui::EndChild();
		}
		else
		{
			addTooltip("Click to expand Texture Randomiser settings.");
		}
		
		ImGui::Unindent();


#pragma endregion TextureRandomiser

#pragma region SoundRandomiser
		ImGui::SeparatorText("");


		if (ImGui::Checkbox("Enable Sound Randomiser", &OptionsState::SoundRandomiser.GetValueDisplay()))
		{
			OptionsState::SoundRandomiser.UpdateValueWithInput();
			changesPending_Sound = false;
		}
		addTooltip("Enables sound randomisation.");

		if (changesPending_Text && (OptionsState::SoundRandomiser.GetValue()))
		{
			ImGui::SameLine();
			if (ImGui::Button("Apply Pending Changes"))
			{
				changesPending_Sound = false;
				OptionsState::SoundRandomiser.UpdateValueWithInput();
			}
			addTooltip("Update CEERs internal state with your inputted changed settings.");
		}



		ImGui::Indent();
		ImGui::Dummy((ImVec2(0, 2)));
		if (ImGui::CollapsingHeader("Sound Randomiser Settings", ImGui::IsItemHovered()))
		{
			ImGui::BeginChild("sndSettings", ImVec2(0, 200), true, 0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Randomise");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.f);
			if (ImGui::InputDouble("Percent of Sounds", &OptionsState::SoundRandomiserPercent.GetValueDisplay(), 0, 0, "%.0f"))
			{
				OptionsState::SoundRandomiserPercent.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("The percent of sounds that should be randomised.");

			if (ImGui::Checkbox("Restrict randomisation to like categories", &OptionsState::SoundRestrictToCategory.GetValueDisplay()))
			{
				OptionsState::SoundRestrictToCategory.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("If enabled, then, for example, character dialogue will only get randomised into other character dialogue. When disabled, anything can become anything.");

			ImGui::SeparatorText("Sound Categories");
			if (ImGui::Checkbox("Dialog", &OptionsState::SoundIncludeDialog.GetValueDisplay()))
			{
				OptionsState::SoundIncludeDialog.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("Enables randomisation of sounds associated with character dialogue (anything that comes out of a mouth).");
			if (ImGui::Checkbox("Music", &OptionsState::SoundIncludeMusic.GetValueDisplay()))
			{
				OptionsState::SoundIncludeMusic.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("Enables randomisation of music.");
			if (ImGui::Checkbox("Animations", &OptionsState::SoundIncludeAnimations.GetValueDisplay()))
			{
				OptionsState::SoundIncludeAnimations.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("Enables randomisation of sounds associated with most animations, such as melee, footsteps, bullet/vehicle impacts.");
			if (ImGui::Checkbox("Effects", &OptionsState::SoundIncludeEffects.GetValueDisplay()))
			{
				OptionsState::SoundIncludeEffects.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("Enables randomisation of sounds associated with effects like explosions, projectiles, ambience, etc.");
			if (ImGui::Checkbox("Weapons and vehicles", &OptionsState::SoundIncludeWeapVehi.GetValueDisplay()))
			{
				OptionsState::SoundIncludeWeapVehi.UpdateValueWithInput();
				changesPending_Sound = true;
			}
			addTooltip("Enables randomisation of sounds associated with weapons and vehicles like engine sounds, firing and reload sounds.");



			ImGui::EndChild();
		}
		else
		{
			addTooltip("Click to expand Sound Randomiser settings.");
		}
		ImGui::Unindent();


#pragma endregion SoundRandomiser

	}

	ImGui::End(); // end main window

#pragma endregion MainWindow




}
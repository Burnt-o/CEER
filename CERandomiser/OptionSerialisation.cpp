#include "pch.h"
#include "OptionSerialisation.h"
#include "OptionsState.h"
#include "InitParameter.h"
#include "MessagesGUI.h"
namespace OptionSerialisation
{
	// need to get some xml code up in here


	void throwOnDuplicateName(pugi::xml_document& doc)
	{
		// recursive interface to test if all tag names with the same parent are unique 
		struct simple_walker : pugi::xml_tree_walker
		{
;
			virtual bool for_each(pugi::xml_node& node)
			{
				if (node.name() == "") return true;

				std::set<std::string> checkForDuplicateSymbols;
				for (pugi::xml_node child : node.children())
				{	
					if (child.type() == pugi::node_null) continue;
					if (checkForDuplicateSymbols.emplace(child.name()).second)
					{
						continue;
					}
					else
					{
						throw SerialisationException(std::format("Non-unique acronym \"{}\"! Burnt made an oopsie", child.name()));
					}
				}
			}
		}walker;
		doc.traverse(walker);
	}





	pugi::xml_document serialiseAllOptions(bool clipboardOnlySettings)
	{
		pugi::xml_document doc;
		// options
		auto optionArray = doc.append_child(acronymOf(Option));

		for (auto option : OptionsState::allSerialisableOptions)
		{
			if (clipboardOnlySettings && !option->isIncludedInClipboard()) continue;

			PLOG_DEBUG << "serialising: " << option->getOptionName();
			option->serialise(optionArray);
		}

		// rules
		auto randoArray = doc.append_child(acronymOf(OptionsState::currentRandomiserRules));
		auto multiArray = doc.append_child(acronymOf(OptionsState::currentMultiplierRules));

		RandomiseXintoY* randRule = nullptr;
		SpawnMultiplierBeforeRando* preMultRule = nullptr;
		SpawnMultiplierAfterRando* postMultRule = nullptr;

		throwOnDuplicateName(doc); // rules are allowed to be duplicate names since you can have multiple of the same type

		for (auto& rule : OptionsState::currentRandomiserRules)
		{
			if (rule.get()->getType() != RuleType::RandomiseXintoY)
				throw SerialisationException("Bad RuleType");

			randRule = dynamic_cast<RandomiseXintoY*>(rule.get());
			randRule->serialise(randoArray);
		}

		for (auto& rule : OptionsState::currentMultiplierRules)
		{
			switch (rule.get()->getType())
			{
			case RuleType::SpawnMultiplierPreRando:
				preMultRule = dynamic_cast<SpawnMultiplierBeforeRando*>(rule.get());
				preMultRule->serialise(multiArray);
				break;

			case RuleType::SpawnMultiplierPostRando:
				postMultRule = dynamic_cast<SpawnMultiplierAfterRando*>(rule.get());
				postMultRule->serialise(multiArray);
				break;

			default:
				throw SerialisationException("Bad RuleType");
			}
		}

		return doc;

	}




	//void deserialiseAll(std::string in)
	//{

	//	deserialiseAll(doc);
	//}


	void deserialiseAllOptions(pugi::xml_document& doc, bool clipboardOnly)
	{

		// rules must be deserialised before options, so that the rules are loaded in before any of the options actually turn on.

		// rules
		OptionsState::currentRandomiserRules.clear();
		OptionsState::currentMultiplierRules.clear();

		auto randoRulesArray = doc.child(acronymOf(OptionsState::currentRandomiserRules));
		auto multiRulesArray = doc.child(acronymOf(OptionsState::currentMultiplierRules));


		for (auto& node : randoRulesArray)
		{
			if (std::strcmp(node.name(), acronymOf(RandomiseXintoY)) == 0)
			{
				auto& rule = OptionsState::currentRandomiserRules.emplace_back(new RandomiseXintoY());
				try
				{
					rule.get()->deserialise(node);
				}
				catch (SerialisationException& ex)
				{
					RuntimeExceptionHandler::handlePopup(ex);
					OptionsState::currentRandomiserRules.pop_back();
				}
			}
		}

		for (auto& node : multiRulesArray)
		{
			if (std::strcmp(node.name(), acronymOf(SpawnMultiplierBeforeRando)) == 0)
			{
				auto& rule = OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierBeforeRando());
				try
				{
					rule.get()->deserialise(node);
				}
				catch (SerialisationException& ex)
				{
					RuntimeExceptionHandler::handlePopup(ex);
					OptionsState::currentMultiplierRules.pop_back();
				}
			}
			else if (std::strcmp(node.name(), acronymOf(SpawnMultiplierAfterRando)) == 0)
			{
				auto& rule = OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierAfterRando());
				try
				{
					rule.get()->deserialise(node);
				}
				catch (SerialisationException& ex)
				{
					RuntimeExceptionHandler::handlePopup(ex);
					OptionsState::currentMultiplierRules.pop_back();
				}
			}
		}

		// options
		auto optionArray = doc.child(acronymOf(Option));
		if (optionArray.type() == pugi::node_null) throw SerialisationException("Could not find OptionArray node");
		for (auto option : OptionsState::allSerialisableOptions)
		{
			if (clipboardOnly && !option->isIncludedInClipboard()) continue;
			auto optionXML = optionArray.child(getShortName(option->getOptionName()).c_str());
			if (optionXML.type() == pugi::node_null) throw SerialisationException(std::format("Could not find Option node {}", option->getOptionName()));
			option->deserialise(optionXML);
		}

	}




	void serialiseToFile()
	{
		std::string filePath = g_ourInitParameters->injectorPath;
		filePath += "CEERConfig.xml";

		try
		{
			auto xml = serialiseAllOptions();
			if (!xml.save_file(filePath.c_str()))
			{
				PLOG_ERROR << "Error saving config to " << filePath;
			}
		}
		catch (SerialisationException& ex)
		{
			RuntimeExceptionHandler::handleMessage(ex);
		}
	}
	void deserialiseFromFile()
	{
		std::string filePath = g_ourInitParameters->injectorPath;
		filePath += "CEERConfig.xml";
		
		pugi::xml_document doc;
			
		pugi::xml_parse_result result = doc.load_file(filePath.c_str());

		if (result)
		{
			try
			{
				deserialiseAllOptions(doc, false);
			}
			catch (SerialisationException& ex)
			{
				RuntimeExceptionHandler::handleMessage(ex);
			}
		}
		else
		{
			std::string resultString = result.description();
			if (resultString == "File was not found")
			{
				MessagesGUI::addMessage("Config file not found, loading default settings.");
			}
			else
			{
				std::string err = std::format("Error parsing file at {}\nError description: {}\nError offset: {}", filePath, resultString, result.offset);
				SerialisationException ex(err);
				RuntimeExceptionHandler::handleMessage(ex);
			}


		}
	}





}
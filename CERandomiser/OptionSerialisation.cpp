#include "pch.h"
#include "OptionSerialisation.h"
#include "OptionsState.h"
#include "InitParameter.h"
#include "MessagesGUI.h"
namespace OptionSerialisation
{
	// need to get some xml code up in here




	pugi::xml_document serialiseAll()
	{
		pugi::xml_document doc;
		// options
		auto optionArray = doc.append_child(nameof(Option));
		for (auto option : OptionsState::allSerialisableOptions)
		{
			PLOG_DEBUG << "serialising: " << option->getOptionName();
			option->serialise(optionArray);
		}

		// rules
		auto randoArray = doc.append_child(nameof(OptionsState::currentRandomiserRules));
		auto multiArray = doc.append_child(nameof(OptionsState::currentMultiplierRules));

		RandomiseXintoY* randRule = nullptr;
		SpawnMultiplierPreRando* preMultRule = nullptr;
		SpawnMultiplierPostRando* postMultRule = nullptr;

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
				preMultRule = dynamic_cast<SpawnMultiplierPreRando*>(rule.get());
				preMultRule->serialise(multiArray);
				break;

			case RuleType::SpawnMultiplierPostRando:
				postMultRule = dynamic_cast<SpawnMultiplierPostRando*>(rule.get());
				postMultRule->serialise(multiArray);
				break;

			default:
				throw SerialisationException("Bad RuleType");
			}
		}

		return doc;

	}

	void deserialiseAll(std::string in)
	{
		PLOG_DEBUG << "deserialising\n" << in;
		// try parsing string to doc
		pugi::xml_document doc;
		auto result = doc.load_string(in.c_str());

		if (!result)
		{
			throw SerialisationException(std::format("Failed to parse clipboard string as xml doc\nError: {}\nOffset: {}: Location: {}",
				result.description(), result.offset, (in.c_str() + result.offset)
			));
		}
		deserialiseAll(doc);
	}

	void deserialiseAll(pugi::xml_document& doc)
	{

		// options
		auto optionArray = doc.child("Option");
		if (optionArray.type() == pugi::node_null) throw SerialisationException("Could not find OptionArray node");
		for (auto option : OptionsState::allSerialisableOptions)
		{
			auto optionXML = optionArray.child(option->getOptionName().c_str());
			if (optionXML.type() == pugi::node_null) throw SerialisationException(std::format("Could not find Option node {}", option->getOptionName()));
			option->deserialise(optionXML);
		}

		// rules
		OptionsState::currentRandomiserRules.clear();
		OptionsState::currentMultiplierRules.clear();

		auto randoRulesArray = doc.child(nameof(OptionsState::currentRandomiserRules));
		auto multiRulesArray = doc.child(nameof(OptionsState::currentMultiplierRules));


		for (auto node : randoRulesArray.children(nameof(RandomiseXintoY)))
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
		
		for (auto node : multiRulesArray.children(nameof(SpawnMultiplierPreRando)))
		{
			auto& rule = OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierPreRando());
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


		for (auto node : multiRulesArray.children(nameof(SpawnMultiplierPostRando)))
		{

			auto& rule = OptionsState::currentMultiplierRules.emplace_back(new SpawnMultiplierPostRando());
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

		// TODO:: check if vectors need to be reversed

	}



	void serialiseToFile()
	{
		std::string filePath = g_ourInitParameters->injectorPath;
		filePath += "CEERConfig.xml";

		try
		{
			auto xml = serialiseAll();
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
				deserialiseAll(doc);
			}
			catch (SerialisationException& ex)
			{
				RuntimeExceptionHandler::handleMessage(ex);
			}
		}
		else
		{
			if (result.description() == "File was not found")
			{
				MessagesGUI::addMessage("Config file not found, loading default settings.");
			}
			else
			{
				std::string err = std::format("Error parsing file at {}\nError description: {}\nError offset: {}", filePath, result.description(), result.offset);
				SerialisationException ex(err);
				RuntimeExceptionHandler::handleMessage(ex);
			}


		}
	}


}
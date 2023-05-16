#include "pch.h"
#include "EnemyRule.h"


void RandomiseXintoY::serialise(pugi::xml_node parent)
{
	auto ruleXML = parent.append_child(nameof(RandomiseXintoY));
	randomiseGroupSelection.serialise(ruleXML, nameof(randomiseGroupSelection));
	rollGroupSelection.serialise(ruleXML, nameof(rollGroupSelection));
	randomisePercent.serialise(ruleXML);

}
void RandomiseXintoY::deserialise(pugi::xml_node in)
{
	randomiseGroupSelection.deserialise(in.child(nameof(randomiseGroupSelection)));
	rollGroupSelection.deserialise(in.child(nameof(rollGroupSelection)));
	randomisePercent.deserialise(in.child(nameof(randomisePercent)));
}

void SpawnMultiplierPreRando::serialise(pugi::xml_node parent)
{
	auto ruleXML = parent.append_child(nameof(SpawnMultiplierPreRando));
	groupSelection.serialise(ruleXML, nameof(groupSelection));
	multiplier.serialise(ruleXML);
	
}
void SpawnMultiplierPreRando::deserialise(pugi::xml_node in)
{
	groupSelection.deserialise(in.child(nameof(groupSelection)));
	multiplier.deserialise(in.child(nameof(multiplier)));
}

void SpawnMultiplierPostRando::serialise(pugi::xml_node parent)
{
	auto ruleXML = parent.append_child(nameof(SpawnMultiplierPostRando));
	groupSelection.serialise(ruleXML, nameof(groupSelection));
	multiplier.serialise(ruleXML);
}
void SpawnMultiplierPostRando::deserialise(pugi::xml_node in)
{
	groupSelection.deserialise(in.child(nameof(groupSelection)));
	multiplier.deserialise(in.child(nameof(multiplier)));
}
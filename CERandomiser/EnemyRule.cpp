#include "pch.h"
#include "EnemyRule.h"


void RandomiseXintoY::serialise(pugi::xml_node parent)
{
	auto ruleXML = parent.append_child(acronymOf(RandomiseXintoY));
	randomiseGroupSelection.serialise(ruleXML, acronymOf(randomiseGroupSelection));
	rollPoolGroupSelection.serialise(ruleXML, acronymOf(rollPoolGroupSelection));
	randomisePercent.serialise(ruleXML);

}
void RandomiseXintoY::deserialise(pugi::xml_node in)
{
	randomiseGroupSelection.deserialise(in.child(acronymOf(randomiseGroupSelection)));
	rollPoolGroupSelection.deserialise(in.child(acronymOf(rollPoolGroupSelection)));
	randomisePercent.deserialise(in.child(acronymOf(randomisePercent)));
}

void SpawnMultiplierBeforeRando::serialise(pugi::xml_node parent)
{
	auto ruleXML = parent.append_child(acronymOf(SpawnMultiplierBeforeRando));
	groupSelection.serialise(ruleXML, acronymOf(groupSelection));
	multiplier.serialise(ruleXML);
	
}
void SpawnMultiplierBeforeRando::deserialise(pugi::xml_node in)
{
	groupSelection.deserialise(in.child(acronymOf(groupSelection)));
	multiplier.deserialise(in.child(acronymOf(multiplier)));
}

void SpawnMultiplierAfterRando::serialise(pugi::xml_node parent)
{
	auto ruleXML = parent.append_child(acronymOf(SpawnMultiplierAfterRando));
	groupSelection.serialise(ruleXML, acronymOf(groupSelection));
	multiplier.serialise(ruleXML);
}
void SpawnMultiplierAfterRando::deserialise(pugi::xml_node in)
{
	groupSelection.deserialise(in.child(acronymOf(groupSelection)));
	multiplier.deserialise(in.child(acronymOf(multiplier)));
}
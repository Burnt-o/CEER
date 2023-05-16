#pragma once
#include "UnitInfo.h"
#include <pugixml.hpp>
class EnemyGroup
{
private:
	
	std::function<bool(UnitInfo& checkEnemy)> mMatchFunction;
	std::string mName;
	std::string mTooltip;
public:

	bool isMatch(UnitInfo& checkEnemy) { return mMatchFunction(checkEnemy); }

	EnemyGroup(std::string name, std::string tooltip, std::function<bool(UnitInfo& checkEnemy)> matchFunction)
		: mName(name), mTooltip(tooltip), mMatchFunction(matchFunction) {}

	EnemyGroup(char* name, char* tooltip, std::function<bool(UnitInfo& checkEnemy)> matchFunction)
		: mName(name), mTooltip(tooltip), mMatchFunction(matchFunction) {}

	 std::string_view getName() const { return mName; }
	 std::string_view getTooltip() const { return mTooltip; }

	 void serialise(pugi::xml_node parent, std::string name);
	 void deserialise(pugi::xml_node in);
};



namespace builtInGroups // built in group declarations - defined in cpp
{
	const extern std::vector<EnemyGroup> builtInGroups; // contains all the below

	const extern EnemyGroup GeneralEverything;
}



extern std::vector<EnemyGroup> customGroups; // starts empty
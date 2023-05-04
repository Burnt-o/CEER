#pragma once
#include "UnitInfo.h"

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
};



namespace builtInGroups // built in group declarations - defined in cpp
{
	const extern std::vector<EnemyGroup> builtInGroups; // contains all the below

	const extern EnemyGroup GeneralEverything;
}



extern std::vector<EnemyGroup> customGroups; // starts empty
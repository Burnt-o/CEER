#pragma once
class EnemyRandomiserGroup
{
private:
	
	std::function<bool(std::string_view checkEnemy)> mMatchFunction;
	std::string mName;
	std::string mTooltip;
public:

	bool isMatch(std::string_view checkEnemy) { return mMatchFunction(checkEnemy); }

	EnemyRandomiserGroup(std::string name, std::string tooltip, std::function<bool(std::string_view checkEnemy)> matchFunction)
		: mName(name), mTooltip(tooltip), mMatchFunction(matchFunction) {}

	EnemyRandomiserGroup(char* name, char* tooltip, std::function<bool(std::string_view checkEnemy)> matchFunction)
		: mName(name), mTooltip(tooltip), mMatchFunction(matchFunction) {}

	 std::string_view getName() const { return mName; }
	 std::string_view getTooltip() const { return mTooltip; }
};



namespace builtInGroups // built in group declarations - defined in cpp
{
	const extern std::vector<EnemyRandomiserGroup> builtInGroups; // contains all the below

	const extern EnemyRandomiserGroup everything;
	const extern EnemyRandomiserGroup hunters;
}



extern std::vector<EnemyRandomiserGroup> customGroups; // starts empty
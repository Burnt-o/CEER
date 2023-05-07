#pragma once

#include "EnemyGroup.h"
#include "Option.h"
enum class RuleType 
{
	RandomiseXintoY,
	SpawnMultiplierPreRando,
	SpawnMultiplierPostRando
};

class EnemyRule
{
protected:
	RuleType thisType;
public:
	RuleType getType() { return thisType; }
	virtual ~EnemyRule() {}
};



class RandomiseXintoY : public EnemyRule
{
public:
	EnemyGroup randomiseGroupSelection = builtInGroups::GeneralEverything;
	EnemyGroup rollGroupSelection = builtInGroups::GeneralEverything;


	Option<double> randomisePercent{
		100.0,
		[](double newValue)
		{
			return newValue >= 0.0 && newValue <= 100.0; // must be positive
		}
	};

	RandomiseXintoY()
	{
		thisType = RuleType::RandomiseXintoY;
	}
};

class SpawnMultiplierPreRando : public EnemyRule
{
public:
	EnemyGroup groupSelection = builtInGroups::GeneralEverything;

	Option<double> multiplier{
		1.0,
		[](double newValue)
		{
			return newValue >= 0.0; // must be positive
		}
	};

	SpawnMultiplierPreRando()
	{
		thisType = RuleType::SpawnMultiplierPreRando;
	}
};

class SpawnMultiplierPostRando : public EnemyRule
{
public:
	EnemyGroup groupSelection = builtInGroups::GeneralEverything;

	Option<double> multiplier{
		1.0,
		[](double newValue)
		{
			return newValue >= 0.0; // must be positive
		}
	};

	SpawnMultiplierPostRando()
	{
		thisType = RuleType::SpawnMultiplierPostRando;
	}
};
#pragma once

#include "EnemyGroup.h"
#include "Option.h"
#include <pugixml.hpp>
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
	virtual void serialise(pugi::xml_node parent) = 0;
	virtual void deserialise(pugi::xml_node in) = 0;
};



class RandomiseXintoY : public EnemyRule
{
public:
	EnemyGroup randomiseGroupSelection = builtInGroups::GeneralEverythingButPopcorn;
	EnemyGroup rollPoolGroupSelection = builtInGroups::GeneralEverythingButRocketFlood;


	Option<double> randomisePercent{
		100.0,
		[](double newValue)
		{
			return newValue >= 0.0 && newValue <= 100.0; // must be positive
		},
		nameof(randomisePercent)
	};

	RandomiseXintoY()
	{
		thisType = RuleType::RandomiseXintoY;
	}

	void serialise(pugi::xml_node parent) override;
	void deserialise(pugi::xml_node in) override;

};

class SpawnMultiplierBeforeRando : public EnemyRule
{
public:
	EnemyGroup groupSelection = builtInGroups::GeneralEverything;

	Option<double> multiplier{
		2.0, // defaults to 2x multiplier
		[](double newValue)
		{
			return newValue >= 0.0; // must be positive
		},
		nameof(multiplier)
	};

	SpawnMultiplierBeforeRando()
	{
		thisType = RuleType::SpawnMultiplierPreRando;
	}

	void serialise(pugi::xml_node parent) override;
	void deserialise(pugi::xml_node in) override;
};

class SpawnMultiplierAfterRando : public EnemyRule
{
public:
	EnemyGroup groupSelection = builtInGroups::GeneralEverything;

	Option<double> multiplier{
		2.0, // defaults to 2x multiplie
		[](double newValue)
		{
			return newValue >= 0.0; // must be positive
		},
		nameof(multiplier)
	};

	SpawnMultiplierAfterRando()
	{
		thisType = RuleType::SpawnMultiplierPostRando;
	}

	void serialise(pugi::xml_node parent) override;
	void deserialise(pugi::xml_node in) override;
};
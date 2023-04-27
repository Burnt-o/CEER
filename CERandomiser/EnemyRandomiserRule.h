#pragma once

#include "EnemyRandomiserGroup.h"
#include "Option.h"
enum class RuleType 
{
	RandomiseXintoY
};

class EnemyRandomiserRule
{
protected:
	RuleType thisType;
public:
	RuleType getType() { return thisType; }
	virtual ~EnemyRandomiserRule() {}
};



class RandomiseXintoY : public EnemyRandomiserRule
{
public:
	EnemyRandomiserGroup randomiseGroupSelection = builtInGroups::GeneralEverything; 
	EnemyRandomiserGroup rollGroupSelection = builtInGroups::GeneralEverything; 

	//Option<std::string> randomisePercent{
	//	"100",
	//	[](std::string newValue)
	//	{
	//		// Test if new value converts to positive float
	//		float num_float;
	//		try
	//		{
	//			num_float = std::stof(newValue);
	//			return num_float >= 0.f;
	//		}
	//		catch (std::invalid_argument& ex)
	//		{
	//			return false;
	//		}
	//		catch (std::out_of_range& ex)
	//		{
	//			return false;
	//		}
	//	}
	//};

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


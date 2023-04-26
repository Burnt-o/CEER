#pragma once

#include "EnemyRandomiserGroup.h"

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
	int randomiseGroupSelection = 0; // index 0 is "everything"
	int rollGroupSelection = 0; // index 0 is "everything"

	RandomiseXintoY()
	{
		thisType = RuleType::RandomiseXintoY;
	}
};


#pragma once

#include "Datum.h"

enum class faction {
	Undefined = -1,
	DefaultByUnit = 0,
	Player = 1,
	Human = 2,
	Covenant = 3,
	Flood = 4,
	Sentinel = 5,
	Unused6 = 6,
	Unused7 = 7,
	Unused8 = 8,
	Unused9 = 9
};


const std::map<faction, std::string> factionToString = {
{faction::Undefined, "Undefined"},
{faction::DefaultByUnit, "DefaultByUnit"},
{faction::Player, "Player"},
{faction::Human, "Human"},
{faction::Covenant, "Covenant"},
{faction::Flood, "Flood"},
{faction::Sentinel, "Sentinel"},
{faction::Unused6, "Unused6"},
{faction::Unused7, "Unused7"},
{faction::Unused8, "Unused8"},
{faction::Unused9, "Unused9"},
};


// Turns "characters\hunter\hunter" into "hunter"
std::string getShortNameFromFull(std::string fullName);

class UnitInfo {
private:
	std::string mFullName;
	std::string mShortName;
public:
	explicit UnitInfo(std::string fullName)
		: mFullName(fullName),
		mShortName(getShortNameFromFull(fullName))
	{
	}

	bool isSentinel = false; // Used to know whether to apply position fix (ie move the sentinel up from the ground)

	double probabilityOfRandomize = 0.0;
	double spawnMultiplierPreRando = 1.0;
	double spawnMultiplierPostRando = 1.0;

	int thingIndex = 0;
	faction defaultTeam = faction::Undefined;

	std::discrete_distribution<int> rollDistribution;

	std::string_view getFullName() const { return mFullName; }
	std::string_view getShortName() const { return mShortName; }

	bool isValidUnit = true;
};
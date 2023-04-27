#pragma once



enum class faction {
	Undefined = -1,
	None = 0,
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



// Turns "characters\hunter\hunter" into "hunter"
std::string getShortNameFromFull(std::string fullName);

class UnitInfo {
private:
	std::string mFullName;
	std::string mShortName;
public:
	explicit UnitInfo(std::string fullName)
		: mFullName(fullName), mShortName(getShortNameFromFull(fullName))
	{
	}

	double probabilityOfRandomize = 0.0;

	int thingIndex = 0;
	faction defaultTeam = faction::Undefined;

	std::discrete_distribution<int> rollDistribution;

	std::string_view getFullName() { return mFullName; }
	std::string_view getShortName() { return mShortName; }

	bool isValidUnit = true;
};
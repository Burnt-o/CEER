#pragma once


class TagInfoBase
{

protected:
	// Turns "characters\hunter\hunter" into "hunter"
	std::string getShortNameFromFull(std::string fullName);
	std::string mFullName;
	std::string mShortName;

	explicit TagInfoBase(std::string fullName)
		: mFullName(fullName),
		mShortName(getShortNameFromFull(fullName))
	{
	}

public:
	std::string_view getFullName() const { return mFullName; }
	std::string_view getShortName() const { return mShortName; }

};


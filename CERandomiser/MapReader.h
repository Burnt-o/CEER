#pragma once
#include "HaloEnums.h"
#include "UnitInfo.h"
struct MCCString {
	union
	{
		char shortString[0x10];
		char* longString;
	};

	size_t stringLength;
	size_t bufferLength;


	std::string copy() const
	{
		if (stringLength < 0x10)
		{
			return std::string(&shortString[0], stringLength);
		}
		else
		{
			if (IsBadReadPtr(longString, stringLength))
			{
				PLOG_ERROR << "Bad string read";
				return "Error";
			}
			return std::string(longString, stringLength);
		}
	}
};
static_assert(sizeof(MCCString) == 0x20);

struct datum {
	uint16_t index;
	uint16_t salt;
};
static_assert(sizeof(datum) == 0x4);

struct tagReference {
	uint32_t tagGroupMagic;
	uint32_t nameOffset;
	uint32_t nameLength; // seems to be unused in h1
	datum tagDatum;
};

struct actorTagReference : tagReference {};

struct bipedTagReference : tagReference {
	char unknown[0x20];
};


struct actorPaletteWrapper
{
	int tagCount;
	actorTagReference* firstTag;
};

struct bipedPaletteWrapper
{
	int tagCount;
	bipedTagReference* firstTag;
};





class MapReader
{
	class MapReaderImpl;
	std::unique_ptr<MapReaderImpl> impl;

public:
	explicit MapReader(eventpp::CallbackList<void(HaloLevel)>& levelLoadEvent);
	~MapReader();

	actorPaletteWrapper getActorPalette();
	bipedPaletteWrapper getBipedPalette();

	std::string getTagName(tagReference* tag);

	bipedTagReference* getActorsBiped(actorTagReference* tag);

	faction getBipedFaction(bipedTagReference* tag);


	static uint32_t stringMagic(std::string str);
	static uint32_t reverseStringMagic(std::string str);

	std::string getObjectName(int nameIndex);

};


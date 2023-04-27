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

struct datum
{
	uint16_t index;
	uint16_t salt;
	bool operator<(const datum& rhs) const;
	bool operator==(const datum& rhs) const;
	constexpr datum(uint16_t i, uint16_t s) : index(i), salt(s) {}
};

constexpr datum nullDatum( 0xFFFF,0xFFFF );


static_assert(sizeof(datum) == 0x4);

struct tagReference {
	uint32_t tagGroupMagic;
	uint32_t nameOffset;
	uint32_t nameLength; // seems to be unused in h1
	datum tagDatum;


};
static_assert(sizeof(tagReference) == 0x10);

struct tagElement {
	uint32_t tagGroupMagic;
	uint32_t parentGroupMagic;
	uint32_t grandparentGroupMagic;
	datum tagDatum;
	uint32_t nameOffset;
	uint32_t offset;
	uint32_t isInDataFile;
	uint32_t pad;
};
static_assert(sizeof(tagElement) == 0x20);


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

	std::string getTagName(const datum& tagDatum);

	//datum getActorsBiped(const datum& actorDatum);

	faction getBipedFaction(const datum& bipedDatum);

	faction getActorsFaction(const datum& actorDatum);

	std::string getObjectName(int nameIndex);

	std::span<tagElement> getTagTable();

	static uint32_t stringToMagic (std::string str);
	static std::string magicToString(uint32_t magic);
	tagElement* getTagElement(const datum& tagDatum);

};


#pragma once
#include "HaloEnums.h"
#include "UnitInfo.h"
#include "Datum.h"

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

struct tagBlock {
	uint32_t entryCount;
	uint32_t pointer;
	uint32_t blockDefinition;
};
static_assert(sizeof(tagBlock) == 0xC);




class MapReader
{
	class MapReaderImpl;
	std::unique_ptr<MapReaderImpl> impl;

public:
	explicit MapReader();
	~MapReader();

	tagBlock* getActorPalette();
	//tagBlock* getBipedPalette();

	std::string getTagName(const datum& tagDatum);

	//datum getActorsBiped(const datum& actorDatum);

	faction getBipedFaction(const datum& bipedDatum);

	faction getActorsFaction(const datum& actorDatum);
	void cacheTagData(HaloLevel newLevel);

	std::string getObjectName(int nameIndex);

	std::span<tagElement> getTagTable();

	static constexpr uint32_t stringToMagic (std::string_view str)
	{
		if (str.length() != 4) throw CEERRuntimeException(std::format("stringMagic bad string length: {}", str.length()));
		uint32_t out = str.at(0) << 24;
		out += str.at(1) << 16;
		out += str.at(2) << 8;
		out += str.at(3) << 0;
		return out;
	}
	static std::string magicToString(uint32_t magic);
	tagElement* getTagElement(const datum& tagDatum);
	datum getEncounterSquadDatum(int encounterIndex, int squadIndex);
	uint16_t getEncounterSquadSpawnCount(int encounterIndex, int squadIndex);

	uintptr_t getTagAddress(const datum& tagDatum);
	uintptr_t getTagAddress(uint32_t tagOffset);

};


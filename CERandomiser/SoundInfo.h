#pragma once
#include "Datum.h"
#include "TagInfoBase.h"

typedef uint32_t MemOffset;

enum class SoundCategory {
	Invalid = -1, // For everything that won't be randomised
	Dialog = 0,
	Music = 1,
	Animations = 2,
	Effects = 3,
	WeapVehi = 4,

};

static const std::map<SoundCategory, std::string> soundCategoryToString
{
	{SoundCategory::Invalid, "Invalid"},
	{SoundCategory::Dialog, "Dialog"},
	{SoundCategory::Music, "Music"},
	{SoundCategory::Animations, "Animations"},
	{SoundCategory::Effects, "Ambience"},
	{SoundCategory::WeapVehi, "WeapVehi"},
};



class SoundInfo : public TagInfoBase
{
private:

public:
	explicit SoundInfo(std::string fullName)
		: TagInfoBase(fullName)
	{
	}

	SoundCategory category = SoundCategory::Invalid;
	datum dat = nullDatum;
	MemOffset memoryOffset = 0;
	MemOffset nameMemoryOffset = 0;
};


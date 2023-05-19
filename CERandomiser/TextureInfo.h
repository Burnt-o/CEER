#pragma once
#include "Datum.h"
#include "TagInfoBase.h"

typedef uint32_t MemOffset;

enum class TextureCategory {
	Invalid = -1, // For everything that won't be randomised
	Character = 0,
	WeapVehi = 1,
	Effect = 2,
	Level = 3,
	UI = 4,

};

static const std::map<TextureCategory, std::string> textureCategoryToString
{
	{TextureCategory::Invalid, "Invalid"},
	{TextureCategory::Character, "Character"},
	{TextureCategory::WeapVehi, "WeapVehi"},
	{TextureCategory::Effect, "Effect"},
	{TextureCategory::Level, "Level"},
	{TextureCategory::UI, "UI"},
};


enum class BitmapGeometry { // "Type" in assembly
	Textures2D = 0,
	Textures3D = 1,
	CupeMaps = 2,
	Sprites = 3,
	Interface = 4,
};

enum class BitmapFormat { // "Format" in assembly
	CompressedWithColorKeyTransparency = 0,
	CompressedWithExplicitAlpha = 1,
	CompressedWithInterpolatedAlpha = 2,
	BitColor16 = 3,
	BitColor32 = 4,
	Monochrome = 5
};


class TextureInfo : public TagInfoBase
{
private:

public:
	explicit TextureInfo(std::string fullName)
		: TagInfoBase(fullName)
	{
	}

	TextureCategory category = TextureCategory::Invalid;
	datum dat = nullDatum;
	MemOffset memoryOffset = 0;


	// Below are for helping debug and prevent crashes
	// that might be caused by a texture being rando'd to one with differing geometry/format etc
	BitmapGeometry bitmapGeometry;
	BitmapFormat bitmapFormat;

	int SpriteCount = 0; // only used when BitmapGeometry == Sprites
};


#pragma once





enum class HaloLevel {
	A10 = 0,
	A30 = 1,
	A50 = 2,
	B30 = 3,
	B40 = 4,
	C10 = 5,
	C20 = 6,
	C40 = 7,
	D20 = 8,
	D40 = 9,
	UNK = -1,
};

// used for debugging / user messages
const std::map<HaloLevel, std::string> levelToString
{
	{HaloLevel::A10, "Pillar of Autumn" },
	{HaloLevel::A30, "Halo" },
	{HaloLevel::A50, "Truth and Reconcilliation" },
	{HaloLevel::B30, "The Silent Cartographer" },
	{HaloLevel::B40, "Assault on the Control Room" },
	{HaloLevel::C10, "343 Guilty Spark" },
	{HaloLevel::C20, "The Library" },
	{HaloLevel::C40, "Two Betrayals" },
	{HaloLevel::D20, "Keyes" },
	{HaloLevel::D40, "The Maw" },
	{HaloLevel::UNK, "Unknown Level" },
};

const std::map<std::string, HaloLevel> codeStringToLevel
{
	{ "a10", HaloLevel::A10 },
	{ "a30", HaloLevel::A30 },
	{ "a50", HaloLevel::A50 },
	{ "b30", HaloLevel::B30},
	{ "b40", HaloLevel::B40},
	{ "c10", HaloLevel::C10},
	{ "c20", HaloLevel::C20},
	{ "c40", HaloLevel::C40},
	{ "d20", HaloLevel::D20},
	{ "d40", HaloLevel::D40},
};
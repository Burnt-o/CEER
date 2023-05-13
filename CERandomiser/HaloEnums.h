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
	D40 = 0,
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
};
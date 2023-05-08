#pragma once

struct datum
{
	uint16_t index;
	uint16_t salt;

	//bool operator<(const datum& rhs) const;
	//bool operator==(const datum& rhs) const;
	auto operator<=>(const datum&) const = default;
	constexpr datum(uint16_t i, uint16_t s) : index(i), salt(s) {}

	constexpr datum() : index(0xFFFF), salt(0xFFFF) {}
	operator uint32_t() const { return (((uint32_t)salt << 16) + index); } // swap endianness

	friend std::ostream& operator<<(std::ostream& os, const datum& dt)
	{
		os << std::hex << "0x" << (uint32_t)dt;
		return os;
	}
};

constexpr datum nullDatum{};
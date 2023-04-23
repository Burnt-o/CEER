#include "pch.h"
#include "SetSeed.h"


// Unlinearizes and expands input ints.
// eg 0x35000 becomes 0x12bfa2a6c88af732
//    0x35001 becomes 0x5154041831987c3b etc
uint64_t SetSeed64::twist64(uint64_t in)
{
	// Adapted from https://github.com/alessandrocuda/randq/blob/master/src/randq.c
	// Ours probably doesn't meet the by-the-book good rng rules but hey,
	// this isn't a lotto, this is an enemy randomiser
	uint64_t v = 4101842887655102017LL;
	uint64_t w = 1;
	uint64_t u = in ^ v;

	u = u * 2862933555777941757LL + 7046029254386353087LL;
	uint64_t x = u ^ (u << 21);
	x ^= x >> 35;
	x ^= x << 4;
	return (x + v) ^ w;
}

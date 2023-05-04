#pragma once

#ifdef WIN32
#undef min
#undef max
#endif




// Meets the <random> definition of a Generator/_Engine by implementing operator(), min, and max. 
// Used to get "random" numbers from things like std::discrete_distribution, but unlike a normal
// generator that gens a different number each time, this one produces consistent results wrt the input int
class SetSeed64
{
private:
	uint64_t mSeed;
	static uint64_t twist64(uint64_t in);
public:
	explicit SetSeed64(uint64_t input) : mSeed(twist64(input)) {} // twist the input int to unlinearize it

	void operator()(uint64_t newInput) { mSeed = twist64(newInput); }

	uint64_t operator()() {
		return mSeed;
	}
	static constexpr uint64_t min() { return 0; }
	static constexpr uint64_t max() { return UINT64_MAX; }
};

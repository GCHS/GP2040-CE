#ifndef _WAVE_H_
#define _WAVE_H_

#include "Color.hpp"

struct __attribute__((__packed__)) Wave {
	Color c;
	float spawnTimeMillis;
	int spawnButtonIndex = 0; //center coords
	constexpr Wave(const int spawnButtonIndex, const Color& c, const float spawnTimeMillis) :
		spawnButtonIndex(spawnButtonIndex), c(c), spawnTimeMillis(spawnTimeMillis) {}
	constexpr Wave() : spawnButtonIndex(), c(), spawnTimeMillis() {}
};
#endif
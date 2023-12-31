#ifndef _BAKED_BUTTON_DISTANCES_H_
#define _BAKED_BUTTON_DISTANCES_H_

#include <array>
#include <unordered_map>
#include "stdint.h"

struct Point {
	float x, y;
	constexpr Point(const float& x, const float& y): x(x), y(y) {}
	constexpr Point() : x(0), y(0) {}
};

	//     2 3 4  5  6  7 8
	// 1  | | | |  |P2|P3|P4
	//1.5 |L|D| |P1|  |  |
	// 2  | | |R|  |K2|K3|K4
	//2.5 | | | |K1|  |  |
	//3.5 | | | |Up|  |  |
static constexpr int BUTTON_COUNT = 12;
static constexpr std::array<Point, BUTTON_COUNT> BUTTON_COORDS{
	Point{2.0, 1.5}, //LEFT
	Point{3.0, 1.5}, //DOWN
	Point{4.0, 2.0}, //RIGHT
	Point{5.0, 1.5}, //P1
	Point{6.0, 1.0}, //P2
	Point{7.0, 1.0}, //P3
	Point{8.0, 1.0}, //P4
	Point{8.0, 2.0}, //K4
	Point{7.0, 2.0}, //K3
	Point{6.0, 2.0}, //K2
	Point{5.0, 2.5}, //K1
	Point{5.0, 3.5}, //UP
};


 class ButtonDistanceTable{ //only stores the upper triangle of the distances matrix
	static constexpr int size = (BUTTON_COUNT * BUTTON_COUNT - BUTTON_COUNT) / 2;
	static const std::array<float, size> distances;
	static inline int getLinearIdx(const int i, const int j);
 public:
	float operator()(const int i, const int j) const;
};
#endif


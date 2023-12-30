#include "BakedButtonDistances.hpp"

#include <cmath>

const std::array<float, ButtonDistanceTable::size> ButtonDistanceTable::distances = []() {
  std::array<float, size> ret;
  for(auto i = 0; i < BUTTON_COUNT; ++i) {
    for(auto j = i + 1; j < BUTTON_COUNT; ++j) {
      float dx = BUTTON_COORDS[j].x - BUTTON_COORDS[i].x;
      float dy = BUTTON_COORDS[j].y - BUTTON_COORDS[i].y;
      ret[getLinearIdx(i, j)] = std::hypotf(dx, dy);
    }
  }
  return ret;
}();

inline int ButtonDistanceTable::getLinearIdx(const int i, const int j) {
  // https://stackoverflow.com/a/27088560
  return (BUTTON_COUNT * (BUTTON_COUNT - 1) / 2) - (BUTTON_COUNT - i) * ((BUTTON_COUNT - i) - 1) / 2 + j - i - 1;
}

float ButtonDistanceTable::operator()(const int i, const int j) const {
  if(i == j) return 0.F;
  if(i > j) return distances[getLinearIdx(j, i)];
  return distances[getLinearIdx(i, j)];
}

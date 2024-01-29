#ifndef _COLOR_H_
#define _COLOR_H_

#include <algorithm>
#include "../../Animation.hpp"

struct __attribute__((__packed__)) Color {
	Color() = default;
	Color(const Color&) = default;

	constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.F) {}
	constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
	constexpr Color(const Color &c, float a) : r(c.r), g(c.g), b(c.b), a(a) {}
	explicit constexpr Color(const RGB &c, float a = 1.F) :
		r(c.r/255.F), g(c.g/255.F), b(c.b/255.F), a(a) {}

	float r;
	float g;
	float b;
	float a;

	inline explicit operator RGB() const {
		return RGB(
			uint8_t(std::clamp(r, 0.F, 1.F)*255.F),
			uint8_t(std::clamp(g, 0.F, 1.F)*255.F),
			uint8_t(std::clamp(b, 0.F, 1.F)*255.F)
		);
	}

	constexpr Color operator/(const float d) const {
		const auto recip = 1.F / d;
		return {r * recip, g * recip, b * recip, a * recip};
	}
	constexpr Color operator*(const float m) const {
		return {r * m, g * m, b * m, a * m};
	}

	constexpr Color operator+(const Color &c) const {
		return {r + c.r, g + c.g, b + c.b, a + c.a};
	}
	constexpr Color operator*(const Color &m) const {
		return {r * m.r, g * m.g, b * m.b, a * m.a};
	}

	constexpr Color over(const Color &under) const {
		if(a >= 1.F) {
			return *this;
		}else if(a <= 0.F) {
			return under;
		}
		const float underACoeff = (1 - a) * under.a;
		return ((*this * a + under * underACoeff) / (a + underACoeff));
	}
	constexpr Color under(const Color &over) const {
		return over.over(*this);
	}
	constexpr Color flip() const {
		return {1.F - r, 1.F - g, 1.F - b};
	}
};
	#endif
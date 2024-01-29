// button_waves.h

#ifndef _BUTTON_WAVES_H_
#define _BUTTON_WAVES_H_

#include <array>
#include "../../Animation.hpp"
#include "hardware/timer.h"
#include "Color.hpp"
#include "ring_buffer.hpp"
#include "Wave.hpp"

#include "BakedButtonDistances.hpp"


class ButtonWaves : public Animation {
public:
	ButtonWaves(PixelMatrix &matrix);
  ButtonWaves(PixelMatrix &matrix, std::vector<Pixel> &pixels);
	~ButtonWaves();
private:
	void init();
public:
	void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();


	enum class MaskSetting : int {
		wavesOfBg, bgTrail, delayedFadeout, fadeAfterIdle, alwaysOn, size
	} currentMaskSetting = MaskSetting::fadeAfterIdle;
	                                                         //The maximum amount of time a wave can be visible is from when it is spawned on the left or right edge
private:                                                 //and has to travel the whole screen, plus the time it takes for the innermost edge of the bleed to run off the far edge
	static constexpr int maxWaves = 12;                  //(display width + (waveWidth + bleedRadius) / onlyInnerHalf) / (displayUnits/sec) * ms/s + slop
	static constexpr float waveBrightness = 1.F;       //        v              v            v               v                   v             v      v
	static constexpr float maxWaveAgeMillis = 340.F; //(       7.5      + (   0.25   +     1.0    ) /       2      ) /         24         * 1000 + ~1.5
	static constexpr float waveWidth = 0.25f; //fullstrength width
	static constexpr float bleedRadius = 1.00f; //light gradient fadeinout over this radius
	static constexpr float velocityPerMilli = 24 / 1000.F;

	struct __attribute__((__packed__)) MaskValues {
		int fill;
		float dimmingPerMilli;
	};

	static constexpr std::array<MaskValues, int(MaskSetting::size)> maskOptions{
		MaskValues{0,    0.F},         //wavesOfBg
		MaskValues{3,   16.F / 1000}, //bgTrail
		MaskValues{32,  16.F / 1000}, //delayedFadeout
		MaskValues{6,  0.33F / 1000}, //fadeAfterIdle
		MaskValues{1,    0.F},         //alwaysOn
	};
	unsigned long lastMillis = -1;

	std::vector<float> bgAniMask;

	glowbox::ring_buffer<Wave, maxWaves> waves;

	void drawWave(std::vector<Color> &pixels, const Wave &w, const float minR, const float maxR);

	static constexpr float curveBrightness(const float brightness);
	static constexpr Color getBleedGradientColor(const Color &over, const float positionInGradient, const Color &under);


	Color composite(const Color &wavePx, const Color &bgPx, const float bgMask) const;
public:
	void addWave(Wave &&w);
	void addWave(const int button_index, const Color &c);

	inline void drawWaves(std::vector<Color> &onto);

	//ButtonWaves(Kaimana& kaimana);
private:
	inline float millis() {
		return time_us_64()/1000.F;
	}

	 std::vector<Pixel> *pressed;
	 unsigned long int wasPressed = 0UL;
};

inline void ButtonWaves::drawWaves(std::vector<Color> &onto) {
	const float nowMillis = millis();
	std::vector<Color> pixels{onto.size()};
	for(auto& w : waves) {
		const float age = nowMillis - w.spawnTimeMillis;
		if(age < maxWaveAgeMillis) {
			const float minFullR = age * velocityPerMilli;
			drawWave(pixels, w, minFullR, minFullR + waveWidth);
		}
	}
	const auto dt = nowMillis - lastMillis;
	for(int i = 0; i < BUTTON_COUNT; ++i) {
		onto[i] = composite(pixels[i], onto[i], bgAniMask[i]);
		bgAniMask[i] -= dt * maskOptions[int(currentMaskSetting)].dimmingPerMilli;
	}
	lastMillis = nowMillis;
}

inline ButtonWaves::MaskSetting& operator++(ButtonWaves::MaskSetting& s) {
	return s = ButtonWaves::MaskSetting((int(s) + 1) % int(ButtonWaves::MaskSetting::size));
}
inline ButtonWaves::MaskSetting& operator--(ButtonWaves::MaskSetting& s) {
	return s = ButtonWaves::MaskSetting((int(s) + int(ButtonWaves::MaskSetting::size) - 1) % int(ButtonWaves::MaskSetting::size));
}

#endif

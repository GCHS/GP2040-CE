#include "ButtonWaves.hpp"

#include <algorithm>
#include "BakedButtonDistances.hpp"

constexpr std::array<Color, 16> RANDOM_COLORS{
	Color(0.859375f, 0.0f,        0.5f),      // magenta
	Color(1.0f,      0.29296875f, 0.8515625f),// pink
	Color(1.0f,      0.0625f,     0.0f),      // red
	Color(1.0f,      0.5f,        0.0f),      // red-orange
	Color(0.859375f, 0.5f,        0.0f),      // orange
	Color(1.0f,      1.0f,        0.0f),      // yellow
	Color(0.5f,      0.859375f,   0.0f),      // lime green
	Color(0.0f,      1.0f,        0.5f),      // cyan
	Color(0.0f,      0.859375f,   0.859375f), // blue
	Color(0.0f,      0.5f,        1.0f),      // cobalt blue
	Color(0.0625f,   0.0f,        1.0f),      // violet?
	Color(0.5f,      0.0f,        0.859375f), // purple
	Color(0.5f,      0.75f,       0.859375f), // lilac?
	Color(1.0f,      1.0f,        1.0f),      // white
	Color(1.5f,      1.5f,        1.5f),      // superwhite
	Color(1.25f,     1.0f,        1.6f),      // superpurple
};

Color getRandomColor() {
	return RANDOM_COLORS[(time_us_32() * 13)%RANDOM_COLORS.size()];
}

ButtonWaves::ButtonWaves(PixelMatrix &matrix) : Animation(matrix) {}

ButtonWaves::ButtonWaves (PixelMatrix &matrix, std::vector<Pixel> &pixels) : Animation(matrix), pressed(&pixels) {}

ButtonWaves::~ButtonWaves() { pressed = nullptr; }

void ButtonWaves::Animate(RGB (&frame)[100]) {
	unsigned long int isPressed = 0UL;
	for(const auto& p : *pressed){
		auto pMask = 1UL << p.index;
		isPressed |= pMask;
		if(pMask & ~wasPressed){
			addWave(p.index, getRandomColor());
		}
	}

	std::vector<Color> hdrFramebuffer{matrix->getPixelCount()};

	for(const auto& col : matrix->pixels){
		for(const auto& px : col){
			if(px.index >= 0){
				hdrFramebuffer[px.index] = Color{frame[px.positions[0]]};
			}
		}
	}

	drawWaves(hdrFramebuffer);

	for(const auto& col : matrix->pixels){
		for(const auto& px : col){
			if(px.index >= 0){
				for(const auto led : px.positions){
					frame[led] = RGB{hdrFramebuffer[px.index]};
				}
			}
		}
	}

	wasPressed = isPressed;
}

void ButtonWaves::ParameterUp() {
	++currentMaskSetting;
}

void ButtonWaves::ParameterDown() {
	--currentMaskSetting;
}

Color ButtonWaves::composite(const Color& wavePx, const Color& bgPx, const float bgMask) const {
	switch(currentMaskSetting) {
		case ButtonWaves::MaskSetting::wavesOfBg:
			return bgPx * (wavePx.a * 2.f);
		case ButtonWaves::MaskSetting::fadeAfterIdle: {
			const auto coeff = std::clamp(bgMask, 0.f, 1.f);
			return wavePx.over(bgPx * (coeff * coeff * coeff));
		}
		case ButtonWaves::MaskSetting::bgTrail:
		case ButtonWaves::MaskSetting::delayedFadeout:
		case ButtonWaves::MaskSetting::alwaysOn: {
			return wavePx.over(bgPx * std::clamp(bgMask, 0.f, 1.f));
		}
	}
}

void ButtonWaves::addWave(Wave &&w) {
	waves.emplace_back(w);
}
void ButtonWaves::addWave(const int button_index, const Color& c) {
	bgAniMask[button_index] = maskOptions[int(currentMaskSetting)].fill;
	addWave(Wave(button_index, c, millis()));
}

//ButtonWaves::ButtonWaves(Kaimana& kaimana):kaimana{&kaimana}, lastMillis{millis()} {}

constexpr float ButtonWaves::curveBrightness(const float brightness) {
	return brightness;
}

constexpr Color ButtonWaves::getBleedGradientColor(const Color &over, const float positionInGradient, const Color &under) { //0.0 is start of gradient, 1.0 is end of gradient
	const float brightness = 1.0f - positionInGradient;
	return Color(over, over.a * curveBrightness(brightness)).over(under);
}

static ButtonDistanceTable distances;



void ButtonWaves::drawWave(std::vector<Color> &pixels, const Wave &w, const float minFullR, const float maxFullR) {
	//unsigned long startTime = micros();
	for(int i = 0; i < BUTTON_COUNT; ++i) {
		const float& d = distances(w.spawnButtonIndex, i);
		const bool fartherThanMinFull = d >= minFullR, closerThanMaxFull = d <= maxFullR;
		if(fartherThanMinFull && closerThanMaxFull) {
			pixels[i] = w.c.over(pixels[i]);
		} else if(fartherThanMinFull && d < (maxFullR + bleedRadius)) {
			bgAniMask[i] = maskOptions[int(currentMaskSetting)].fill;
			const float positionInGradient = ((d - maxFullR) / bleedRadius);
			pixels[i] = getBleedGradientColor(w.c, positionInGradient, pixels[i]);
		} else if(closerThanMaxFull && d > (minFullR - bleedRadius)) {
			const float positionInGradient = ((minFullR - d) / bleedRadius);
			pixels[i] = getBleedGradientColor(w.c, positionInGradient, pixels[i]);
		}
	}
	//Serial.print("Wave: ");
	//Serial.print(micros() - startTime);
	//Serial.println("us");
}

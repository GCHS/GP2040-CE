#include "ButtonWaves.hpp"

#include <algorithm>
#include "BakedButtonDistances.hpp"

ButtonWaves::ButtonWaves(PixelMatrix &matrix) : Animation(matrix){}

ButtonWaves::ButtonWaves (PixelMatrix &matrix, std::vector<Pixel> &pixels) : Animation(matrix), pressed(&pixels) {
	
}

ButtonWaves::~ButtonWaves() { pressed = nullptr; }

void ButtonWaves::Animate(RGB (&frame)[100]) {
	unsigned long int isPressed = 0UL;
	for(const auto& p : *pressed){
		auto pMask = 1UL << p.index;
		isPressed |= pMask;
		if(pMask & ~wasPressed){
			addWave(p.index, Color(1,1,1));
		}
	}

	std::vector<Color> hdrFramebuffer{};
	hdrFramebuffer.reserve(12);

	for(const auto& row : matrix->pixels){
		for(const auto& px : row){
			hdrFramebuffer.emplace_back(frame[px.positions[0]]);
		}
	}

	drawWaves(hdrFramebuffer);

	for(const auto& row : matrix->pixels){
		for(const auto& px : row){
			for(const auto led : px.positions){
				frame[led] = RGB{hdrFramebuffer[px.index]};
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

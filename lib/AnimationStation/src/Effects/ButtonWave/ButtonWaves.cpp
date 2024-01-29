#include "ButtonWaves.hpp"

#include <algorithm>
#include "BakedButtonDistances.hpp"

#include "../../AnimationStation.hpp"

constexpr std::array<Color, 12> RANDOM_COLORS{
	Color{1.0F, 0.0F, 0.0F}, //Red
	Color{1.0F, 0.5F, 0.0F}, //Orange
	Color{1.0F, 1.0F, 0.0F}, //Yellow
	Color{0.5F, 1.0F, 0.0F}, //LimeGreen
	Color{0.0F, 1.0F, 0.0F}, //Green
	Color{0.0F, 1.0F, 0.5F}, //Seafoam
	Color{0.0F, 1.0F, 1.0F}, //Aqua
	Color{0.0F, 0.5F, 1.0F}, //SkyBlue
	Color{0.0F, 0.0F, 1.0F}, //Blue
	Color{0.5F, 0.0F, 1.0F}, //Purple
	Color{1.0F, 0.0F, 1.0F}, //Pink
	Color{1.0F, 0.0F, 0.5F}, //Magenta
};

static constexpr float bgBrightness = 4.F/16.F;

Color getRandomColor() {
	return RANDOM_COLORS[(time_us_32() * 11)%RANDOM_COLORS.size()];
}

ButtonWaves::ButtonWaves(PixelMatrix &matrix) : Animation(matrix) {
	init();
}

ButtonWaves::ButtonWaves (PixelMatrix &matrix, std::vector<Pixel> &pixels) : Animation(matrix), pressed(&pixels) {
	for(const auto& p : *pressed) {wasPressed |= 1UL << p.index;}
	init();
}

ButtonWaves::~ButtonWaves() { pressed = nullptr; }

void ButtonWaves::init(){
	bgAniMask = std::vector<float>{};
	bgAniMask.resize(matrix->getPixelCount(), 0.F);
	for(size_t i = 0; i < bgAniMask.size(); i++){
		bgAniMask[i] = -1.F;
	}
	if(AnimationStation::options.buttonWavesMaskSetting < 0 || AnimationStation::options.buttonWavesMaskSetting >= int(MaskSetting::size)){
		AnimationStation::options.buttonWavesMaskSetting = int(currentMaskSetting);
	}else{
		currentMaskSetting = MaskSetting(AnimationStation::options.buttonWavesMaskSetting);
	}
}

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
	AnimationStation::options.buttonWavesMaskSetting = int(currentMaskSetting);
}

void ButtonWaves::ParameterDown() {
	--currentMaskSetting;
	AnimationStation::options.buttonWavesMaskSetting = int(currentMaskSetting);
}

Color ButtonWaves::composite(const Color& wavePx, const Color& bgPx, const float bgMask) const {
	switch(currentMaskSetting) {
		case ButtonWaves::MaskSetting::wavesOfBg:
			return bgPx * (wavePx.a * 2.f);
		case ButtonWaves::MaskSetting::fadeAfterIdle: {
			const auto coeff = std::clamp(bgMask, 0.f, 1.f);
			return wavePx.over(bgPx * (coeff * coeff * coeff * bgBrightness));
		}
		case ButtonWaves::MaskSetting::bgTrail:
		case ButtonWaves::MaskSetting::delayedFadeout:
		case ButtonWaves::MaskSetting::alwaysOn: {
			return wavePx.over(bgPx * (std::clamp(bgMask, 0.f, 1.f) * bgBrightness));
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

constexpr float ButtonWaves::curveBrightness(const float brightness) {
	return brightness;
}

constexpr Color ButtonWaves::getBleedGradientColor(const Color &over, const float positionInGradient, const Color &under) { //0.0 is start of gradient, 1.0 is end of gradient
	const float brightness = 1.0f - positionInGradient;
	return Color(over, over.a * curveBrightness(brightness)).over(under);
}

static ButtonDistanceTable distances;

void ButtonWaves::drawWave(std::vector<Color> &pixels, const Wave &w, const float minFullR, const float maxFullR) {
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
}

void ButtonWaves::drawWaves(std::vector<Color> &onto) {
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
		if(bgAniMask[i] < 0){
			onto[i] = {0, 0, 0};
		}else{
			onto[i] = composite(pixels[i], onto[i], bgAniMask[i]);
			bgAniMask[i] -= dt * maskOptions[int(currentMaskSetting)].dimmingPerMilli;
		}
	}
	lastMillis = nowMillis;
}
#include "RainbowScroll.hpp"

#include <array>
#include <cmath>

static const float pi = std::acos(-1);

Color rainbow(float position){
	position *= 2*pi;
	Color ret = Color{//sine waves in each channel, all 120deg out of phase with each other
		(cosf(position           )+1.f)/2.f,
		(cosf(position+2.f*pi/3.f)+1.f)/2.f,
		(cosf(position+4.f*pi/3.f)+1.f)/2.f
	};
	return ret;
}

Color rainbowSquared(float position){
	Color ret = rainbow(position);
	return ret * ret;
}

Color rainbowSquaredFlipped(float position){
	return rainbowSquared(position).flip();
}

static const std::array<Color(*)(float), 3> gradients{rainbow, rainbowSquared, rainbowSquaredFlipped};

RainbowScroll::RainbowScroll(PixelMatrix &matrix) : Animation(matrix) {
 if(AnimationStation::options.rainbowScrollGradientIndex < 0 || AnimationStation::options.rainbowScrollGradientIndex >= (int)gradients.size()){
	AnimationStation::options.rainbowScrollGradientIndex = gradientIdx;
 }else{
 	gradientIdx = AnimationStation::options.rainbowScrollGradientIndex;
 }
}

inline float normalizeXPos(float x){
	return (x-2.F)/6.F;
}

void RainbowScroll::Animate(RGB (&frame)[100]) {
	const float currentTime = time_us_64()/1'000'000.F;
	const float offset = currentTime/2.F;

	const float gradientWidth = 2.F;

	for(const auto& col : matrix->pixels){
		for(const auto& px : col){
			if(px.index < 0) continue;
			RGB color{gradients[gradientIdx](normalizeXPos(BUTTON_COORDS[px.index].x)/gradientWidth + offset)};
			for(const auto& led : px.positions){
				frame[led] = color;
			}
		}
	}
}

void RainbowScroll::ParameterUp() {
	AnimationStation::options.rainbowScrollGradientIndex = gradientIdx = (gradientIdx+1)%gradients.size();
}

void RainbowScroll::ParameterDown() {
	AnimationStation::options.rainbowScrollGradientIndex = gradientIdx = (gradientIdx+gradients.size()-1)%gradients.size();
}

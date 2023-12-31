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
	return ret*ret;
}

RainbowScroll::RainbowScroll(PixelMatrix &matrix) : Animation(matrix) {}

void RainbowScroll::Animate(RGB (&frame)[100]) {
	float offset = time_us_64()/1'000'000.F;
	for(const auto& col : matrix->pixels){
		for(const auto& px : col){
			if(px.index < 0) continue;
			RGB color{rainbow((BUTTON_COORDS[px.index].x - 2.F)/12.F + offset/2.F)};
			for(const auto& led : px.positions){
				frame[led] = color;
			}
		}
	}
	// int ledCount = matrix->getLedCount();
	// for(int i = 0; i < 100; i++){
	// 	frame[i] = RGB{rainbow(float(i)/6.F/ledCount+offset/3.F)};
	// 	frame[i] = RGB{0xFF'FF'FF'FF * (i%2)};
	// }
}

void RainbowScroll::ParameterUp() {
}

void RainbowScroll::ParameterDown() {
}

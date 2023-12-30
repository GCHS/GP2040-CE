#include "RainbowScroll.hpp"

#include <array>

RainbowScroll::RainbowScroll(PixelMatrix &matrix) : Animation(matrix) {}

void RainbowScroll::Animate(RGB (&frame)[100]) {
	for(const auto& row : matrix->pixels){
		for(const auto& px : row){
			for(const auto& led : px.positions){
				frame[led] = RGB{48, 48, 48};
			}
		}
	}
}

void RainbowScroll::ParameterUp() {
}

void RainbowScroll::ParameterDown() {
}

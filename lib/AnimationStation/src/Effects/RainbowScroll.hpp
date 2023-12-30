#ifndef _RAINBOW_SCROLL_H_
#define _RAINBOW_SCROLL_H_

#include "../Animation.hpp"
#include "hardware/clocks.h"

#include <array>
#include <stdio.h>
#include <stdlib.h>
#include "../AnimationStation.hpp"

#include "Effects/ButtonWave/Color.hpp"
#include "Effects/ButtonWave/ButtonWaves.hpp"

class RainbowScroll : public Animation {
public:
	RainbowScroll(PixelMatrix &matrix);
	~RainbowScroll() {};

	void Animate(RGB (&frame)[100]);
  void ParameterUp();
  void ParameterDown();

private:

};
#endif
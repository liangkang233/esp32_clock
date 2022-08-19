#ifndef MYCLOCK
#define MYCLOCK

#include "mytools.h"

extern TFT_eSPI tft;
extern TFT_eSprite clk;

void myclock_setup();
void myclock_loop();

#endif
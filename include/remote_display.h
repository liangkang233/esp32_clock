#ifndef REMOTE_DISPLAY
#define REMOTE_DISPLAY

#include "mytools.h"

extern TFT_eSPI tft;
extern TFT_eSprite clk;

void remote_display_loop();
void remote_display_setup();
void remote_display_close();

#endif
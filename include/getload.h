#ifndef GET_LOAD
#define GET_LOAD

#include "mytools.h"

extern TFT_eSPI tft;
extern TFT_eSprite clk;

void get_load_loop();
void get_load_setup();
void get_load_close();

#endif
#ifndef MYTOOLS
#define MYTOOLS

#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

void click();
void key_setup();
void tft_setup();
void Smart_Config(const char* ssid, const char* password);

void fun_clear(int last);
void fun_setup(int now);
void fun_loop(int now);

unsigned int get_flag();

enum myfun {
    my_clock = 0, 
    remote_play,
    get_load, 

    MAXN // 请在此行前添加模式
};

extern int LCD_BL_PWM;

#endif
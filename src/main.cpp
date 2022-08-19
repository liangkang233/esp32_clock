#include <Arduino.h>
#include "mytools.h"

//---------------初始wifi配置信息--------------------
const char ssid[] = "Xbox_Home";      // WIFI名称
const char pass[] = "liangkang233";   // WIFI密码
int LCD_BL_PWM = 200;                 // 屏幕亮度0-255
//----------------------------------------------------

static int last = -1;

void setup() {
    Serial.begin(115200);
    key_setup();
    tft_setup();
    Smart_Config(ssid, pass);
    Serial.println("Power your dreams!");
}

void loop() {
    unsigned int now = get_flag();
    if(last != now) { // 切换模式需要启动 清理函数 和 初始化函数
        fun_clear(last);
        fun_setup(now);
    }
    fun_loop(now);
    last = now;
}

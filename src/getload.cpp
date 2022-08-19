#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

#include "getload.h"
#include "font/ZdyLwFont_20.h"
#include "mytools.h"
#include "img/myJpg.h"
#include "img/earth.h"


static int Anim = 0, AprevTime = 0;

int colos(int used) {
    if (used < 15)
        return 0xFFFF;
    else if (used < 30)
        return 0x00FF;
    else if (used < 45)
        return 0x0AFF;
    else if (used < 60)
        return 0x0F0F;
    else if (used < 80)
        return 0xFF0F;
    else
        return 0xF00F;
}
void Draw_Used(int Used, int x, int y, const uint8_t* jpg, int jpgsize) // 窗口位置为x y
{
    TJpgDec.drawJpg(x-30, y-10, jpg, sizeof(cpu_jpg));
    clk.setColorDepth(8);
    clk.loadFont(ZdyLwFont_20);
    clk.createSprite(52, 6);  //创建窗口
    clk.fillSprite(0x0000);    //填充率
    int col = colos(Used);
    char Used_str[10];
    sprintf(Used_str, "%d%%", Used);
    Used /= 2;
    clk.drawRoundRect(0, 0, 52, 6, 3, 0xFFFF);  // 空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
    clk.fillRoundRect(1, 1, Used, 4, 2, col);   // 根据百分比填充上面的实心圆角矩形 区间为0-50
    clk.pushSprite(x, y);
    clk.deleteSprite();

    clk.createSprite(78, 24);
    clk.fillSprite(0x0000);
    clk.setTextDatum(CC_DATUM);
    // clk.setTextFont(3);
    clk.setTextColor(TFT_WHITE, 0x0000);
    clk.drawString(Used_str, 20, 16);
    clk.pushSprite(x+55, y-10);
    clk.deleteSprite();
    clk.unloadFont();
}
void print_used(char *text1, char* text2) {
    clk.createSprite(120, 48);
    clk.fillSprite(0x0000);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_WHITE, 0x0000);
    clk.drawString(text1, 50, 16);
    clk.drawString(text2, 50, 32);
    clk.pushSprite(65, 20);
    clk.deleteSprite();
}


void earth_imgAnim() // earth gif
{
    if (millis() - AprevTime > 65) //x ms切换一次
    {
        Anim++;
        AprevTime = millis();
    }
    if (Anim == 30) Anim = 0;
    TJpgDec.drawJpg(70, 78, earth[Anim], earth_size[Anim]);
}

// Init Web Server
WebServer Httpserver(80);
void get_load_setup(void) {
    tft.fillScreen(TFT_BLACK);//清屏
    TJpgDec.drawJpg(0, 100, logo_jpg, sizeof(logo_jpg)); // logo
    Httpserver.on(UriBraces("/update/{}"), []() {
        char text1[25], text2[25];
        int cpu_Used = (int) atof(Httpserver.arg(0).c_str());
        int mem_Used = (int) atof(Httpserver.arg(2).c_str());
        sprintf(text1, "process: %s", Httpserver.arg(1).c_str());
        sprintf(text2, "mem_free: %sG", Httpserver.arg(3).c_str());
        // sprintf(text3, "recv: %d send: %d", atoi(Httpserver.arg(4).c_str())/1024, atoi(Httpserver.arg(5).c_str())/1024);
        Serial.printf("cpu:%d men:%d\n", cpu_Used, mem_Used);
        Serial.printf("%s, %s\n", text1, text2);
        print_used(text1, text2);
        Draw_Used(cpu_Used, 90, 188, cpu_jpg, sizeof(cpu_jpg));
        Draw_Used(mem_Used, 90, 215, mem_jpg, sizeof(mem_jpg));
    });
    Httpserver.begin();
}

void get_load_loop() {
    Httpserver.handleClient();
    earth_imgAnim();
}

void get_load_close() {
    Httpserver.close();
}

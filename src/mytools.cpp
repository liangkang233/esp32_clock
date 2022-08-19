#include <OneButton.h>
#include <analogWrite.h>
#include <EEPROM.h>
#include <WiFi.h>
#include "qr.h"
#include "mytools.h"
#include "myclock.h"
#include "getload.h"
#include "remote_display.h"

#define doownload_key 16    // 下载按键IO

uint16_t  PROGMEM dmaBuffer1[32 * 32]; // Toggle buffer for 32*32 MCU block, 1024bytes
uint16_t  PROGMEM dmaBuffer2[32 * 32]; // Toggle buffer for 32*32 MCU block, 1024bytes
uint16_t* dmaBufferPtr = dmaBuffer1;
bool dmaBufferSel = 0;

OneButton button(doownload_key, true, true); // 使用下载按钮做切换

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);

static unsigned int flag = 0;

void doubleclick() {
  flag = (flag + MAXN - 1) % MAXN;
  Serial.println("double_click");
  delay(50);
}

void click() {
  flag = (flag + 1) % MAXN;
  Serial.println("click");
  delay(50);
}

void longclick() {
  Serial.println("long_click");
  delay(50);
}

void key_setup(){
    button.attachClick(click);
    button.attachDoubleClick(doubleclick);
    button.attachLongPressStart(longclick);
}

// 使用dma 需要 如下释放通道使用
// tft.startWrite();//必须先使用startWrite，以便TFT芯片选择保持低的DMA和SPI通道设置保持配置
// TJpgDec.drawJpg(0,0,img_buff, sizeof(img_buff));//在左上角的0,0处绘制图像——在这个草图中，DMA请求在回调 tft_output() 中处理
// tft.endWrite();//必须使用endWrite来释放TFT芯片选择和释放SPI通道吗
bool tft_output_dma(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;

  if (dmaBufferSel) dmaBufferPtr = dmaBuffer2;
  else dmaBufferPtr = dmaBuffer1;
  dmaBufferSel = !dmaBufferSel; // Toggle buffer selection
  tft.pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr); // Initiate DMA - blocking only if last DMA is not complete
  return 1;
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}


void tft_setup() {
    if (!SPIFFS.begin(true)) {
        Serial.println("安装SPIFFS时发生错误");
        return;
    }
    EEPROM.begin(1024);
    pinMode(TFT_CS, OUTPUT);
    Serial.printf("初始亮度为%d\n", LCD_BL_PWM);
    // analogWrite(TFT_BL, LCD_BL_PWM); // 板子背光控制脚没装

    tft.begin();
    tft.initDMA();
    tft.fillScreen(TFT_BLACK);//黑色

    // jpeg图像可以按1、2、4或8的倍数缩放
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);//解码成功回调函数
}

unsigned int get_flag() { // 检测按键并返回当前key
    button.tick();
    return flag;
}


void loading(byte delayTime, byte* loadNum) { //启动动画
    clk.setColorDepth(8);
    clk.createSprite(200, 50);
    clk.fillSprite(0x0000);
    clk.drawRoundRect(0, 0, 200, 16, 8, 0xFFFF);
    clk.fillRoundRect(3, 3, *loadNum, 10, 5, 0xFFFF);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_GREEN, 0x0000);
    clk.drawString("Connecting to WiFi", 100, 40, 2);
    clk.pushSprite(20, 67); //20 110
    clk.deleteSprite();
    *loadNum += 1;
    if (*loadNum >= 195) {
        *loadNum = 195;
    }
    delay(delayTime);
}

void Smart_Config(const char* ssid, const char* password) { // 微信配置网络
    byte loadNum = 6;
    WiFi.begin(ssid, password); //连接wifi
    while (WiFi.status() != WL_CONNECTED) {
        loading(100, &loadNum);
        if (loadNum >= 194) {
            WiFi.mode(WIFI_STA);        //设置STA模式
            tft.fillScreen(TFT_BLACK);  //清屏
            // tft.pushImage(35, 35, 170, 170, qr); //240 240 分辨率 缩小到圆盘
            TJpgDec.drawJpg(35,35, qr, sizeof(qr));
            Serial.println("\r\nWait for Smartconfig...");    //打印log信息
            WiFi.beginSmartConfig();      //开始SmartConfig，等待手机端发出用户名和密码
            while (1) {
                Serial.print(".");
                delay(100);                   // wait for a second
                if (WiFi.smartConfigDone()) { //配网成功，接收到SSID和密码
                    Serial.println("SmartConfig Success");
                    Serial.printf("SSID : %s\r\n", WiFi.SSID().c_str());
                    Serial.printf("PSW : %s\r\n", WiFi.psk().c_str());
                    break;
                }
            }
            loadNum = 194;
            break;
        }
    }
    delay(10);
    while (loadNum < 194) //让动画走完
        loading(1, &loadNum);
}

void fun_setup(int now) {
    switch(now) {
        case my_clock: {
            myclock_setup();
            break;
        }
        case remote_play: {
            remote_display_setup();
            break;
        }
        case get_load: {
            get_load_setup();
            break;
        }
        default: {
            Serial.printf("无效模式%d\n", now);
        }
    }
}

void fun_clear(int last) {
    switch(last) {
        case my_clock: {
            break;
        }
        case remote_play: {
            remote_display_close();
            break;
        }
        case get_load: {
            get_load_close();
            break;
        }
        default: {
            Serial.printf("无效模式%d\n", last);
        }
    }
}

void fun_loop(int now) {
    switch (now) {
        case my_clock: {
            myclock_loop();
            break;
        }
        case remote_play: {
            remote_display_loop();
            break;
        }
        case get_load: {
            get_load_loop();
            break;
        }
        default: {
            Serial.printf("无效模式%d\n", now);
        }
    }
}

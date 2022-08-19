#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <pgmspace.h>
#include "remote_display.h"

#define httpPort 8081 //设置监听端口

WiFiServer server; //初始化一个服务端对象
uint8_t buff[7000] PROGMEM = {0}; //每一帧的临时缓存
uint8_t img_buff[50000] PROGMEM = {0}; //用于存储tcp传过来的图片
uint16_t size_count = 0; //计算一帧的字节大小

void remote_display_setup() {
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  //client.setNoDelay(false);//关闭Nagle算法
  Serial.println("wifi is connected!");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("Port: "+String(httpPort));
  tft.fillScreen(TFT_BLACK);//黑色
  tft.setTextColor(TFT_BLACK,TFT_WHITE);
  tft.drawString("Wifi Have Connected To:",20,60,2);
  tft.drawString(WiFi.SSID(),20,80,2);

  tft.drawString("IP: "+ip.toString(),20,100,2);
  tft.drawString("Port: "+String(httpPort),20,120,2);
  Serial.println("Waiting for PC connection.........");
  server.begin(httpPort); //服务器启动监听端口号
  server.setNoDelay(true);
}

void remote_display_close() {
  server.end();
}

static uint16_t read_count = 0; //读取buff的长度
static uint8_t pack_size[2];//用来装包大小字节
static uint16_t frame_size;//当前帧大小
static float start_time, end_time; //帧处理开始和结束时间
static float receive_time, deal_time; //帧接收和解码时间
void remote_display_loop() {
  // 粘包问题 recv阻塞，长时间收不到数据就会断开
  // 断开连接原因，读取buff太快，上位机发送太快造成buff溢出，清空缓冲区会断开（FLUSH）
  WiFiClient client = server.available(); //尝试建立客户对象
  if(client){
    Serial.println("[New Client!]");
    client.write("ok");//向上位机发送下一帧发送指令
    
    while(client.connected())//如果客户端处于连接状态client.connected()
    {
      client.write("no");//向上位机发送当前帧未写入完指令
      while (client.available()) {//检测缓冲区是否有数据
        if(read_count==0)
        {
          start_time=millis();
          client.read(pack_size,2);//读取帧大小
          frame_size=pack_size[0]+(pack_size[1]<<8);
        }
        read_count=client.read(buff,7000);//向缓冲区读取数据
        memcpy(&img_buff[size_count],buff,read_count);//将读取的buff字节地址复制给img_buff数组
        size_count=size_count+read_count;//计数当前帧字节位置
        // Serial.println(size_count); // lk233 读到数据
        if(img_buff[frame_size-3]==0xaa && img_buff[frame_size-2]==0xbb && img_buff[frame_size-1]==0xcc)//判断末尾数据是否为当前帧校验位
        {
          receive_time=millis()-start_time;
          deal_time=millis();
          img_buff[frame_size-3]=0;img_buff[frame_size-2]=0;img_buff[frame_size-1]=0;//清除标志位
          tft.startWrite();//必须先使用startWrite，以便TFT芯片选择保持低的DMA和SPI通道设置保持配置
          TJpgDec.drawJpg(0,0,img_buff, sizeof(img_buff));//在左上角的0,0处绘制图像——在这个草图中，DMA请求在回调 tft_output() 中处理
          tft.endWrite();//必须使用endWrite来释放TFT芯片选择和释放SPI通道吗
          // memset(&img_buff,0,sizeof(img_buff));//清空buff
          size_count=0;//下一帧
          read_count=0;
          client.write("ok");//向上位机发送下一帧发送指令
          end_time = millis(); //计算mcu刷新一张图片的时间，从而算出1s能刷新多少张图，即得出最大刷新率
          // Serial.printf("帧大小：%d " ,frame_size);Serial.print("MCU处理速度："); Serial.print(1000 / (end_time - start_time), 2); Serial.print("Fps");
          // Serial.printf("帧接收耗时:%.2fms,帧解码显示耗时:%.2fms\n",receive_time,(millis()-deal_time));
        }
      }
    }
    client.stop();
    // Serial.println("连接中断,请复位重新创建服务端");
    click(); // 连接中断 自动按下按键切换到下一模式
  }
}

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

 U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
 
HTTPClient http;
WiFiClient client;
const unsigned long HTTP_TIMEOUT = 5000; 
bool autoConfig()
{
  WiFi.begin();                    // 默认连接保存的WIFI
  for (int i = 0; i < 20; i++)
  {
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.printDiag(Serial);
      return true;
      //break;
    }
    else
    {
      Serial.print("AutoConfig Waiting......");
      Serial.println(wstatus);
      delay(1000);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
  //WiFi.printDiag(Serial);
}
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);  // 设置自动连接
      break;
    }
    delay(1000); // 这个地方一定要加延时，否则极易崩溃重启
  }
}
void setup() {

  Serial.begin(115200);
//  pinMode(D2,OUTPUT);
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a); // choose a suitable font  (设置字体)
  u8g2.setCursor(30, 50);
  u8g2.print("  Cosmos  ");  
  u8g2.sendBuffer();
  delay(2000); 
  u8g2.clearDisplay();
  u8g2.setCursor(40, 40);
  u8g2.print("正在联网..."); 
  delay(1000); 
  u8g2.sendBuffer();
  if (!autoConfig())
  {
    u8g2.clearDisplay();
    Serial.println("Start module");
    u8g2.setCursor(40, 30);
    u8g2.print("联网失败...");  
    u8g2.setCursor(40, 50);
    u8g2.print("智能配网...");  
    u8g2.sendBuffer();
    smartConfig();
  }
    delay(1000);
    u8g2.clearDisplay();
    http.setTimeout(HTTP_TIMEOUT);

    u8g2.setCursor(0, 20);
    u8g2.print("粉丝数:");     
    u8g2.setCursor(0, 40);
    u8g2.print("关注数:");     
    u8g2.setCursor(0, 60);
    u8g2.print("播放量:");     
 }
uint8_t QieHuan  = 0;
void loop() {
  if(WiFi.status() == WL_CONNECTED){
   const char* HOST = "http://api.bilibili.com"; 
   if(QieHuan == 0 || QieHuan == 1)
   {
     String GetUrl1 = String(HOST) + "/x/relation/stat?vmid=";
     GetUrl1 += "163637592";
     http.begin(GetUrl1);
   }
   else if(QieHuan == 2)
   {
     String GetUrl2 = String(HOST) + "/x/space/upstat?mid=";
     GetUrl2 += "163637592";
     http.begin(GetUrl2);
   }
   
   
    int httpCode=http.GET();
    Serial.printf("code = %d\r\n",httpCode);
    if(httpCode > 0){
      String resBuff = http.getString();
      Serial.println(resBuff);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(resBuff);
      if (!root.success()) 
      {
        Serial.println("parseObject() failed");
        return;
      }
      if(QieHuan == 0)
      {
        long fans = root["data"]["follower"];
        Serial.println(fans);
        u8g2.setCursor(60, 20);
        u8g2.print(fans);
      }
      else if(QieHuan == 1)
      {
        long follow = root["data"]["following"];
        u8g2.setCursor(60, 40);
        u8g2.print(follow);
        u8g2.sendBuffer();
      }
      else if(QieHuan == 2)
      {
        long playnum = root["data"]["archive"]["view"];
        Serial.println(playnum);
        u8g2.setCursor(60, 60);
        u8g2.print(playnum);
        u8g2.sendBuffer();
      }
      http.end();
      QieHuan = QieHuan + 1;
      if(QieHuan == 3)
      {
        QieHuan = 0;
      }
      delay(1000);
  }
 }
}

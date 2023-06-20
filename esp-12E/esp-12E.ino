#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <i2cdetect.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

// WiFi网络名称和密码
const char* ssid = "YOUR_WIFIID";
const char* password = "YOUR_WIFIPASSWORD";
int Year = 0; int Month = 0; int Day = 0;
unsigned long previousMills = 0;  // 保存上一次刷新的时间
unsigned long interval = 1000;     // 刷新时间间隔（毫秒）
// 每个月的天数
int daysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int DHours = 0;       //剩余小时数
int DMinutes = 0;     //剩余分钟数
int DSeconds = 0;     //剩余秒数
int Duration = 0;     //倒计时时间
int startTime = 0;    //程序开始运行时间

// 创建ESP8266WebServer对象
ESP8266WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 8*3600, 60000);

//提取字符串中的数字
void extractNumbers(String str, int* numbers, int* count) {
    int num = 0;
    int index = 0;
    int strLength = str.length();

    for (int i = 0; i < strLength; i++) {
        if (isdigit(str[i])) {
            num = str[i] - '0';
            numbers[index++] = num;
            num = 0;
        }
    }

    *count = index;
}

void splitDate(int* dateArr, int& Year, int& Month, int& Day) {
    Year = dateArr[0] * 1000 + dateArr[1] * 100 + dateArr[2] * 10 + dateArr[3];
    Month = dateArr[4] * 10 + dateArr[5];
    Day = dateArr[6] * 10 + dateArr[7];
}

void handleRoot() {
  if (server.method() == HTTP_POST) {
    String jsonStr = server.arg(0);  
    Serial.println(jsonStr);
    //存放日期的数组
    int numArr[8] = {0};
    int count = 0;
    extractNumbers(jsonStr, numArr, &count);
    for(int i = 0; i < count; i++)
    {
      Serial.print(numArr[i]);
    }
    Serial.println();

    
    splitDate(numArr, Year, Month, Day);
    Serial.println(Year);
    Serial.println(Month);
    Serial.println(Day);

    server.send(200, "text/plain", "Data received by Arduino");
  } 
  else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void printDigits(int digits) {
  // 在单个数字前面添加零以保持两位数格式
  if (digits < 10) {
    Serial.print("0");
  }
  Serial.print(digits);
}
//获取当前时间
time_t getTime() {
  return timeClient.getEpochTime(); // 返回当前的时间
}

// 判断是否是闰年
int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 计算从当前年份到2023年的天数
int daysFromYearNow(int NYear) {
    int days = 0;
    for (int y = 2023; y < NYear; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }
    return days;
}

// 计算给定日期距离2023年1月1日的天数
int daysFromNow(int NYear, int NMonth, int NDay) {
    int days = 0;

    // 计算前面年份的天数
    days += daysFromYearNow(NYear);

    // 计算当前年份的前几个月的天数
    for (int m = 0; m < NMonth - 1; m++) {
        if (m == 1 && isLeapYear(NYear)) {
            days += 29;  // 闰年的二月有29天
        } else {
            days += daysOfMonth[m];
        }
    }

    // 加上当前月的天数
    days += NDay;

    return days;
}

void setup() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("before u8g2.begin()");
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    Serial.println("after u8g2.begin()");

    // 连接WiFi网络
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    timeClient.begin();

    // 打印ESP8266的IP地址
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // 启动服务器
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Server started");

    setSyncProvider(getTime);
    setSyncInterval(0.1); // 每1秒同步一次时间

    // 计算倒计时时间差
    startTime = millis();  // 记录倒计时开始的时间戳
}



void loop() {
  //i2cdetect();
  server.handleClient();  // 处理客户端请求
  unsigned long currentMills = millis();  // 获取当前时间
  //Serial.println(currentMills);

  timeClient.update();

  // 获取当前的年、月、日
  int NYear = year();
  int NMonth = month();
  int NDay = day();
  //Serial.println(timeClient.getDay());
  // Serial.println(timeClient.getFormattedTime());

  // unsigned long epochTime = timeClient.getEpochTime();
  // Serial.print("Epoch Time:");
  // Serial.println(epochTime);

  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  //OLED显示ddl时间
  u8g2.print("deadline:");
  u8g2.setCursor(0, 25);
  u8g2.print(Year);
  u8g2.setCursor(35, 25);
  u8g2.print(Month);
  u8g2.setCursor(55, 25);
  u8g2.print(Day);
  //OLED显示当前时间
  u8g2.setCursor(0,38);
  u8g2.print(NYear);
  u8g2.setCursor(35, 38);
  u8g2.print(NMonth);
  u8g2.setCursor(55, 38);
  u8g2.print(NDay);
  u8g2.setCursor(0,50);
  u8g2.print(timeClient.getFormattedTime());
  u8g2.setCursor(0,62);
  switch(timeClient.getDay())
  {
    case 1:
      u8g2.print("Monday");
      break;
    case 2:
      u8g2.print("Tuesday");
      break;
    case 3:
      u8g2.print("Wednesday");
      break;
    case 4:
      u8g2.print("Thursday");
      break;
    case 5:
      u8g2.print("Friday");
      break;
    case 6:
      u8g2.print("Saturday");
      break;
    case 7:
      u8g2.print("Sunday");
      break;
  }

  int Day1 = daysFromNow(NYear, NMonth, NDay);
  int Day2 = daysFromNow(Year, Month, Day);
  int DDays = Day2 - Day1 - 1;
  DSeconds = 60 - second();
  DMinutes = 59 - minute();
  DHours = DDays*24 + 23 - hour();
  // 倒计时显示
  u8g2.setCursor(70, 10);
  u8g2.print("Countdown:");
  u8g2.setCursor(74, 20);
  u8g2.print(DHours);
  u8g2.setCursor(110, 20);  
  u8g2.print("h");
  u8g2.setCursor(74, 40);
  u8g2.print(DMinutes);
  u8g2.setCursor(110, 40);  
  u8g2.print("m");
  u8g2.setCursor(74, 60);
  u8g2.print(DSeconds);
  u8g2.setCursor(110, 60);
  u8g2.print("s");
  u8g2.sendBuffer();
}

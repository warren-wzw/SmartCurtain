#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 13// DS18B20 on NodeMCU pin D4 
#define LEFT_MOTOR_PIN1 12 // pin 1 of left motor (D6) b-1b
#define LEFT_MOTOR_PIN2 14 // pin 2 of left motor (D5) b-1a
#define MOTOR_SPEED 1000 // speed for motor
#define MOTOR_SPEED1 400 // speed for motor
#define PIN_A A0
                                                                   // WiFi 设置
#define WIFI_MODE 1 // 1: AP模式，NodeMCU自身起一个wifi信号；2：SA模式，NodeMCU连上一个已有wifi。推荐使用AP模式
#define SSID_AP "NodeMCU_WiFi_Curtain" // for AP mode
#define PASSWORD_AP "12345678" // for AP mode
#define SSID_STA "702" // for STA mode
#define PASSWORD_STA "10309866" // for STA mode

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// 定义引脚编号
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3

IPAddress local_ip(192, 168, 1, 1); //IP for AP mode
IPAddress gateway(192, 168, 1, 1); //IP for AP mode
IPAddress subnet(255, 255, 255, 0); //IP for AP mode
ESP8266WebServer server(80);
int curtain_mode = 0; // set car drive mode (0 = stop)
 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

long lastMsg = 0;
float temp_0;
float temp_1;

// 定义变量
long duration;
int distance;

void setup() 
{
    u8g2.begin();
     DS18B20.begin();
    pinMode(trigPin, OUTPUT); // 将trigPin设置为输出
    pinMode(echoPin, INPUT); // 将echoPin设置为输入
    Serial.begin(9600); // 启动串行通信
    Serial.println("NodeMCU Curtain");
pinMode(LEFT_MOTOR_PIN1, OUTPUT);
pinMode(LEFT_MOTOR_PIN2, OUTPUT);
curtain_control(); // stop the car

//if (WIFI_MODE == 1) { // AP mode
//WiFi.softAP(SSID_AP, PASSWORD_AP);
//WiFi.softAPConfig(local_ip, gateway, subnet);
//} 
//else {                                  // STA mode
WiFi.begin(SSID_STA, PASSWORD_STA);
Serial.print("Connecting to WiFi...");
int i=0;
while (WiFi.status() != WL_CONNECTED) 
{
delay(100);
Serial.print(i++);
Serial.print(".");
delay(800);                               // 如果WiFi连接成功则返回值为WL_CONNECTED                       
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100);  
}
Serial.println("");
Serial.print("Connected! IP: ");
Serial.println(WiFi.localIP()); //the IP is needed for connection in STA mode
//}

// setup web server to handle specific HTTP requests
server.on("/", HTTP_GET, handle_OnConnect);
server.on("/auto", HTTP_GET, handle_auto);
server.on("/manual", HTTP_GET, handle_manual);
server.on("/left", HTTP_GET, handle_left);
server.on("/right", HTTP_GET, handle_right);
server.on("/stop", HTTP_GET, handle_stop);
server.onNotFound(handle_NotFound);

//start server
server.begin();
Serial.println("NodeMCU web server started.");
}

void loop()
{
  
   int val;
      val=analogRead(PIN_A);
      Serial.print("\n当前亮度为:");
      Serial.print(val);
    delay(500);
  long now = millis();
    if (now - lastMsg > 1000) //每隔一秒发送一次数据
    {
      lastMsg = now;
       DS18B20.requestTemperatures(); 
      temp_0 = DS18B20.getTempCByIndex(0); // Sensor 0 will capture Temp in Celcius
  
      Serial.print("\n当前温度为");
                Serial.print(temp_0);                   
    }
      
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    //将trigPin设置为HIGH状态10微秒
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // 读取echoPin，以微秒为单位返回声波传播时间
    duration = pulseIn(echoPin, HIGH);
    
    // 计算距离
    distance= duration*0.034/2;
     
    //网页控制开始 ****************************************************************************** 
   server.handleClient();
   curtain_control();
   
   //电机开始启动 
   if(handle_auto())
   {
    if(val<=100)
 
   {
   
   if(distance>=20) 
   {
analogWrite(LEFT_MOTOR_PIN1, MOTOR_SPEED);
digitalWrite(LEFT_MOTOR_PIN2, LOW);
    } 
    int speed2=800;
    int speed3=400;
    int i=0;
    if(distance<20&&distance >=10)
    {
      for(i=0;i<5;i++)
      {   speed2=speed2-80;
      digitalWrite(LEFT_MOTOR_PIN1, LOW);
analogWrite(LEFT_MOTOR_PIN2, speed2);
      }
      }
       if(distance<10 )
    {
      for(i=0;i<3;i++)
      {   speed3=speed3-30;
      digitalWrite(LEFT_MOTOR_PIN1, LOW);
analogWrite(LEFT_MOTOR_PIN2, speed3);
      }
      }
      if(distance<5 )
    {
    
      digitalWrite(LEFT_MOTOR_PIN1, LOW);
analogWrite(LEFT_MOTOR_PIN2, 0);
  
      }
      
   }
}
  
//******************************************************************************
      
//OLED显示
int light=1023-val;
     u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
    u8g2.setCursor(0,15);
     u8g2.print("The distance is : ");
      u8g2.print(distance);
       u8g2.print(" cm");
     u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
    u8g2.setCursor(0,25);
   
      u8g2.print("the light is : ");
     u8g2.print(light);
     u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
    u8g2.setCursor(0,40);
     u8g2.println("The current temperature is:");
      u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
    u8g2.setCursor(0,55);
     u8g2.print("is : ");  
    u8g2.print(temp_0);
     u8g2.drawCircle(55,45,1,U8G2_DRAW_ALL);
  u8g2.setCursor(60,55);
  u8g2.print("C");  
  }
  
  while(u8g2.nextPage());
  
   {}
   
  
}
                                          //loop函数结束
// HTTP request: on connect
void handle_OnConnect() {
curtain_mode = 0;
Serial.println("Client connected");
server.send(200, "text/html", SendHTML());
}

// HTTP request: stop car
void handle_stop() {
curtain_mode = 0;
Serial.println("Stopped");
server.send(200, "text/html", SendHTML());
}
// HTTP request:auto
int handle_auto() 
{
  
 Serial.println("Auto");
server.send(200, "text/html", SendHTML1());
return 1;
}
// HTTP request: manual
void handle_manual() {
curtain_mode = 1;
Serial.println("manual");
server.send(200, "text/html", SendHTML());
}


// HTTP request: turn left
void handle_left() {
curtain_mode = 3;
Serial.println("Turn left...");
server.send(200, "text/html", SendHTML());
}

// HTTP request: turn right
void handle_right() {
curtain_mode = 4;
Serial.println("Turn right...");
server.send(200, "text/html", SendHTML());
}

// HTTP request: other
void handle_NotFound() {
curtain_mode = 0;
Serial.println("Page error");
server.send(404, "text/plain", "Not found");
}

// 电机运动模式
void curtain_control() {
switch (curtain_mode) {
case 0: // stop car
digitalWrite(LEFT_MOTOR_PIN1, LOW);
digitalWrite(LEFT_MOTOR_PIN2, LOW);
break;
case 1: // stop car
 
break;

case 3: // turn left
digitalWrite(LEFT_MOTOR_PIN1, LOW);
analogWrite(LEFT_MOTOR_PIN2, MOTOR_SPEED);
break;

case 4: // turn right
analogWrite(LEFT_MOTOR_PIN1, MOTOR_SPEED);
digitalWrite(LEFT_MOTOR_PIN2, LOW);

}
}

//发送html
String SendHTML() {
String html = "<!DOCTYPE html>"
"<html>"
"<head>"
"<title>NodeMCU Wifi  </title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"</head>"
"<body>"
"<div align=\"center\">"
"<h3>NodeMCU Wifi curtain by wzw</h3>"
"<br>"
"<form method=\"GET\">"
"<br><br>"
"<input type=\"button\" value=\"AUTO\" onclick=\"window.location.href='/auto'\">"
"<br><br>"
"<input type=\"button\" value=\"GO\" onclick=\"window.location.href='/left'\">"
"<br><br>"
"<input type=\"button\" value=\"BACK\" onclick=\"window.location.href='/right'\">"
"<br><br>"
"<input type=\"button\" value=\"STOP\" onclick=\"window.location.href='/stop'\">"
"</form>"
"</body>"
"</html>";
return html;
}
String SendHTML1() {
String html1 = "<!DOCTYPE html>"
"<html>"
"<head>"
"<title>NodeMCU Wifi  </title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"</head>"
"<body>"
"<div align=\"center\">"
"<h3>NodeMCU Wifi curtain by wzw</h3>"
"<br>"
"<form method=\"GET\">"
"<br><br>"
"<input type=\"button\" value=\"MANUAL\" onclick=\"window.location.href='/manual'\">"
"</form>"
"</body>"
"</html>";
return html1;
}

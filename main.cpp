#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const int MPU = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int16_t IAcX, IAcY, IAcZ, ITmp, IGyX, IGyY, IGyZ;

// const char* ssid = "WLED-AP";
// const char* password = "wled1234";
// const char* serverName = "http://4.3.2.1/json/state";

const char *ssid = "mmlab206";
const char *password = "mmlab206";
const char *serverName = "http://192.168.31.242/json/state";

int NumLeds = 100;
int liftHeight = 60;
int BanNum = 3;         // 瓣数量
int BanLength = 10;     // 瓣长度
int TailLength = 3;     // 拖尾长度
int NumFiber = 10;      // 光纤数量

unsigned long startTime = 0;
bool energyAccumulationStarted = false;
float accumulatedEnergy = 0;

void sendPostRequest(String payload)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

void setup()
{
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  // 读取初始值
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, true);
  IAcX = Wire.read() << 8 | Wire.read();
  IAcY = Wire.read() << 8 | Wire.read();
  IAcZ = Wire.read() << 8 | Wire.read();
  IGyX = Wire.read() << 8 | Wire.read();
  IGyY = Wire.read() << 8 | Wire.read();
  IGyZ = Wire.read() << 8 | Wire.read();

  // 初始化所有的灯珠亮度为0
  sendPostRequest("{\"seg\":[{\"bri\":0}]*4}");
}
String generatePayloadWithTail(int start, int bloomIndex, int tailLength)
{
  DynamicJsonDocument doc(1024);
  JsonArray segArray = doc.createNestedArray("seg");
  JsonObject seg0 = segArray.createNestedObject();
  seg0["id"] = 0;
  seg0["start"] = 0;
  seg0["stop"] = NumLeds;
  seg0["bri"] = 0;

  for (int i = 0; i < BanNum; i++)
  {
    JsonObject seg = segArray.createNestedObject();
    seg["start"] = start + bloomIndex - tailLength + i * BanLength;
    seg["stop"] = start + bloomIndex + 1 + i * BanLength;
    seg["bri"] = 255;
    seg["n"] = "2-subbloom" + String(i);
  }

  String payload;
  serializeJson(doc, payload);
  return payload;
}

void loop()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 12, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  // 计算加速度计平方和
  float accelSum = sqrt(pow((AcX - IAcX) / 1000.0, 2) + pow((AcY - IAcY) / 1000.0, 2) + pow((AcZ - IAcZ) / 1000.0, 2));

  // 计算陀螺仪平方和
  float gyroSum = sqrt(pow((GyX - IGyX) / 1000.0, 2) + pow((GyY - IGyY) / 1000.0, 2) + pow((GyZ - IGyZ) / 1000.0, 2));

  // Serial.print("Accelerometer: ");
  // Serial.print("X = "); Serial.print((AcX-IAcX)/1000);
  // Serial.print(" | Y = "); Serial.print((AcY-IAcY)/1000);
  // Serial.print(" | Z = ");  Serial.println((AcZ-IAcZ)/1000);
  // Serial.print("Gyroscope: ");
  // Serial.print("X  = "); Serial.print((GyX-IGyX)/1000);
  // Serial.print(" | Y = "); Serial.print((GyY-IGyY)/1000);
  // Serial.print(" | Z = "); Serial.println((GyZ-IGyZ)/1000);
  // Serial.println(" ");

  // 输出加速度计和陀螺仪的平方和
  // Serial.print("Accelerometer Sum of Squares: ");
  // Serial.println(accelSum); // 2以下视为禁止，50视为最大值
  // Map accelSum from range 2-50 to 1-15
  int mapValue = map(accelSum, 2, 50, 1, 30);
  // i = constrain(i, 1, 15);
  // Serial.print("Mapped value: ");
  // Serial.println(i);

  // Serial.print("Gyroscope Sum of Squares: ");
  // Serial.println(gyroSum);

  // Serial.print(accelSum);
  // Serial.print(",");
  // Serial.println(gyroSum);

  // accelSum超过20认为摇晃
  if (accelSum > 20 && !energyAccumulationStarted)
  {
    Serial.println("Start!!!!=============");

    // 3秒累计能量
    startTime = millis();
    energyAccumulationStarted = true;

    accumulatedEnergy += accelSum;
  }

  if (energyAccumulationStarted)
  {
    accumulatedEnergy += accelSum;
  }

  if (energyAccumulationStarted && (millis() - startTime >= 3000))
  {
    Serial.print("Accumulated Energy: ");
    Serial.println(accumulatedEnergy);
    // 完全不动是100
    // 轻微摇晃<500
    // 手腕抖动<2000
    // 疯狂抖<4000
    float speedRadio = 1.0; // 速度系数
    speedRadio=map(accumulatedEnergy,0,4000,0,10);
    speedRadio=10-speedRadio;
    speedRadio*=0.1;
    
    Serial.println("=============speedRadio===============");
    Serial.println(speedRadio);


    // 1. 一个灯珠从最开始移动到第10位，模拟烟花飞上天空，移动速度0.1s/led
    // for (int i = 0; i <= liftHeight; i++)
    for (int i = liftHeight; i >= 0; i--)
    {
      String payload = "{\"seg\":[{\"id\":0,\"start\":0,\"stop\":" + String(NumLeds) + ",\"bri\":0},{\"id\":1,\"start\":" + String(i) + ",\"stop\":" + String(i + 1) + ",\"bri\":255,\"n\":\"1-lift\"}]}";
      sendPostRequest(payload);
      delay(30*speedRadio);
    }
    delay(100);
    // 2. 3个5灯珠灯带，模拟烟花炸开效果
    int fireworkStart = liftHeight + 1;
    // 2.5 同时，点亮光纤(假设有5条)
    int guangqianStart = fireworkStart + BanLength * BanNum;

    String payload = "{\"seg\":[{\"id\":4,\"bri\":255,\"start\":" + String(guangqianStart) + ",\"stop\":" + String(guangqianStart + NumFiber) + ",\"n\":\"guangqian\"}]}";
    sendPostRequest(payload);

    for (int bloomIndex = 0; bloomIndex < BanLength; bloomIndex++)
    {
      String payload;
      if (bloomIndex < TailLength)
      {
        payload = generatePayloadWithTail(fireworkStart, bloomIndex, bloomIndex);
      }
      else
      {
        payload = generatePayloadWithTail(fireworkStart, bloomIndex, TailLength);
      }
      sendPostRequest(payload);
      delay(300*speedRadio);
    }

    // 到头之后拖尾后处理
    for (int tuo = TailLength; tuo >= -1; tuo--)
    {
      String payload = generatePayloadWithTail(fireworkStart, BanLength - 1, tuo);
      sendPostRequest(payload);
      delay(300*speedRadio);
    }
    // 重置能量累积
    energyAccumulationStarted = false;
    accumulatedEnergy = 0;

    delay(5000);
  }

  IAcX = AcX;
  IAcY = AcY;
  IAcZ = AcZ;
  IGyX = GyX;
  IGyY = GyY;
  IGyZ = GyZ;

  delay(50);
}
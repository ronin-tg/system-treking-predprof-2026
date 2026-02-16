#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>

WiFiUDP udp;
Adafruit_MPU6050 mpu;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

void setup() {
  Wire.begin(8, 9); 
  if (!mpu.begin()) { while(1); }
  mpu.setI2CBypass(true); 
  mag.begin();

  WiFi.begin("Stick_Hub", "12345678");
  while (WiFi.status() != WL_CONNECTED) delay(100);
}

void loop() {
  sensors_event_t a, g, temp, m;
  mpu.getEvent(&a, &g, &temp);
  mag.getEvent(&m);

  String p = String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z) + "," +
             String(g.gyro.x) + "," + String(g.gyro.y) + "," + String(g.gyro.z) + "," +
             String(m.magnetic.x) + "," + String(m.magnetic.y) + "," + String(m.magnetic.z);

  udp.beginPacket("192.168.4.1", 1234); // IP Стика всегда такой в режиме AP
  udp.print(p);
  udp.endPacket();
  delay(20); 
}

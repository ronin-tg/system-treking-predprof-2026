#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const int PIN_STEP   = 4;
const int PIN_DIR    = 5;
const int PIN_ENABLE = 6;

const int stepDelay = 200; 
const int stepsPerRevolution = 2300; 

WebServer server(80);

void moveGateSteps(bool direction, int steps) {
  digitalWrite(PIN_DIR, direction ? HIGH : LOW);
  digitalWrite(PIN_ENABLE, LOW); 
  
  for(int i = 0; i < steps; i++) {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(stepDelay);
  }
  
  digitalWrite(PIN_ENABLE, HIGH); 
}

void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  
  delay(2000); 
  moveGateSteps(true, stepsPerRevolution); // Тестовый проезд при включении

  WiFi.begin("Stick_Hub", "12345678");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.on("/move", []() {
    int dir = server.arg("dir").toInt();
    int stp = server.arg("steps").toInt();
    
    server.send(200, "text/plain", "OK");
    
    if (stp > 0) {
      moveGateSteps((dir == 1), stp);
    }
  });
  
  server.begin();
}

void loop() {
  server.handleClient();
}

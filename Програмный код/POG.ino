#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <vector>
#include <numeric>

enum class SystemLogic {
    BOOT_UP,
    SENSE_CALIBRATION,
    ACTIVE_AUTO_PILOT
};

SystemLogic currentLogic = SystemLogic::BOOT_UP;
WiFiUDP inputUDP;
WiFiUDP broadcastUDP;

const char* AP_SSID = "Stick_Hub";
const char* AP_PASS = "12345678";
const char* BCAST_ADDR = "192.168.4.255";
const int IN_PORT = 1234;
const int OUT_PORT = 12345;

std::vector<float> calibHeap;
std::vector<float> signalWindow;
float ayZeroOffset = 0.0f;
int gatePositionStatus = 3;
volatile int scoreRegistry = 0;
unsigned long commandCooldown = 0;

void IRAM_ATTR irqScoreCounter() {
    static unsigned long lastIrqTime = 0;
    unsigned long now = millis();
    if (now - lastIrqTime > 600) {
        scoreRegistry++;
        lastIrqTime = now;
    }
}

void transmitGatewayCommand(int pos) {
    if (millis() - commandCooldown < 1300) return;
    static HTTPClient httpClient;
    httpClient.begin("http://192.168.4.2/move?pos=" + String(pos));
    if (httpClient.GET() == 200) {
        gatePositionStatus = pos;
        commandCooldown = millis();
    }
    httpClient.end();
}

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    StickCP2.Display.setRotation(3);
    
    WiFi.softAP(AP_SSID, AP_PASS);
    inputUDP.begin(IN_PORT);
    
    pinMode(33, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(33), irqScoreCounter, FALLING);
    
    currentLogic = SystemLogic::SENSE_CALIBRATION;
}

void loop() {
    StickCP2.update();

    int packetSize = inputUDP.parsePacket();
    if (packetSize) {
        char frameBuffer[255];
        int len = inputUDP.read(frameBuffer, 255);
        frameBuffer[len] = 0;
        String rawData(frameBuffer);

        broadcastUDP.beginPacket(BCAST_ADDR, OUT_PORT);
        broadcastUDP.write((uint8_t*)frameBuffer, len);
        broadcastUDP.endPacket();

        int c1 = rawData.indexOf(',');
        float ay_val = rawData.substring(c1 + 1, rawData.indexOf(',', c1 + 1)).toFloat();

        if (currentLogic == SystemLogic::SENSE_CALIBRATION) {
            calibHeap.push_back(ay_val);
            if (calibHeap.size() >= 120) {
                ayZeroOffset = std::accumulate(calibHeap.begin(), calibHeap.end(), 0.0f) / 120.0f;
                calibHeap.clear();
                currentLogic = SystemLogic::ACTIVE_AUTO_PILOT;
                StickCP2.Speaker.tone(1500, 200);
            }
        } 
        else if (currentLogic == SystemLogic::ACTIVE_AUTO_PILOT) {
            float ay_pure = ay_val - ayZeroOffset;
            signalWindow.push_back(ay_pure);
            if (signalWindow.size() > 15) signalWindow.erase(signalWindow.begin());

            float avgSignal = std::accumulate(signalWindow.begin(), signalWindow.end(), 0.0f) / signalWindow.size();

            if (abs(avgSignal) > 1100.0f) {
                int target = 3;
                if (avgSignal > 3800.0f) target = 5;
                else if (avgSignal > 1200.0f) target = 4;
                else if (avgSignal < -3800.0f) target = 1;
                else if (avgSignal < -1200.0f) target = 2;

                if (target != gatePositionStatus) transmitGatewayCommand(target);
            }
        }
    }

    StickCP2.Display.fillRect(0, 40, 240, 95, BLACK);
    StickCP2.Display.setCursor(10, 50);
    StickCP2.Display.setTextSize(3);
    StickCP2.Display.printf("GOALS: %d", scoreRegistry);
    StickCP2.Display.setCursor(10, 90);
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.printf("GATE: %d | NET: OK", gatePositionStatus);
}

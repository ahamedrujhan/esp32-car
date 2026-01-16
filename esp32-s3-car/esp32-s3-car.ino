#include <AsyncTCP.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* ================= Motor Pins ================= */
// Motor A
#define ENA 4
#define IN1 5
#define IN2 6

// Motor B
#define ENB 7
#define IN3 15
#define IN4 16

int speedVal = 220; // 0–255

/* ================= BLE UUIDs ================= */
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

BLECharacteristic *pCharacteristic;

/* ================= Dead-Man Settings ================= */
unsigned long lastCmdTime = 0;
const unsigned long CMD_TIMEOUT = 300; // milliseconds

bool motorRunning = false;

/* ================= Motor Functions ================= */
void forward();
void backward();
void left();
void right();
void stopMotors();

/* ================= BLE Callbacks ================= */
class CmdCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override {
    String value = pChar->getValue();
    if (value.length() == 0) return;

    char cmd = value[0];
    lastCmdTime = millis();   // refresh heartbeat
    motorRunning = true;

    Serial.write(cmd);
    Serial.write('\n');

    switch (cmd) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': left(); break;
      case 'R': right(); break;
      case 'S':
        stopMotors();
        motorRunning = false;
        break;
      default:
        Serial.println("❓ Unknown Command");
        break;
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onDisconnect(BLEServer* pServer) override {
    Serial.println("🔌 BLE Disconnected → STOP");
    stopMotors();
    motorRunning = false;
    pServer->startAdvertising();
  }
  
  void onConnect(BLEServer* pServer) override {
     Serial.println("✅ BLE Connected");
  }
};

/* ================= Setup ================= */
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("BOOT OK");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // ESP32 Core 3.x LEDC API
  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);

  BLEDevice::init("Ruju's_ESP32_S3_CAR");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
     CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  BLE2902 *p2902 = new BLE2902();
  pCharacteristic->addDescriptor(p2902);
  pCharacteristic->setValue("S");  // initial STOP
  pCharacteristic->setCallbacks(new CmdCallback());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  stopMotors();
  Serial.println("🚗 BLE Car Ready (Dead-Man Enabled)");
}

/* ================= Loop ================= */
void loop() {
  // Dead-man timeout check
  if (motorRunning && (millis() - lastCmdTime > CMD_TIMEOUT)) {
    Serial.println("⏱️ Command timeout → STOP");
    stopMotors();
    motorRunning = false;
  }
}

/* ================= Motor Logic ================= */
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void stopMotors() {
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

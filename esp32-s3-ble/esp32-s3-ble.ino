#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Motor A
#define ENA 4
#define IN1 5
#define IN2 6

// Motor B
#define ENB 7
#define IN3 15
#define IN4 16

int speedVal = 180; // 0–255

// BLE UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

BLECharacteristic *pCharacteristic;

// --- Global command variable (volatile for ISR safety) ---
volatile char currentCmd = 'S';

// Motor functions definition
void forward();
void backward();
void left();
void right();
void stopMotors();

class CmdCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
  String value = pChar->getValue();
  if (value.length() == 0) return;

    char cmd = value.charAt(0);
    Serial.print("BLE CMD: ");
    Serial.println(cmd);

    switch (cmd) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': left(); break;
      case 'R': right(); break;
      case 'S': stopMotors(); break;
      default : 
        Serial.print("❓ Unknown Command: "); 
        Serial.println(cmd); 
        break;
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(2000);          // allow ESP32-S3 to stabilize
  Serial.println("BOOT OK");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // New LEDC API (ESP32 Core 3.x)
  ledcAttach(ENA, 1000, 8);  // pin, frequency (Hz), resolution (bits)
  ledcAttach(ENB, 1000, 8);

  BLEDevice::init("Ruju's_ESP32_S3_CAR");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setCallbacks(new CmdCallback());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  stopMotors();
  Serial.println("🚗 ESP32 Bluetooth Car Ready");
  Serial.println("Commands: F B L R S");
}

void forward() {
  Serial.println("➡️ FORWARD");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void backward() {
  Serial.println("⬅️ BACKWARD");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void left() {
  Serial.println("↩️ LEFT");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void right() {
  Serial.println("↪️ RIGHT");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, speedVal);
  ledcWrite(ENB, speedVal);
}

void stopMotors() {
  Serial.println("⛔ STOP");
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

// void demoMotion() {
//   forward();   delay(2000);
//   left();      delay(1000);
//   right();     delay(1000);
//   backward();  delay(2000);
//   stopMotors();delay(2000);
// }

void loop() {
//
}

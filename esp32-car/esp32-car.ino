#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Motor A
#define ENA 25
#define IN1 26
#define IN2 27

// Motor B
#define ENB 14
#define IN3 18  // changed from 12 to 18 (safe boot)
#define IN4 19  // changed from 13 to 19 (safe boot)

int speedVal = 180; // 0–255

unsigned long lastCmdTime = 0;
const unsigned long demoDelay = 3000; // 3 seconds

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Ruju'S_ESP32_CAR"); // Bluetooth name

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // New LEDC API (ESP32 Core 3.x)
  ledcAttach(ENA, 1000, 8);  // pin, frequency (Hz), resolution (bits)
  ledcAttach(ENB, 1000, 8);

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

void demoMotion() {
  forward();   delay(2000);
  left();      delay(1000);
  right();     delay(1000);
  backward();  delay(2000);
  stopMotors();delay(2000);
}

void loop() {
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    lastCmdTime = millis();

    Serial.print("📡 Command Received: ");
    Serial.println(cmd);

    switch (cmd) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': left(); break;
      case 'R': right(); break;
      case 'S': stopMotors(); break;
      default:
        Serial.println("❓ Unknown Command - " + cmd);
        break;
    }
  }

  // // 🟡 Demo mode if no Bluetooth command
  // if (millis() - lastCmdTime > demoDelay * 3) {
  //   Serial.println("🧪 Demo Mode Active");
  //   demoMotion();
  //   lastCmdTime = millis();
  // }
}

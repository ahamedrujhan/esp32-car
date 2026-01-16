#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>

/* ================= Motor Pins ================= */
#define ENA 4
#define IN1 5
#define IN2 6

#define ENB 7
#define IN3 15
#define IN4 16

int speedVal = 220;

/* ================= NeoPixel Config ================= */
#define NEOPIXEL_PIN 48
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

/* ================= NeoPixel Functions ================= */
void setupPixels() {
  pixels.begin();
  Serial.print("Pixels Initialized....");
}

void pixelStart() {
  int lastPixel = NUMPIXELS - 1;

  // Increase Brightness
  for (int i = 0; i <= 225; i++) {
    pixels.setPixelColor(lastPixel, pixels.Color(225,225,225));
    pixels.setBrightness(i);
    pixels.show();
    delay(10);
  }

  // Decrease Brightness
  for (int i = 225; i >= 0; i--) {
    pixels.setPixelColor(lastPixel, pixels.Color(225,225,225));
    pixels.setBrightness(i);
    pixels.show();
    delay(10);
  }
}

void hexToRGB(String hex, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (hex.startsWith("#")) hex = hex.substring(1); // remove #
  if (hex.length() != 6) { r = g = b = 0; return; } // invalid
  r = strtol(hex.substring(0,2).c_str(), nullptr, 16);
  g = strtol(hex.substring(2,4).c_str(), nullptr, 16);
  b = strtol(hex.substring(4,6).c_str(), nullptr, 16);
}
void stopBlink() {
  pixels.setPixelColor(0,pixels.Color(0,0,0));
  pixels.show();
}

void blinkPixel(int timeout, String ColorHash, int brightness) {
  stopBlink();
  uint8_t r, g, b;
  hexToRGB(ColorHash, r, g, b);

  pixels.setPixelColor(0,pixels.Color(r, g, b));
  pixels.setBrightness(brightness);
  pixels.show();
  delay(timeout);
}



/* ================= WiFi ================= */
const char* ssid = "Ruju's ESP32_CAR";
const char* password = "12345678";

/* ================= Web Server ================= */
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

/* ================= Dead-Man ================= */
unsigned long lastCmdTime = 0;
const unsigned long CMD_TIMEOUT = 300;
bool motorRunning = false;

/* ================= Motor Functions ================= */
void stopMotors() {
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
  motorRunning = false;
}

void forward() {
  // blinkPixel(1500, "#FFFFFF", 180);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, speedVal); ledcWrite(ENB, speedVal);
  motorRunning = true;
}

void backward() {
  // blinkPixel(1500, "#FF0000", 180);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  ledcWrite(ENA, speedVal); ledcWrite(ENB, speedVal);
  motorRunning = true;
}

void left() {
  // blinkPixel(1500, "#FFB030", 180);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, speedVal); ledcWrite(ENB, speedVal);
  motorRunning = true;
}

void right() {
  // blinkPixel(1500, "#FFB030", 180);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  ledcWrite(ENA, speedVal); ledcWrite(ENB, speedVal);
  motorRunning = true;
}

/* ================= WebSocket Handler ================= */
void onWsEvent(AsyncWebSocket *server,
               AsyncWebSocketClient *client,
               AwsEventType type,
               void *arg,
               uint8_t *data,
               size_t len) {

  if (type == WS_EVT_DATA && len > 0) {
    char cmd = data[0];
    lastCmdTime = millis();
    Serial.print("WS Command: ");
    Serial.println(cmd);

    switch (cmd) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': left(); break;
      case 'R': right(); break;
      case 'S': stopMotors(); break;
      default : Serial.print("Unknown Command...."); break;
    }
  }
}


/* ================= Controll Page ================= */

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">
<title>ESP32 Car RC</title>
<style>
  body {
    font-family: Arial, sans-serif;
    text-align: center;
    background: #f0f0f0;
    margin: 0;
    padding: 0;
  }

  h2 {
    margin-top: 20px;
    color: #333;
  }

  .dpad {
    display: inline-grid;
    grid-template-columns: 100px 100px 100px;
    grid-template-rows: 100px 100px 100px;
    gap: 15px;
    justify-content: center;
    margin-top: 30px;
  }

  button {
    width: 100px;
    height: 100px;
    border-radius: 20px;
    border: none;
    background: #4CAF50;
    cursor: pointer;
    box-shadow: 0 5px #2e7d32;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: all 0.1s ease-in-out;
  }

  button:active {
    transform: translateY(3px);
    box-shadow: 0 2px #2e7d32;
    background: #45a049;
  }

  .stop {
    background: #f44336;
    box-shadow: 0 5px #c62828;
  }

  .empty {
    background: transparent;
    box-shadow: none;
    cursor: default;
  }

  svg {
    width: 40px;
    height: 40px;
    fill: white;
  }
</style>
</head>
<body>
<h2>🚗 ESP32 Car RC</h2>

<div class="dpad">
  <button ontouchstart="send('F')" ontouchend="send('S')">
    <!-- Up arrow -->
    <svg viewBox="0 0 24 24">
      <path d="M12 2L5 9h5v10h4V9h5z"/>
    </svg>
  </button>
  <div class="empty"></div>
  <button ontouchstart="send('F')" ontouchend="send('S')">
    <svg viewBox="0 0 24 24">
      <path d="M12 2L5 9h5v10h4V9h5z"/>
    </svg>
  </button>

  <button ontouchstart="send('L')" ontouchend="send('S')">
    <!-- Left arrow -->
    <svg viewBox="0 0 24 24">
      <path d="M2 12l9-7v4h10v6H11v4z"/>
    </svg>
  </button>
  <button class="stop" ontouchstart="send('S')">
    <!-- Stop -->
    <svg viewBox="0 0 24 24">
      <rect x="6" y="6" width="12" height="12"/>
    </svg>
  </button>
  <button ontouchstart="send('R')" ontouchend="send('S')">
    <!-- Right arrow -->
    <svg viewBox="0 0 24 24">
      <path d="M22 12l-9 7v-4H3v-6h10V5z"/>
    </svg>
  </button>

  <button ontouchstart="send('B')" ontouchend="send('S')">
    <!-- Down arrow -->
    <svg viewBox="0 0 24 24">
      <path d="M12 22l7-7h-5V5h-4v10H5z"/>
    </svg>
  </button>
  <div class="empty"></div>
  <button ontouchstart="send('B')" ontouchend="send('S')">
    <svg viewBox="0 0 24 24">
      <path d="M12 22l7-7h-5V5h-4v10H5z"/>
    </svg>
  </button>
</div>

<script>
let ws = new WebSocket("ws://" + location.host + "/ws");
function send(c){ ws.send(c); }
</script>
</body>
</html>
)rawliteral";




/* ================= Setup ================= */
void setup() {
  Serial.begin(115200);

  delay(1000);
  Serial.println("ESP32 BOOT OK");

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);

  stopMotors();
  // setupPixels(); // Initalized the NeoPixels
  // pixelStart();

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
}

/* ================= Loop ================= */
void loop() {
  // if (motorRunning && millis() - lastCmdTime > CMD_TIMEOUT) {
  //   stopMotors();
  // }
}

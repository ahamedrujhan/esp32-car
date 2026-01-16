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
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 RC Car</title>

<style>
:root {
  --bg: #1b1b1b;
  --glow: #2fd6e8;
  --btn: #222;
  --text: #6fe7f2;
}

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  background: var(--bg);
  color: var(--text);
  font-family: Arial, Helvetica, sans-serif;
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
}

/* ---------- LAYOUT ---------- */
.container {
  width: 95%;
  max-width: 900px;
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  align-items: center;
}

/* ---------- D-PAD ---------- */
.dpad {
  position: relative;
  width: 220px;
  height: 220px;
  border-radius: 50%;
  box-shadow: 0 0 25px var(--glow);
  margin: auto;
}

.dpad button {
  position: absolute;
  width: 70px;
  height: 70px;
  background: transparent;
  border: none;
  color: var(--glow);
  font-size: 42px;
  font-weight: bold;

  touch-action: none;
  user-select: none;

  display: flex;
  align-items: center;
  justify-content: center;
}

.dpad button:active {
  color: white;
  transform: scale(1.2);
}

/* Arrow symbols */
.up::before    { content: "F"; }
.down::before  { content: "B"; }
.left::before  { content: "L"; }
.right::before { content: "R"; }

.up    { top: 5px; left: 50%; transform: translateX(-50%); }
.down  { bottom: 5px; left: 50%; transform: translateX(-50%); }
.left  { left: 5px; top: 50%; transform: translateY(-50%); }
.right { right: 5px; top: 50%; transform: translateY(-50%); }

/* ---------- CENTER ---------- */
.center {
  text-align: center;
}

.slider {
  width: 80%;
}

.value {
  font-size: 22px;
  margin-bottom: 5px;
}

.action-btn {
  margin: 15px;
  padding: 15px 30px;
  background: var(--btn);
  border-radius: 12px;
  border: none;
  color: var(--glow);
  font-size: 18px;
  box-shadow: 0 0 15px var(--glow);

  touch-action: none;
  user-select: none;
}

.action-btn:active {
  transform: scale(1.1);
  color: white;
}

/* ---------- RIGHT CLUSTER ---------- */
.cluster {
  display: grid;
  grid-template-columns: repeat(3, 80px);
  grid-template-rows: repeat(3, 80px);
  gap: 15px;
  justify-content: center;
}

.cluster button {
  border-radius: 50%;
  border: none;
  background: var(--btn);
  color: var(--glow);
  font-size: 22px;
  font-weight: bold;
  box-shadow: 0 0 15px var(--glow);

  touch-action: none;
  user-select: none;

  display: flex;
  align-items: center;
  justify-content: center;
}

.cluster button:active {
  transform: scale(1.15);
  color: white;
  box-shadow: 0 0 25px white;
}

.cluster .f { grid-column: 2; }
.cluster .l { grid-column: 1; grid-row: 2; }
.cluster .r { grid-column: 3; grid-row: 2; }
.cluster .b { grid-column: 2; grid-row: 3; }

/* ---------- MOBILE ---------- */
@media (max-width: 768px) {
  .container {
    grid-template-columns: 1fr;
    gap: 30px;
  }
}
</style>
</head>

<body>
<div class="container">

  <!-- LEFT : D-PAD -->
  <div class="dpad">
    <button class="up"    ontouchstart="send('F')" ontouchend="send('S')"></button>
    <button class="down"  ontouchstart="send('B')" ontouchend="send('S')"></button>
    <button class="left"  ontouchstart="send('L')" ontouchend="send('S')"></button>
    <button class="right" ontouchstart="send('R')" ontouchend="send('S')"></button>
  </div>

  <!-- CENTER -->
  <div class="center">
    <div class="value" id="spd">50</div>
    <input class="slider" type="range" min="0" max="100" value="50"
           oninput="updateSpeed(this.value)">
    <div>Speed</div>

    <button class="action-btn" ontouchstart="send('H')">Horn</button>
    <button class="action-btn" ontouchstart="send('I')">Light</button>
  </div>

  <!-- RIGHT : CLUSTER -->
  <div class="cluster">
    <button class="f" ontouchstart="send('F')" ontouchend="send('S')">F</button>
    <button class="l" ontouchstart="send('L')" ontouchend="send('S')">L</button>
    <button class="r" ontouchstart="send('R')" ontouchend="send('S')">R</button>
    <button class="b" ontouchstart="send('B')" ontouchend="send('S')">B</button>
  </div>

</div>

<script>
let ws = new WebSocket("ws://" + location.host + "/ws");

function send(cmd) {
  if (ws.readyState === 1) {
    ws.send(cmd);
  }
}

function updateSpeed(v) {
  document.getElementById("spd").innerText = v;
  send("V" + v);
}
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

#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <WebSocketsServer.h>
#include <Adafruit_NeoPixel.h>

// השלט משדר AP – הדפדפן מתחבר ישירות
const char* ssid     = "ESP32_Console_V2";
const char* password = "87654321";

// OLED 0.42" – SSD1306 72x40
U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

WebServer        httpServer(80);
WebSocketsServer wsServer(81);

// ─── פינים ───────────────────────────────────────────────
#define SDA_PIN 5
#define SCL_PIN 6

// ג'ויסטיק: VRX → IO3, VRY → IO4, SW → IO7
#define VRX_PIN 3
#define VRY_PIN 4
#define SW_PIN  7

// כפתורים דיגיטליים (active LOW, INPUT_PULLUP)
#define LEFT_PIN  0   // SW_LEFT  = IO0
#define RIGHT_PIN 9   // SW_RIGHT = IO9

// מנוע רטט דרך MOSFET Q1
#define MOTOR_PIN 10

// סוללה – מחלק מתח R12=30kΩ (עליון) + R13=110kΩ (תחתון)
// 4.2V × 110/(30+110) = 3.3V → ADC בטוח ב-3.3V מקסימום
#define BATTERY_PIN   1         // IO1 = T-Vbat
#define R_TOP_KOHM    30.0f     // R12
#define R_BOT_KOHM   110.0f     // R13

// 4 לדים RGB (WS2812B) בשרשור על IO2
#define LEDS_PIN  2
#define NUM_LEDS  4
Adafruit_NeoPixel strip(NUM_LEDS, LEDS_PIN, NEO_GRB + NEO_KHZ800);

// ─── קבועי פולס ─────────────────────────────────────────
const unsigned long MOTOR_PULSE_MS = 120;
const unsigned long LED_PULSE_MS   = 350;

// ─── מצב קודם לזיהוי שינויים ────────────────────────────
bool    lastBtnLeft        = false;
bool    lastBtnRight       = false;
bool    lastJoyLeft        = false;
bool    lastJoyRight       = false;
bool    lastJoyUp          = false;
bool    lastJoyDown        = false;
bool    lastSelect         = false;
int     lastTilt           = 0;
bool    lastAttack         = false;
bool    lastMotor          = false;
uint8_t lastLed            = 0;
float   lastBatteryVoltage = 0.0f;

// ─── setup ───────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(2000);

  // ג'ויסטיק: ADC_11db מרחיב טווח ל-0–3.3V (ברירת מחדל: 0–1.1V)
  analogSetPinAttenuation(VRX_PIN, ADC_11db);
  analogSetPinAttenuation(VRY_PIN, ADC_11db);

  pinMode(LEFT_PIN,  INPUT_PULLUP);  // SW_LEFT  – active LOW
  pinMode(RIGHT_PIN, INPUT_PULLUP);  // SW_RIGHT – active LOW
  pinMode(SW_PIN,    INPUT_PULLUP);  // SW_JOY   – active LOW

  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  initOled();

  Wire.setTimeOut(20);
  initMpu6050();

  initAP();
  initWebServer();
  initWebSocket();
}

// ─── loop ────────────────────────────────────────────────
void loop() {
  httpServer.handleClient();
  wsServer.loop();

  // 1. כפתורים דיגיטליים
  bool btnLeft   = (digitalRead(LEFT_PIN)  == LOW);
  bool btnRight  = (digitalRead(RIGHT_PIN) == LOW);
  bool btnSelect = (digitalRead(SW_PIN)    == LOW);

  // 2. ג'ויסטיק – ADC 12-bit (0–4095), מרכז ~2048
  int  joyX     = analogRead(VRX_PIN);  // IO3
  int  joyY     = analogRead(VRY_PIN);  // IO4

  bool joyLeft  = (joyX > 3500);
  bool joyRight = (joyX < 500);
  bool joyUp    = (joyY > 3500);
  bool joyDown  = (joyY < 500);

  // 3. MPU6050 – הטיה + זיהוי מכה
  int  currentTilt  = lastTilt;
  bool specialAttack = lastAttack;
  readMpu6050(currentTilt, specialAttack);

  // 3.5 מתח סוללה: Vbat = Vadc × (R_TOP+R_BOT)/R_BOT
  int   batteryRaw     = analogRead(BATTERY_PIN);
  float batteryVoltage = (batteryRaw / 4095.0f) * 3.3f
                         * (R_TOP_KOHM + R_BOT_KOHM) / R_BOT_KOHM;

  // 4. משוב מקומי – מנוע + לדים על אירוע ג'ויסטיק (edge בלבד)
  static bool prevJoyLeft = false, prevJoyRight = false,
              prevJoyUp   = false, prevJoyDown  = false;
  static unsigned long motorPulseUntil = 0;
  static unsigned long ledPulseUntil   = 0;
  static uint8_t       pulseLedColor   = 0;

  bool joyEdge = (joyLeft  && !prevJoyLeft)  || (joyRight && !prevJoyRight) ||
                 (joyUp    && !prevJoyUp)     || (joyDown  && !prevJoyDown);

  if (joyEdge) motorPulseUntil = millis() + MOTOR_PULSE_MS;

  if      (joyLeft  && !prevJoyLeft)  { pulseLedColor = 1; ledPulseUntil = millis() + LED_PULSE_MS; }
  else if (joyRight && !prevJoyRight) { pulseLedColor = 2; ledPulseUntil = millis() + LED_PULSE_MS; }
  else if (joyUp    && !prevJoyUp)    { pulseLedColor = 3; ledPulseUntil = millis() + LED_PULSE_MS; }
  else if (joyDown  && !prevJoyDown)  { pulseLedColor = 4; ledPulseUntil = millis() + LED_PULSE_MS; }

  prevJoyLeft = joyLeft; prevJoyRight = joyRight;
  prevJoyUp   = joyUp;   prevJoyDown  = joyDown;

  bool motorOn = (millis() < motorPulseUntil);
  digitalWrite(MOTOR_PIN, motorOn ? HIGH : LOW);

  uint8_t ledState = (millis() < ledPulseUntil) ? pulseLedColor : 0;

  if (ledState != lastLed) {
    if      (ledState == 1) strip.fill(strip.Color(255,   0,   0));  // שמאל – אדום
    else if (ledState == 2) strip.fill(strip.Color(  0,   0, 255));  // ימין  – כחול
    else if (ledState == 3) strip.fill(strip.Color(  0, 255,   0));  // למעלה – ירוק
    else if (ledState == 4) strip.fill(strip.Color(255, 255,   0));  // למטה  – צהוב
    else                    strip.clear();
    strip.show();
  }

  // 5. OLED
  drawStatus(currentTilt, joyX, joyY, btnLeft, btnRight, specialAttack, motorOn, ledState, batteryVoltage);

  // 6. שידור WebSocket
  static unsigned long lastSendTime       = 0;
  static unsigned long lastBatterySendTime = 0;

  if (millis() - lastSendTime >= 50) {

    if (btnLeft   != lastBtnLeft   || btnRight  != lastBtnRight  ||
        joyLeft   != lastJoyLeft   || joyRight  != lastJoyRight  ||
        joyUp     != lastJoyUp     || joyDown   != lastJoyDown   ||
        btnSelect != lastSelect    || specialAttack != lastAttack ||
        abs(currentTilt - lastTilt) > 150 ||
        motorOn   != lastMotor     || ledState  != lastLed       ||
        fabsf(batteryVoltage - lastBatteryVoltage) > 0.05f) {

      char jsonBuffer[250];
      snprintf(jsonBuffer, sizeof(jsonBuffer),
               "CTRL:{\"btnLeft\":%d,\"btnRight\":%d,"
               "\"joyLeft\":%d,\"joyRight\":%d,\"joyUp\":%d,\"joyDown\":%d,"
               "\"select\":%d,\"tilt\":%d,\"attack\":%d,"
               "\"motor\":%d,\"led\":%d,\"battery\":%.2f}",
               btnLeft, btnRight,
               joyLeft, joyRight, joyUp, joyDown,
               btnSelect, currentTilt, specialAttack,
               motorOn, ledState, batteryVoltage);

      wsServer.broadcastTXT(jsonBuffer);

      lastBtnLeft  = btnLeft;  lastBtnRight = btnRight;
      lastJoyLeft  = joyLeft;  lastJoyRight = joyRight;
      lastJoyUp    = joyUp;    lastJoyDown  = joyDown;
      lastSelect   = btnSelect;
      lastTilt     = currentTilt;
      lastAttack   = specialAttack;
      lastMotor    = motorOn;
      lastLed      = ledState;
      lastBatteryVoltage = batteryVoltage;
    }

    // שידור סוללה כל 5 שניות גם אם אין שינוי
    if (fabsf(batteryVoltage - lastBatteryVoltage) > 0.1f ||
        millis() - lastBatterySendTime >= 5000) {
      char batteryBuffer[60];
      snprintf(batteryBuffer, sizeof(batteryBuffer),
               "CTRL:{\"battery\":%.2f}", batteryVoltage);
      wsServer.broadcastTXT(batteryBuffer);
      lastBatterySendTime = millis();
    }

    lastSendTime = millis();
  }

  yield();
  delay(50);
}

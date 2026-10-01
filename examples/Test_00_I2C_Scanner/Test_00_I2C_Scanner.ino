/*
  Test 0 – סורק I2C + תצוגה על OLED
  סורק כתובות I2C על זוגות פינים (5,6) ו-(8,9)
  ומציג את התוצאות ישירות על מסך ה-OLED (בלי צורך ב-Serial Monitor).

  כתובות צפויות:
    OLED  SSD1306 → 0x3C
    MPU6050       → 0x68
*/
#include <Wire.h>
#include <U8g2lib.h>

// OLED 0.42" – SSD1306 72x40  (SDA=5, SCL=6)
U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// ─── עזר: הדפסת שורה על OLED ──────────────────────────
static byte oledLine = 0;  // שורה נוכחית (0=ראשונה)
const byte  LINE_H   = 10; // גובה שורה בפיקסלים

void oledPrint(const char* text) {
  if (oledLine == 0) display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, (oledLine + 1) * LINE_H, text);
  display.sendBuffer();
  oledLine++;
  if (oledLine > 3) { // 4 שורות מקסימום
    delay(3000);
    oledLine = 0;
    display.clearBuffer();
    display.sendBuffer();
  }
}

// ─── סריקת זוג פינים ───────────────────────────────────
void scanPins(int sda, int scl) {
  Wire.end();
  Wire.begin(sda, scl);
  delay(50);

  char header[20];
  snprintf(header, sizeof(header), "SDA%d SCL%d:", sda, scl);
  oledPrint(header);

  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      char msg[16];
      snprintf(msg, sizeof(msg), " 0x%02X", addr);
      oledPrint(msg);
      found++;
    }
  }
  if (found == 0) oledPrint(" (nothing)");
}

// ─── setup ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // אתחול ראשוני של OLED על IO5/IO6
  Wire.begin(5, 6);
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "I2C Scan...");
  display.sendBuffer();
  delay(800);
  oledLine = 0;

  // סריקה על שני זוגות הפינים
  scanPins(5, 6);
  delay(2000);
  scanPins(8, 9);
  delay(2000);

  // סיכום – בדיקה ספציפית ל-MPU6050
  Wire.end();
  Wire.begin(5, 6);
  delay(50);

  bool found68 = false, found69 = false;
  Wire.beginTransmission(0x68); if (Wire.endTransmission() == 0) found68 = true;
  Wire.beginTransmission(0x69); if (Wire.endTransmission() == 0) found69 = true;

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, found68 ? "MPU 0x68 OK!" : (found69 ? "MPU 0x69 (AD0=VCC)" : "MPU NOT FOUND!"));
  display.drawStr(0, 22, "OLED 0x3C OK");
  display.drawStr(0, 34, found68||found69 ? "I2C OK" : "Check wiring");
  display.sendBuffer();
}

void loop() {
  delay(5000);
}

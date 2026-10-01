//================= functionsOfOled =================
void initOled() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setTimeOut(20);
  display.begin();

  Wire.setClock(400000);

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 12, "OLED 0.42 OK");
  display.sendBuffer();
}

int mapY(byte y) {
  if (y < 10) return 10;
  if (y < 20) return 20;
  if (y < 30) return 30;
  return 38;
}

void functionsForPrintingAmessage(byte x, byte y, String str) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(x, mapY(y), str.c_str());
  display.sendBuffer();
}

void drawStatus(int tilt, int joyX, int joyY, bool left, bool right, bool attack, bool motor, uint8_t led, float vBat) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);

  // שורה 1: הטיה + מתח סוללה
  char line1[20];
  snprintf(line1, sizeof(line1), "T:%d B:%.1fV", tilt, vBat);
  display.drawStr(0, 9, line1);

  // שורה 2: ג'ויסטיק גולמי
  String line2 = "X:" + String(joyX) + " Y:" + String(joyY);
  display.drawStr(0, 19, line2.c_str());

  // שורה 3: כפתורים + מכה
  String line3 = "L:" + String(left) + " R:" + String(right) + " A:" + String(attack);
  display.drawStr(0, 29, line3.c_str());

  // שורה 4: מנוע + לד
  String ledTxt = led == 1 ? "RED" : (led == 2 ? "BLU" : (led == 3 ? "GRN" : (led == 4 ? "YLW" : "-")));
  String line4 = "M:" + String(motor ? "ON " : "OFF") + " L:" + ledTxt;
  display.drawStr(0, 39, line4.c_str());

  display.sendBuffer();
}
//===================================================

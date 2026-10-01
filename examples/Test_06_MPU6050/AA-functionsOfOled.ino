//================== AA-functionsOfOled ==================
void initOled() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setTimeOut(20);
  display.begin();
  Wire.setClock(400000);

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 12, "MPU6050 Test");
  display.sendBuffer();
  delay(800);
}

// מציג: תאוצה (ax, ay) + גירוסקופ (gz) + סטטוס
void drawImu(float ax, float ay, float gz, bool mpuOk) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);

  if (!mpuOk) {
    display.drawStr(0, 10, "MPU ERROR!");
    display.drawStr(0, 22, "0x68 not found");
    display.drawStr(0, 34, "AD0->GND?");
    display.sendBuffer();
    return;
  }

  char line1[20], line2[20], line3[20];
  snprintf(line1, sizeof(line1), "aX:%.1f", ax);
  snprintf(line2, sizeof(line2), "aY:%.1f", ay);
  snprintf(line3, sizeof(line3), "gZ:%.1f", gz);

  display.drawStr(0, 10, line1);
  display.drawStr(0, 22, line2);
  display.drawStr(0, 34, line3);
  display.sendBuffer();
}

//================== CC-functionsOfMpu6050 ==================
void initMpu6050() {
  // I2C כבר אותחל ב-initOled()
  if (!mpu.begin()) {
    // מציג שגיאה על OLED ונשאר תקוע
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 10, "MPU ERROR!");
    display.drawStr(0, 22, "0x68 missing");
    display.drawStr(0, 34, "AD0->GND?");
    display.sendBuffer();
    while (true) delay(1000);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // הצגת אישור על OLED
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "MPU6050 OK");
  display.drawStr(0, 22, "0x68 found");
  display.sendBuffer();
  delay(1200);
}

void readImu(float &ax, float &ay, float &gx, float &gy, float &gz) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  ax = a.acceleration.x;
  ay = a.acceleration.y;
  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;
}

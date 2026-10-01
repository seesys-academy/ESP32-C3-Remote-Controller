//================= functionsOfMpu6050 ===================
// קריאה ישירה מרגיסטרים (ללא ספרייה), החלקת הטיה על ציר X,
// וזיהוי "מכה" (jerk) על ציר Z.

#define MPU_ADDR 0x68  // AD0 → GND

float   smoothedTilt   = 0.0f;
const float  ALPHA          = 0.2f;
int16_t last_raw_Z     = 0;
const int    JERK_THRESHOLD = 7000;

void initMpu6050() {
  // Reset חומרתי ל-I2C לפני הכל
  Wire.end();
  delay(10);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setTimeOut(20);
  Wire.setClock(400000);
  delay(100);

  // בדיקה: האם MPU6050 מגיב על כתובת 0x68
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission(true);

  if (error == 0) {
    // MPU מגיב – שולחים פקודת התעוררות (יוצאים מ-Sleep Mode)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);  // PWR_MGMT_1
    Wire.write(0x00);  // Wake Up
    Wire.endTransmission(true);
    delay(50);
    // הצגה על OLED
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 10, "MPU6050 OK");
    display.drawStr(0, 22, "0x68 found");
    display.sendBuffer();
    delay(1500);
  } else {
    // מציג שגיאה על OLED
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 10, "MPU ERROR!");
    char errLine[20];
    snprintf(errLine, sizeof(errLine), "I2C err:%d", error);
    display.drawStr(0, 22, errLine);
    display.drawStr(0, 34, "AD0->GND?");
    display.sendBuffer();
    delay(3000);
  }
}

void readMpu6050(int &currentTilt, bool &specialAttack) {
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead < 50) return;  // קריאה כל 50ms
  lastSensorRead = millis();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H
  byte i2cError = Wire.endTransmission(false);

  if (i2cError == 0) {
    if (Wire.requestFrom(MPU_ADDR, 6, true) == 6) {
      int16_t AcX = Wire.read() << 8 | Wire.read();
      int16_t AcY = Wire.read() << 8 | Wire.read();
      int16_t AcZ = Wire.read() << 8 | Wire.read();

      smoothedTilt = (ALPHA * AcX) + ((1.0f - ALPHA) * smoothedTilt);
      currentTilt  = (int)smoothedTilt;

      int jerk_Z   = AcZ - last_raw_Z;
      last_raw_Z   = AcZ;
      specialAttack = (abs(jerk_Z) > JERK_THRESHOLD);
    }
  } else {
    // קו I2C נתקע – ריסט ואתחול מחדש
    static unsigned long lastResetTime = 0;
    if (millis() - lastResetTime > 2000) {
      Serial.println("I2C Bus Error – Resetting...");
      Wire.end();
      delay(10);
      Wire.begin(SDA_PIN, SCL_PIN);
      Wire.setTimeOut(20);
      Wire.setClock(400000);
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x6B);
      Wire.write(0x00);
      Wire.endTransmission(true);
      lastResetTime = millis();
    }
  }
}
//==========================================================

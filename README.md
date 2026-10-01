# ESP32-C3 Remote Controller — Quick Start

פרויקט שלט מבוסס ESP32-C3 (לוח CS-CTRL עם מסך OLED 0.42" מובנה) עם:
מסך OLED 0.42", MPU6050 (ג'יירו/אקסלרומטר), ג'ויסטיק אנלוגי, מנוע רטט,
4 לדים RGB (WS2812B), שני כפתורים (SW_LEFT / SW_RIGHT), ומד מתח סוללה.

---

## 1 — התקנת ספריות (Arduino IDE → כלים → נהל ספריות)

| ספרייה | שימוש |
|---|---|
| `U8g2` (by oliver) | מסך OLED |
| `Adafruit MPU6050` | חיישן תאוצה/ג'יירו |
| `Adafruit Unified Sensor` | תלות של Adafruit MPU6050 |
| `Adafruit NeoPixel` | לדים WS2812B |
| `WebSockets` (by Markus Sattler) | Test_07_WebDashboard בלבד |

**Board Manager:** חפש `esp32` מאת `Espressif Systems`.
לאחר ההתקנה בחר: **ESP32C3 Dev Module**.

---

## 2 — מיפוי פינים (לפי סכמה)

| רכיב | פונקציה | GPIO |
|---|---|---|
| OLED (מובנה בלוח) | SDA | IO5 |
| OLED (מובנה בלוח) | SCL | IO6 |
| MPU6050 (חיצוני, אותו באס I2C) | SDA | IO5 |
| MPU6050 (חיצוני, אותו באס I2C) | SCL | IO6 |
| MPU6050 | I2C address | 0x68 (AD0 → GND) |
| ג'ויסטיק | VRX (ציר X) | IO3 |
| ג'ויסטיק | VRY (ציר Y) | IO4 |
| ג'ויסטיק | SW_JOY (לחיצה, active LOW) | IO7 |
| כפתור שמאל | SW_LEFT (active LOW) | IO0 |
| כפתור ימין | SW_RIGHT (active LOW) | IO9 |
| מנוע רטט | MOTOR_PIN (דרך Q1 MOSFET, HIGH=דולק) | IO10 |
| לדים WS2812B × 4 | LEDS (data in) | IO2 |
| סוללה | T-Vbat (מחלק מתח) | IO1 |

---

## 3 — נקודות קריטיות בקוד

### ג'ויסטיק — חובה ADC_11db
ברירת מחדל של ESP32-C3 היא 0–1.1V. הג'ויסטיק מוציא 0–3.3V — חובה להגדיר:
```cpp
analogSetPinAttenuation(VRX_PIN, ADC_11db);  // IO3
analogSetPinAttenuation(VRY_PIN, ADC_11db);  // IO4
```
סף תנועה מומלץ: `< 500` ו-`> 3500` (ADC 12-bit, מרכז ~2048).

### כפתורים — INPUT_PULLUP
כל הכפתורים active LOW, חובה:
```cpp
pinMode(LEFT_PIN,  INPUT_PULLUP);  // IO0
pinMode(RIGHT_PIN, INPUT_PULLUP);  // IO9
pinMode(SW_PIN,    INPUT_PULLUP);  // IO7
```

### מתח סוללה — R12=30kΩ / R13=110kΩ
```
Vbat = (batteryRaw / 4095.0) × 3.3 × (30 + 110) / 110
```
- 4.2V (סוללה מלאה) → 3.3V על ADC (בטוח).

### MPU6050 — אתחול עם Wire reset
```cpp
#define MPU_ADDR 0x68   // AD0 → GND
Wire.end();
delay(10);
Wire.begin(SDA_PIN, SCL_PIN);  // IO5, IO6
Wire.setClock(400000);
```

---

## 4 — סדר בדיקת התרגילים

התרגילים ממוספרים מהקל לקשה (01→07). `Test_00` הוא כלי אבחון — הרץ אותו רק אם MPU6050 לא מגיב.

| תרגיל | מה נבדק |
|---|---|
| `Test_00_I2C_Scanner` | סורק I2C ומציג על OLED: מצפה 0x3C (OLED) + 0x68 (MPU) |
| `Test_01_OLED` | מסך בלבד — מונה מתחלף לאימות תצוגה |
| `Test_02_ButtonCounter` | SW_LEFT (IO0), SW_RIGHT (IO9) — מונה לחיצות |
| `Test_03_Joystick` | VRX/VRY על OLED, זיהוי כיוונים |
| `Test_04_VibrationMotor` | מנוע רטט IO10 — פולס בתנועת ג'ויסטיק |
| `Test_05_WS2812_LEDs` | 4 × WS2812B על IO2 — צבע לפי כיוון |
| `Test_06_MPU6050` | הטיה + ג'יירו — לדים לפי ציר X/Y |
| `Test_07_WebDashboard` | AP WiFi + WebSocket + דשבורד HTML בדפדפן |

---

## 5 — Test_07 WebDashboard — הפעלה

1. **העלה קבצי דף:** כלים → ESP32 Sketch Data Upload (מעלה את תיקיית `data/`)
2. **העלה קוד:** Ctrl+U כרגיל
3. **חבר WiFi:** `ESP32_Console_V2` · סיסמה: `87654321`
4. **פתח דפדפן:** `http://192.168.4.1`

---

## 6 — מבנה הפרויקט

```
ESP32C3_Remote_Controller - KIT-/
├── examples/
│   ├── Test_00_I2C_Scanner/
│   │   └── Test_00_I2C_Scanner.ino
│   ├── Test_01_OLED/
│   │   ├── Test_01_OLED.ino
│   │   └── AA-functionsOfOled.ino
│   ├── Test_02_ButtonCounter/
│   │   ├── Test_02_ButtonCounter.ino
│   │   ├── AA-functionsOfOled.ino
│   │   └── BB-functionsOfButtons.ino
│   ├── Test_03_Joystick/
│   │   ├── Test_03_Joystick.ino
│   │   ├── AA-functionsOfOled.ino
│   │   └── BB-functionsOfJoystick.ino
│   ├── Test_04_VibrationMotor/
│   │   ├── Test_04_VibrationMotor.ino
│   │   ├── AA-functionsOfOled.ino
│   │   ├── BB-functionsOfJoystick.ino
│   │   └── CC-functionsOfMotor.ino
│   ├── Test_05_WS2812_LEDs/
│   │   ├── Test_05_WS2812_LEDs.ino
│   │   ├── AA-functionsOfOled.ino
│   │   ├── BB-functionsOfJoystick.ino
│   │   ├── CC-functionsOfMotor.ino
│   │   └── DD-functionsOfLeds.ino
│   ├── Test_06_MPU6050/
│   │   ├── Test_06_MPU6050.ino
│   │   ├── AA-functionsOfOled.ino
│   │   ├── BB-functionsOfLeds.ino
│   │   └── CC-functionsOfMpu6050.ino
│   └── Test_07_WebDashboard/
│       ├── Test_07_WebDashboard.ino
│       ├── functionsOfOled.ino
│       ├── functionsOfMpu6050.ino
│       ├── functionsWebServer.ino
│       ├── functionsWebSockets.ino
│       ├── functionsWiFi.ino
│       └── data/
│           ├── index.html
│           ├── app.js
│           ├── style.css
│           └── board.jpg
└── ESP32C3_Remote_Controller/   ← הפרויקט המאוחד
    ├── ESP32C3_Remote_Controller.ino
    ├── Pins.h
    ├── OLED_Display.h / .cpp
    ├── IMU_MPU6050.h / .cpp
    ├── Joystick.h / .cpp
    ├── VibrationMotor.h / .cpp
    ├── RGB_LEDs.h / .cpp
    └── Buttons.h / .cpp
```

כל תרגיל בנוי מטאב ראשי (`<שם>.ino`) וטאבי עזר (`AA-/BB-/CC-/DD-functionsOfX.ino`) — כל הטאבים באותה תיקייה ייפתחו ב-Arduino IDE יחד.

---

## 7 — פתרון בעיות נפוצות

**MPU6050 לא נמצא**
הרץ `Test_00_I2C_Scanner`. אם מצא רק 0x3C: בדוק חיווט, ודא AD0→GND, הרץ שוב.

**ג'ויסטיק לא מגיב / ערכים נמוכים**
ודא שהוספת `analogSetPinAttenuation(..., ADC_11db)` — בלי זה טווח ADC מוגבל ל-0–1.1V.

**Serial Monitor לא עובד**
ESP32-C3 לעיתים צריך לחיצה על BOOT בזמן חיבור. לחלופין — כל המידע מוצג על ה-OLED.

**מסך OLED קפוא**
ה-OLED לא מוחק את עצמו אם I2C נתקע. ודא שהתוכן משתנה בזמן אמת; נתק חשמל בין בדיקות.

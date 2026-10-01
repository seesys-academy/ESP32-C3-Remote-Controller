//================== BB-functionsOfJoystick ==================
void initJoystick() {
  pinMode(PIN_JOY_SW_SEL, INPUT_PULLUP); // פול-אפ חיצוני קיים בחומרה (R1)
  analogSetPinAttenuation(PIN_JOY_VRX, ADC_11db);
  analogSetPinAttenuation(PIN_JOY_VRY, ADC_11db);
  analogReadResolution(8); 
}

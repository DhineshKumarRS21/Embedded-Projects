#define BLYNK_TEMPLATE_ID   "TMPL3hjU4vcys"
#define BLYNK_TEMPLATE_NAME "Real Time Infant Monitoring and Parental Alert"
#define BLYNK_AUTH_TOKEN    "y2T8DH2ySdjzaAknMIOP7p1JzeLh1vzB"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Wi-Fi Credentials
char ssid[] = "SHK";
char pass[] = "srihari22";

// LCD
LiquidCrystal_I2C lcd(0x27,16,2);

// DS18B20 Sensor
#define ONE_WIRE_BUS 32
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Other Sensors / Actuators
const int PIR_PIN = 33;
const int SOUND_DO = 26;
const int MOTOR_PIN = 25;   // Fan
const int BUZZER_PIN = 27;  // Heater

// Temperature thresholds
const float FAN_THRESHOLD = 32.0;
const float HEATER_THRESHOLD = 31.0;


float fLM35 = 36.5;


int fPIR = 0;
int fSound = 0;
unsigned long lastFUpdate = 0;
unsigned long lastBlinkUpdate = 0;
bool blinkState = false;

// Blynk Virtual Pins
#define VPIN_LM35     V0
#define VPIN_DS18B20  V1
#define VPIN_PIR      V2
#define VPIN_SOUND    V3
#define VPIN_FAN      V4
#define VPIN_HEATER   V5

// Fan & Heater state
bool fanState = false;
bool heaterState = false;

void setup() {
  Serial.begin(115200);

  // LCD
  Wire.begin(21,22);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Initializing...");

  // DS18B20
  sensors.begin();

  // Pins
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_DO, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Wi-Fi & Blynk
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);

  lcd.setCursor(0,1);
  lcd.print("LCD Ready!");
}

void loop() {
  Blynk.run();

  // Read DS18B20
  sensors.requestTemperatures();
  float tempDS18B20 = sensors.getTempCByIndex(0);
  if(tempDS18B20 == DEVICE_DISCONNECTED_C) tempDS18B20 = 0.0;

  
  float change = random(-2,3)/10.0;
  fLM35 += change;
  if(fLM35 < 36.0) fLM35 = 36.0;
  if(fLM35 > 37.5) fLM35 = 37.5;

  if (millis() - lastFUpdate > 2000) {
    lastFUpdate = millis();
    fPIR = (random(0, 10) < 3) ? 1 : 0;
    fSound = (random(0, 10) < 4) ? 1 : 0;
  }

  int motion = fPIR;
  int soundState = fSound;

  // Fan & Heater control
  if(tempDS18B20 > FAN_THRESHOLD){
    digitalWrite(MOTOR_PIN,HIGH);
    fanState = true;
    digitalWrite(BUZZER_PIN,LOW);
    heaterState = false;
  }
  else if(tempDS18B20 < HEATER_THRESHOLD){
    digitalWrite(MOTOR_PIN,LOW);
    fanState = false;
    digitalWrite(BUZZER_PIN,HIGH);
    heaterState = true;
  }
  else{
    digitalWrite(MOTOR_PIN,LOW);
    fanState = false;
    digitalWrite(BUZZER_PIN,LOW);
    heaterState = false;
  }

  lcd.setCursor(0,0);
  lcd.print("LM:");
  lcd.print(fLM35,1);
  lcd.print("C DS:");
  lcd.print(tempDS18B20,1);
  lcd.print("C  ");
  lcd.setCursor(0,1);
  lcd.print("PIR:");
  lcd.print(motion);
  lcd.print(" Snd:");
  lcd.print(soundState);
  lcd.print("   ");

  // 🔹 Blink LEDs every 500ms (0 or 1)
  if (millis() - lastBlinkUpdate > 500) {
    lastBlinkUpdate = millis();
    blinkState = !blinkState;

    Blynk.virtualWrite(VPIN_PIR, motion && blinkState ? 1 : 0);
    Blynk.virtualWrite(VPIN_SOUND, soundState && blinkState ? 1 : 0);
    Blynk.virtualWrite(VPIN_FAN, fanState && blinkState ? 1 : 0);
    Blynk.virtualWrite(VPIN_HEATER, heaterState && blinkState ? 1 : 0);
  }

  // Temperature readings
  if(WiFi.status() == WL_CONNECTED){
    Blynk.virtualWrite(VPIN_LM35, fLM35);
    Blynk.virtualWrite(VPIN_DS18B20, tempDS18B20);
  }

  // Serial debug
  Serial.print("DS:");
  Serial.print(tempDS18B20);
  Serial.print("C LM:");
  Serial.print(fLM35);
  Serial.print("C PIR:");
  Serial.print(motion);
  Serial.print(" Sound:");
  Serial.print(soundState);
  Serial.print(" Fan:");
  Serial.print(fanState);
  Serial.print(" Heater:");
  Serial.println(heaterState);

  delay(100);
}

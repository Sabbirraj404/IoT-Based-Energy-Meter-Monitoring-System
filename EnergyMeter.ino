#define BLYNK_TEMPLATE_ID "TMPL6zKoG3-Ho"
#define BLYNK_TEMPLATE_NAME "Smart Energy Meter"
#define BLYNK_AUTH_TOKEN "YDGPH7He86V9tSmVo93ciAukei6_zUNp"

#define BLYNK_PRINT Serial   // Debug messages to Serial Monitor

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD config (20x4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// WiFi credentials
char ssid[] = "Sabbir";
char pass[] = "19122918";

// PZEM UART pin configuration
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17

// Initialize PZEM with Serial2
PZEM004Tv30 pzem(&Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

// Electricity rate in BDT per kWh
const float BDT_RATE = 4.50;

// Blynk timer
BlynkTimer timer;

void sendData() {
  float voltage   = pzem.voltage();
  float current   = pzem.current();
  float energy    = pzem.energy(); // in kWh
  float frequency = pzem.frequency();

  float cost = (!isnan(energy)) ? energy * BDT_RATE : 0;

  // --- Always update LCD (only sensor readings) ---
  lcd.setCursor(0, 0); lcd.print("V:"); lcd.print(voltage, 2); lcd.print("V   ");
  lcd.setCursor(0, 1); lcd.print("I:"); lcd.print(current, 3); lcd.print("A   ");
  lcd.setCursor(0, 2); lcd.print("Unit:"); lcd.print(energy, 3); lcd.print("kWh   ");
  lcd.setCursor(0, 3); lcd.print("F:"); lcd.print(frequency, 2); lcd.print("Hz   ");
  lcd.setCursor(10, 3); lcd.print("৳"); lcd.print(cost, 2); lcd.print("   ");

  // --- Send to Blynk only if connected ---
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    if (!isnan(voltage))   Blynk.virtualWrite(V0, voltage);
    if (!isnan(current))   Blynk.virtualWrite(V1, current);
    if (!isnan(energy))    Blynk.virtualWrite(V3, energy);
    if (!isnan(frequency)) Blynk.virtualWrite(V4, frequency);
    Blynk.virtualWrite(V6, cost);
  }
}

void setup() {
  Serial.begin(115200);

  // LCD init
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Start WiFi + Blynk (non-blocking)
  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Start PZEM
  Serial2.begin(9600, SERIAL_8N1, PZEM_RX_PIN, PZEM_TX_PIN);

  // Timer for sending data
  timer.setInterval(2000L, sendData);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run(); // run only if WiFi OK
  }
  timer.run();   // LCD + sensor updates always
}

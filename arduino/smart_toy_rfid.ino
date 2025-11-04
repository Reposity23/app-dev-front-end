#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClientSecure.h>

// --- Hardware & Network Configuration ---
#define SS_PIN 21
#define RST_PIN 22

// --- WiFi Credentials ---
const char* ssid = "john_m";
const char* password = "123456789";

// --- Cloud Server Configuration ---
const char* serverName = "912dcb36-25f8-4bc1-b60e-c7b4e4ca564c-00-1k5bfypkvk6lm.pike.replit.dev";

// --- LED Pins ---
const int LED_TOY_GUNS = 25;
const int LED_ACTION_FIGURES = 27;
const int LED_DOLLS = 26;
const int LED_PUZZLES = 33;

// --- Buzzer ---
const int BUZZER_PIN = 14;

// --- RFID ---
MFRC522 rfid(SS_PIN, RST_PIN);

// --- LCD (I2C) ---
#define SDA_PIN 17
#define SCL_PIN 5
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Card Mapping ---
struct PersonMapping {
  const char* physicalUid;
  const char* personName;
};

PersonMapping personMappings[] = {
  {"A9 6C 6A 05", "John Marwin"},
  {"01 02 03 04", "Jannalyn"},
  {"YOUR_UID_HERE_MARL", "Marl Prince"},
  {"YOUR_UID_HERE_RENZ", "Renz"}
};
const int numMappings = sizeof(personMappings) / sizeof(PersonMapping);

// --- Prototypes ---
void setupWiFi();
void setupHardware();
void turnOffAllLEDs();
void indicateSystemReady();
void indicateError();
String getScannedUID();
String getPersonFromPhysicalUID(String physicalUid);
void handleScan(String personName);
void executeLedAction(const char* action, const char* category);
int getLedPinForCategory(const char* category);
void blinkLed(int pin, int times, int duration);
void buzzBuzzer(int duration_ms);
void resetRFID();

// ====================================================
// SETUP
// ====================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n[SETUP] RFID Cloud System Initializing...");
  setupHardware();
  setupWiFi();
  Serial.println("[SETUP] Ready.");
  indicateSystemReady();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
}

// ====================================================
// LOOP
// ====================================================
unsigned long lastScanTime = 0;
const unsigned long idleTimeout = 10000; // 10 seconds

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Disconnected. Reconnecting...");
    indicateError();
    setupWiFi();
  }

  if (millis() - lastScanTime > idleTimeout) {
    resetRFID();
    lastScanTime = millis();
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    lastScanTime = millis();

    String physicalUid = getScannedUID();
    String personName = getPersonFromPhysicalUID(physicalUid);

    if (personName.length() > 0) {
      buzzBuzzer(50);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scanned:");
      lcd.setCursor(0, 1);
      lcd.print(personName);
      handleScan(personName);
    } else {
      indicateError();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unknown Card");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(300);
  }
}

// ====================================================
// FUNCTIONS
// ====================================================

void buzzBuzzer(int duration_ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration_ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void handleScan(String personName) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();

  String url = String("https://") + serverName + "/api/process-next";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<128> doc;
  doc["person_name"] = personName;
  String payload;
  serializeJson(doc, payload);

  Serial.print("[HTTPS] POST → "); Serial.println(url);
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String responsePayload = http.getString();
    Serial.print("[HTTPS] Response: "); Serial.println(responsePayload);
    StaticJsonDocument<256> responseDoc;
    deserializeJson(responseDoc, responsePayload);
    executeLedAction(responseDoc["action"], responseDoc["led"]);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(responseDoc["led"].as<const char*>());
    lcd.setCursor(0, 1);
    lcd.print(responseDoc["action"].as<const char*>());
  } else {
    Serial.print("[HTTPS] Failed: "); Serial.println(http.errorToString(httpCode));
    indicateError();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Server Error");
  }

  http.end();
}

void executeLedAction(const char* action, const char* category) {
  int targetLed = getLedPinForCategory(category);
  if (strcmp(action, "processing_success") == 0) {
    blinkLed(targetLed, 3, 200);
  } else if (strcmp(action, "no_pending_orders") == 0) {
    indicateSystemReady();
  } else {
    indicateError();
  }
}

void setupHardware() {
  pinMode(LED_TOY_GUNS, OUTPUT);
  pinMode(LED_ACTION_FIGURES, OUTPUT);
  pinMode(LED_DOLLS, OUTPUT);
  pinMode(LED_PUZZLES, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  turnOffAllLEDs();

  // --- RFID ---
  SPI.begin();
  rfid.PCD_Init();

  // --- LCD ---
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
}

void setupWiFi() {
  Serial.print("[WIFI] Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] Connection Failed.");
  }
}

void resetRFID() {
  Serial.println("[RFID] Resetting reader...");
  SPI.end();
  delay(10);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("[RFID] Reader reinitialized.");
}

String getScannedUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (i > 0) uid += " ";
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

String getPersonFromPhysicalUID(String physicalUid) {
  for (int i = 0; i < numMappings; i++) {
    if (physicalUid.equals(personMappings[i].physicalUid)) {
      return String(personMappings[i].personName);
    }
  }
  return "";
}

int getLedPinForCategory(const char* category) {
  if (strcmp(category, "Toy Guns") == 0) return LED_TOY_GUNS;
  if (strcmp(category, "Action Figures") == 0) return LED_ACTION_FIGURES;
  if (strcmp(category, "Dolls") == 0) return LED_DOLLS;
  if (strcmp(category, "Puzzles") == 0) return LED_PUZZLES;
  return -1;
}

void turnOffAllLEDs() {
  digitalWrite(LED_TOY_GUNS, LOW);
  digitalWrite(LED_ACTION_FIGURES, LOW);
  digitalWrite(LED_DOLLS, LOW);
  digitalWrite(LED_PUZZLES, LOW);
}

void blinkLed(int pin, int times, int duration) {
  if (pin == -1) return;
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
  }
}

void indicateSystemReady() {
  turnOffAllLEDs();
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_TOY_GUNS, HIGH);
    digitalWrite(LED_ACTION_FIGURES, HIGH);
    digitalWrite(LED_DOLLS, HIGH);
    digitalWrite(LED_PUZZLES, HIGH);
    delay(100);
    turnOffAllLEDs();
    delay(100);
  }
}

void indicateError() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_TOY_GUNS, HIGH);
    digitalWrite(LED_ACTION_FIGURES, HIGH);
    digitalWrite(LED_DOLLS, HIGH);
    digitalWrite(LED_PUZZLES, HIGH);
    delay(80);
    turnOffAllLEDs();
    delay(80);
  }
}

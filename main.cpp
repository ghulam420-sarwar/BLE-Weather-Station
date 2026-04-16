/**
 * BLE Weather Station
 * -------------------
 * Target MCU : ESP32
 * Sensors    : BME280 (I2C — temp, humidity, pressure)
 * Display    : SSD1306 128×32 OLED (I2C)
 * Wireless   : Bluetooth Low Energy (BLE) GATT server
 * Author     : Ghulam Sarwar
 *
 * Exposes a custom GATT service with three readable/notify characteristics:
 *   - Temperature    (float, °C,  UUID ends ...0001)
 *   - Humidity       (float, %,   UUID ends ...0002)
 *   - Pressure       (float, hPa, UUID ends ...0003)
 *
 * Use nRF Connect (Android/iOS) or the companion Python script to read values.
 * The ESP32 enters light-sleep between samples to save battery.
 *
 * MIT License
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ── BLE UUIDs ────────────────────────────────────────────────────────────────
#define SVC_UUID   "12345678-1234-1234-1234-123456780000"
#define CHAR_TEMP  "12345678-1234-1234-1234-123456780001"
#define CHAR_HUM   "12345678-1234-1234-1234-123456780002"
#define CHAR_PRES  "12345678-1234-1234-1234-123456780003"

// ── pins ──────────────────────────────────────────────────────────────────────
#define SDA_PIN    21
#define SCL_PIN    22
#define BTN_PIN     0   // BOOT button cycles display pages

Adafruit_BME280   bme;
Adafruit_SSD1306  oled(128, 32, &Wire, -1);

BLECharacteristic* cTemp;
BLECharacteristic* cHum;
BLECharacteristic* cPres;
bool bleConnected = false;
uint8_t displayPage = 0;

// ── BLE server callbacks ───────────────────────────────────────────────────────
class ServerCb : public BLEServerCallbacks {
    void onConnect(BLEServer*)    { bleConnected = true;  Serial.println("BLE client connected"); }
    void onDisconnect(BLEServer* s) {
        bleConnected = false;
        BLEDevice::startAdvertising();
        Serial.println("BLE client disconnected — advertising");
    }
};

void setFloatChar(BLECharacteristic* c, float v) {
    uint8_t buf[4];
    memcpy(buf, &v, 4);
    c->setValue(buf, 4);
    if (bleConnected) c->notify();
}

void drawOled(float t, float h, float p, uint8_t page) {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);

    switch (page % 3) {
        case 0:
            oled.println(F("Temperature"));
            oled.setTextSize(2);
            oled.printf("%.1f C", t);
            break;
        case 1:
            oled.println(F("Humidity"));
            oled.setTextSize(2);
            oled.printf("%.1f %%", h);
            break;
        case 2:
            oled.println(F("Pressure"));
            oled.setTextSize(2);
            oled.printf("%.0f hPa", p);
            break;
    }

    oled.setTextSize(1);
    oled.setCursor(100, 24);
    oled.print(bleConnected ? F("BLE") : F("---"));
    oled.display();
}

void setup() {
    Serial.begin(115200);
    pinMode(BTN_PIN, INPUT_PULLUP);
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!bme.begin(0x76)) {
        Serial.println("BME280 fail");
        while (1) delay(1000);
    }
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::FILTER_X4,
                    Adafruit_BME280::STANDBY_MS_500);

    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    oled.clearDisplay(); oled.display();

    // BLE setup
    BLEDevice::init("Weather Station");
    BLEServer* srv = BLEDevice::createServer();
    srv->setCallbacks(new ServerCb());
    BLEService* svc = srv->createService(SVC_UUID);

    auto mkChar = [&](const char* uuid) -> BLECharacteristic* {
        auto* c = svc->createCharacteristic(uuid,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
        c->addDescriptor(new BLE2902());
        return c;
    };
    cTemp = mkChar(CHAR_TEMP);
    cHum  = mkChar(CHAR_HUM);
    cPres = mkChar(CHAR_PRES);

    svc->start();
    BLEDevice::startAdvertising();
    Serial.println("BLE advertising as 'Weather Station'");
}

void loop() {
    static uint32_t tNext = 0;
    static uint32_t tBtn  = 0;

    // Button debounce — cycle display page
    if (!digitalRead(BTN_PIN) && millis() - tBtn > 300) {
        tBtn = millis();
        displayPage++;
    }

    if (millis() < tNext) return;
    tNext = millis() + 2000;

    float t = bme.readTemperature();
    float h = bme.readHumidity();
    float p = bme.readPressure() / 100.0f;

    setFloatChar(cTemp, t);
    setFloatChar(cHum,  h);
    setFloatChar(cPres, p);
    drawOled(t, h, p, displayPage);

    Serial.printf("T=%.2f H=%.2f P=%.2f BLE=%s\n", t, h, p,
                  bleConnected ? "conn" : "adv");
}

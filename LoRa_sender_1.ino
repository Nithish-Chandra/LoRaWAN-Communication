// LoRa ESP32 Sender
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <U8g2lib.h>
#include <LoRa.h>

// LoRa settings
#define BAND 433E6         // LoRa frequency
#define SS_PIN 18          // LoRa chip select pin
#define RST_PIN 14         // LoRa reset pin
#define DIO0_PIN 26        // LoRa DIO0 pin

// BMP280 settings
#define BMP_SDA 4          // BMP280 SDA pin
#define BMP_SCL 15         // BMP280 SCL pin

// OLED settings
#define OLED_CLOCK 15      // OLED SCL pin
#define OLED_DATA 4        // OLED SDA pin
#define OLED_RESET 16      // OLED reset pin

// Objects for BMP280 and OLED
Adafruit_BMP280 bmp;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RESET, OLED_CLOCK, OLED_DATA);

bool bmpInitialized = false;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Initialize OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 15, "Initializing...");
  u8g2.sendBuffer();

  // Initialize LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa initialization failed!");
    displayError("LoRa Init Failed!");
    while (1);
  }
  Serial.println("LoRa initialized!");

  // Initialize BMP280
  Wire.begin(BMP_SDA, BMP_SCL);
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 initialization failed!");
    displayError("BMP280 Init Failed!");
    while (1);
  }
  bmpInitialized = true;
  Serial.println("BMP280 initialized!");

  // Display startup success
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "LoRa Sender Ready");
  u8g2.sendBuffer();
}

void loop() {
  if (!bmpInitialized) {
    displayError("Sensor Not Ready!");
    delay(1000);
    return;
  }

  // Read temperature and pressure
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0; // Convert Pa to hPa

  // Log data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C\tPressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // Display data on OLED
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Temperature: ");
  u8g2.print(temperature);
  u8g2.print(" C");
  u8g2.setCursor(0, 35);
  u8g2.print("Pressure: ");
  u8g2.print(pressure);
  u8g2.print(" hPa");
  u8g2.sendBuffer();

  // Prepare data packet
  String dataPacket = "Temperature: " + String(temperature) + " C, Pressure: " + String(pressure) + " hPa";

  // Send data over LoRa
  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  Serial.println("Data sent over LoRa!");
  delay(5000); // Wait 5 seconds before the next transmission
}

void displayError(const char *message) {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print(message);
  u8g2.sendBuffer();
  Serial.println(message);
}

// LoRa ESP32 Sender 2
#include <Wire.h>
#include <LoRa.h>
#include <DHT.h>
#include <U8g2lib.h>

// DHT settings
#define DHT_PIN 4         // DHT sensor data pin
#define DHT_TYPE DHT22    // DHT22 sensor type

// LoRa settings
#define BAND 433E6        // LoRa frequency
#define SS_PIN 18         // LoRa chip select pin
#define RST_PIN 14        // LoRa reset pin
#define DIO0_PIN 26       // LoRa DIO0 pin

// OLED settings
#define OLED_CLOCK 15     // OLED SCL pin
#define OLED_DATA 4       // OLED SDA pin
#define OLED_RESET 16     // OLED reset pin

// Create DHT and OLED objects
DHT dht(DHT_PIN, DHT_TYPE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RESET, OLED_CLOCK, OLED_DATA);

bool dhtInitialized = false;

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

  // Initialize DHT sensor
  dht.begin();
  dhtInitialized = true;
  Serial.println("DHT sensor initialized!");

  // Display startup success
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "LoRa Sender Ready");
  u8g2.sendBuffer();
}

void loop() {
  if (!dhtInitialized) {
    displayError("DHT Not Ready!");
    delay(1000);
    return;
  }

  // Read humidity
  float humidity = dht.readHumidity();

  // Check if the reading is valid
  if (isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    displayError("DHT Read Failed!");
    delay(1000);
    return;
  }

  // Log data to Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Display data on OLED
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Humidity: ");
  u8g2.print(humidity);
  u8g2.print(" %");
  u8g2.sendBuffer();

  // Prepare data packet
  String dataPacket = "Humidity: " + String(humidity) + " %";

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

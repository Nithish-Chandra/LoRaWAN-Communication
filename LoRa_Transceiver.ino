#include "thingProperties.h"
#include <WiFi.h>
#include <LoRa.h>
#include <HTTPClient.h>

// LoRa settings
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 433E6 // LoRa frequency

// WiFi credentials
const char* ssid = "your_network_ssid";
const char* password = "your_network_password";

// PHP script URL
const char* phpScriptUrl = "http://192.168.241.124/bmp280_project/testdata_final.php";

// Variables to store received values
String receivedTemperature;
String receivedPressure;
String receivedHumidity;
int receivedRSSI;

String LoRaData;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial);

  delay(1500); // Wait for Serial Monitor

  // Initialize Arduino IoT Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Set debug message level
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Initialize SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);

  // Setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized!");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
}

void loop() {
  ArduinoCloud.update();

  // Try to parse LoRa packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    Serial.println("Received packet:");

    // Read the LoRa packet
    LoRaData = LoRa.readString();
    Serial.println(LoRaData);

    // Extract values
    receivedTemperature = getValue(LoRaData, "Temperature:", " %");
    receivedPressure = getValue(LoRaData, "Pressure:", " hPa");
    receivedHumidity = getValue(LoRaData, "Humidity:", " %");

    // Check if all parameters are received
    if (!receivedTemperature.isEmpty() && !receivedPressure.isEmpty() && !receivedHumidity.isEmpty()) {
      // Print received values
      Serial.println("Temperature: " + receivedTemperature);
      Serial.println("Pressure: " + receivedPressure);
      Serial.println("Humidity: " + receivedHumidity);

      // Print RSSI of the packet
      receivedRSSI = LoRa.packetRssi();
      Serial.println("RSSI: " + String(receivedRSSI));

      // Update Arduino IoT Cloud variables
      temperature = receivedTemperature.toFloat();
      pressure = receivedPressure.toFloat();
      humidity = receivedHumidity.toFloat();

      // Send values to the database
      sendDataToDatabase(receivedTemperature, receivedPressure, receivedHumidity, receivedRSSI);

      // Reset variables
      receivedTemperature = "";
      receivedPressure = "";
      receivedHumidity = "";
    }
  }
}

// Function to extract a value between two delimiters
String getValue(String data, String startDelimiter, String endDelimiter) {
  int startIndex = data.indexOf(startDelimiter) + startDelimiter.length();
  int endIndex = data.indexOf(endDelimiter, startIndex);
  return data.substring(startIndex, endIndex);
}

// Function to send data to the PHP script and insert into the database
void sendDataToDatabase(String temperature, String pressure, String humidity, int rssi) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(phpScriptUrl);

    // Set content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Build the data to send in the request body
    String requestBody = "temperature=" + temperature + "&pressure=" + pressure + "&humidity=" + humidity + "&rssi=" + String(rssi);

    // Send POST request
    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Data sent successfully");
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error sending data");
      Serial.println("HTTP Response code: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

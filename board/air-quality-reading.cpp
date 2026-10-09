#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>

// 1. CONFIGURATION: Wi-Fi, Server, and Custom Pins
const char *ssid = "InSpace - IoT"; // Change to your Wi-Fi name
const char *password = "";          // Change to your Wi-Fi password
const char *serverUrl = "";         // Your endpoint

#define I2C_SDA 5 // Your custom SDA pin
#define I2C_SCL 4 // Your custom SCL pin

// 2. TIMING VARIABLES (Non-blocking 15-second timer)
unsigned long previousMillis = 0;
const long interval = 15000; // 15 seconds in milliseconds

Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);

  // Initialize I2C with your custom pins
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize BME280
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println(F("Error: BME280 sensor not found! Check pins and wiring."));
    while (1)
      ;
  }
  Serial.println(F("BME280 connected."));

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi network!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if 15 seconds have passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check Wi-Fi connection status before attempting to send data
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // Read sensor parameters
      float temperature = bme.readTemperature();
      float humidity = bme.readHumidity();
      float pressure = bme.readPressure() / 100.0F;

      // Establish connection to the target URL
      http.begin(serverUrl);

      // Specify content-type header for JSON payload
      http.addHeader("Content-Type", "application/json");

      // Construct JSON payload string manually to save memory
      String jsonPayload = "{\"temperature\":" + String(temperature, 2) +
                           ",\"humidity\":" + String(humidity, 2) +
                           ",\"pressure\":" + String(pressure, 2) + "}";

      Serial.print("Sending POST request: ");
      Serial.println(jsonPayload);

      // Send the actual HTTP POST request
      int httpResponseCode = http.POST(jsonPayload);

      // Evaluate the server return code
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Server Response: ");
        Serial.println(response);
      } else {
        Serial.print("Error code on sending POST: ");
        Serial.println(httpResponseCode);
      }

      // Always free up resources when finished
      http.end();
    } else {
      Serial.println("Wi-Fi Disconnected. Reconnecting...");
      WiFi.begin(ssid, password);
    }
    Serial.println("------------------------------------");
  }
}

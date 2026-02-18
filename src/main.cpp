#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <ESP32Encoder.h>

#include "config.h"
#include "secrets.h"
#include "types.h"

#include "hue.h"
#include "syncbox.h"
#include "display.h"
#include "input.h"

// Globals
Light lights[10];
int numLights = 0;
int currentLightIndex = 0;

Room rooms[10];
int numRooms = 0;

Mode currentMode = BRIGHTNESS;

bool syncActive = false;
String tvStripRid = "";

ESP32Encoder encoder;
WiFiClientSecure secureClient;
HTTPClient http;

Adafruit_SSD1351 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

void connectWifi(const char *ssid, const char *password, Adafruit_SSD1351 &display);

void setup()
{
  Serial.begin(115200);

  initDisplay(display);
  secureClient.setInsecure();

  setupButtons();
  setupEncoder(encoder);

  connectWifi(WIFI_SSID, WIFI_PASSWORD, display);

  showMessage(display, "Fetching rooms...");
  fetchRooms(http, secureClient, rooms, numRooms);

  showMessage(display, "Fetching lights...");
  fetchLights(http, secureClient, lights, numLights, rooms, numRooms, tvStripRid);

  syncActive = fetchSyncState(http, secureClient);

  updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid);

  Serial.println("Ready!");
}

void loop()
{
  handleInputs(
      display,
      encoder,
      http,
      secureClient,
      lights,
      numLights,
      currentLightIndex,
      currentMode,
      syncActive,
      tvStripRid);
}

void connectWifi(const char *ssid, const char *password, Adafruit_SSD1351 &display)
{
  showMessage(display, "Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nWiFi connection FAILED!");
    showMessage(display, "WiFi connection\nFAILED!");
    while (1)
      ; // Stop here
  }
  Serial.println("\nWiFi connected!");
  secureClient.setInsecure();
}
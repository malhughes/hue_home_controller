#pragma once

#include <Arduino.h>
#include <ESP32Encoder.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <Adafruit_SSD1351.h>

#include "types.h"

// Setup
void setupButtons();
void setupEncoder(ESP32Encoder &encoder);

// Loop handler
void handleInputs(
    Adafruit_SSD1351 &display,
    ESP32Encoder &encoder,
    HTTPClient &http,
    WiFiClientSecure &client,
    Light lights[],
    int numLights,
    int &currentLightIndex,
    Mode &currentMode,
    bool &syncActive,
    String &tvStripRid);

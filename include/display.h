#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

#include "types.h"
#include "config.h"

// UI update function
void updateDisplay(
    Adafruit_SSD1351 &display,
    Light lights[],
    int numLights,
    int currentLightIndex,
    Mode currentMode,
    bool syncActive,
    String tvStripRid,
    int pendingValue = -1);

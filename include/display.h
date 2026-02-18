#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

#include "types.h"
#include "config.h"

// Canvas for double buffering (add this)
extern GFXcanvas16 canvas;

// Initialize display and canvas
void initDisplay(Adafruit_SSD1351 &display);

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

// Helper for showing messages during setup
void showMessage(Adafruit_SSD1351 &display, const char *message);

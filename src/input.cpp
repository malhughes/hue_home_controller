#include "input.h"

#include "config.h"
#include "display.h"
#include "hue.h"
#include "syncbox.h"

static bool lastSyncButtonState = HIGH;
static bool lastCycleButtonState = HIGH;
static bool lastEncoderButtonState = HIGH;

static unsigned long encoderPressStart = 0;
static bool isLongPress = false;

static unsigned long lastEncoderChange = 0;
static int pendingValue = -1;
static bool needsUpdate = false;

void setupButtons()
{
  pinMode(SYNC_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CYCLE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
}

void setupEncoder(ESP32Encoder &encoder)
{
  encoder.attachFullQuad(17, 16);
  encoder.setCount(50);
}

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
    String &tvStripRid)
{
  // --- Sync button ---
  bool currentSyncState = digitalRead(SYNC_BUTTON_PIN);
  if (currentSyncState == LOW && lastSyncButtonState == HIGH)
  {
    toggleSync(http, client, syncActive, lights, numLights, tvStripRid);

    showMessage(display, syncActive ? "SYNC ON" : "SYNC OFF");
    delay(1000);

    updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid, pendingValue);

    delay(50);
  }
  lastSyncButtonState = currentSyncState;

  // --- Cycle button ---
  bool currentCycleState = digitalRead(CYCLE_BUTTON_PIN);
  if (currentCycleState == LOW && lastCycleButtonState == HIGH)
  {
    currentLightIndex = (currentLightIndex + 1) % numLights;
    encoder.setCount((int)lights[currentLightIndex].brightness);

    updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid, pendingValue);

    delay(50);
  }
  lastCycleButtonState = currentCycleState;

  // --- Encoder button ---
  bool currentEncoderState = digitalRead(ENCODER_BUTTON_PIN);

  if (currentEncoderState == LOW && lastEncoderButtonState == HIGH)
  {
    encoderPressStart = millis();
    isLongPress = false;
  }

  if (currentEncoderState == LOW && millis() - encoderPressStart > 1000 && !isLongPress)
  {
    lights[currentLightIndex].isOn = !lights[currentLightIndex].isOn;

    toggleLight(http, client, lights[currentLightIndex].rid, lights[currentLightIndex].isOn);

    updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid, pendingValue);

    isLongPress = true;
    Serial.println("Long press - toggled light");
  }

  if (currentEncoderState == HIGH && lastEncoderButtonState == LOW)
  {
    if (!isLongPress)
    {
      if (currentMode == BRIGHTNESS)
        currentMode = COLOR;
      else if (currentMode == COLOR)
        currentMode = TEMPERATURE;
      else
        currentMode = BRIGHTNESS;

      updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid, pendingValue);

      Serial.println("Mode switched");
    }
  }

  lastEncoderButtonState = currentEncoderState;

  // --- Encoder rotation ---
  static int lastCount = 50;
  int currentCount = encoder.getCount();

  bool isTVStripWithSync = (lights[currentLightIndex].rid == tvStripRid && syncActive);

  if (!isTVStripWithSync && abs(currentCount - lastCount) >= 2)
  {
    currentCount = constrain(currentCount, 0, 100);
    encoder.setCount(currentCount);

    if (currentMode == BRIGHTNESS)
    {
      lights[currentLightIndex].brightness = currentCount;
      setBrightness(http, client, lights[currentLightIndex].rid, (float)currentCount);

      updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid, currentCount);
    }
    else
    {
      pendingValue = currentCount;
      needsUpdate = true;
      lastEncoderChange = millis();

      updateDisplay(display, lights, numLights, currentLightIndex, currentMode, syncActive, tvStripRid, pendingValue);
    }

    lastCount = currentCount;
  }

  // --- Send delayed updates ---
  if (!isTVStripWithSync && needsUpdate && millis() - lastEncoderChange > 100)
  {
    if (currentMode == COLOR)
    {
      setColor(http, client, lights[currentLightIndex].rid, (float)pendingValue);
    }
    else if (currentMode == TEMPERATURE)
    {
      setColorTemperature(http, client, lights[currentLightIndex].rid, (float)pendingValue);
    }

    needsUpdate = false;
  }

  delay(10);
}

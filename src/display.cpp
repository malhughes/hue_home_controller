#include "display.h"

// Create canvas for double buffering
GFXcanvas16 canvas(128, 128);

void initDisplay(Adafruit_SSD1351 &display)
{
  display.begin();
  canvas.fillScreen(COLOR_BLACK);
  display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
}

void showMessage(Adafruit_SSD1351 &display, const char *message)
{
  canvas.fillScreen(COLOR_BLACK);
  canvas.setCursor(0, 0);
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);
  canvas.println(message);
  display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
}

void updateDisplay(
    Adafruit_SSD1351 &display,
    Light lights[],
    int numLights,
    int currentLightIndex,
    Mode currentMode,
    bool syncActive,
    String tvStripRid,
    int pendingValue)
{
  // Draw to canvas instead of display
  canvas.fillScreen(COLOR_BLACK);
  canvas.setCursor(0, 0);
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);

  if (numLights <= 0)
  {
    canvas.println("No lights found!");
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
    return;
  }

  canvas.print("(");
  canvas.print(lights[currentLightIndex].room);
  canvas.print(") ");
  canvas.println("");
  canvas.println(lights[currentLightIndex].name);
  canvas.println("");

  bool isTVStripWithSync = (lights[currentLightIndex].rid == tvStripRid && syncActive);

  if (isTVStripWithSync)
  {
    canvas.println("SYNC ACTIVE");
    canvas.println("");
    canvas.println("Press SYNC button");
    canvas.println("to control light");
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
    return;
  }

  canvas.println(lights[currentLightIndex].isOn ? "ON" : "OFF");
  canvas.println("");

  if (!lights[currentLightIndex].isOn)
  {
    canvas.println("Long press dial");
    canvas.println("to turn on");
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
    return;
  }

  if (currentMode == BRIGHTNESS)
  {
    canvas.println("MODE: Brightness");
    canvas.print("Value: ");
    canvas.print((int)lights[currentLightIndex].brightness);
    canvas.println("%");
  }
  else if (currentMode == COLOR)
  {
    canvas.println("MODE: Color");
    canvas.print("Hue: ");
    canvas.print((int)(pendingValue * 3.6));
    canvas.println(" deg");
  }
  else
  {
    canvas.println("MODE: Temperature");
    int kelvin = map(pendingValue, 0, 100, 2000, 6500);
    canvas.print(kelvin);
    canvas.println("K");
  }

  // Push entire canvas to display in one operation
  display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
}
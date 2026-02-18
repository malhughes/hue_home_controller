#include "display.h"

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
  display.fillScreen(COLOR_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(COLOR_WHITE, COLOR_BLACK);

  if (numLights <= 0)
  {
    display.println("No lights found!");
    return;
  }

  display.print("(");
  display.print(lights[currentLightIndex].room);
  display.print(") ");
  display.println("");
  display.println(lights[currentLightIndex].name);
  display.println("");

  bool isTVStripWithSync = (lights[currentLightIndex].rid == tvStripRid && syncActive);

  if (isTVStripWithSync)
  {
    display.println("SYNC ACTIVE");
    display.println("");
    display.println("Press SYNC button");
    display.println("to control light");
    return;
  }

  display.println(lights[currentLightIndex].isOn ? "ON" : "OFF");
  display.println("");

  if (!lights[currentLightIndex].isOn)
  {
    display.println("Long press dial");
    display.println("to turn on");
    return;
  }

  if (currentMode == BRIGHTNESS)
  {
    display.println("MODE: Brightness");
    display.print("Value: ");
    display.print((int)lights[currentLightIndex].brightness);
    display.println("%");
  }
  else if (currentMode == COLOR)
  {
    display.println("MODE: Color");
    display.print("Hue: ");
    display.print((int)(pendingValue * 3.6));
    display.println(" deg");
  }
  else
  {
    display.println("MODE: Temperature");

    int kelvin = map(pendingValue, 0, 100, 2000, 6500);
    display.print(kelvin);
    display.println("K");
  }
}

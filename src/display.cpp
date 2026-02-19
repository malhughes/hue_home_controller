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

// Helper function to draw a progress bar
void drawProgressBar(int x, int y, int width, int height, int percentage, uint16_t fillColor, uint16_t bgColor)
{
  // Draw background (empty part)
  canvas.fillRect(x, y, width, height, bgColor);

  // Draw filled part
  int fillWidth = (width * percentage) / 100;
  canvas.fillRect(x, y, fillWidth, height, fillColor);

  // Draw border
  canvas.drawRect(x, y, width, height, COLOR_WHITE);
}

// Helper function to center text horizontally
void centerText(const String &text, const String &print_type, int y, uint16_t color)
{
  int16_t x1, y1;
  uint16_t w, h;
  canvas.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int centerX = (128 - w) / 2;
  canvas.setCursor(centerX, y);
  canvas.setTextColor(color);
  if (print_type == "println")
    canvas.println(text);
  else
    canvas.print(text);
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
  canvas.fillScreen(COLOR_BLACK);

  if (numLights <= 0)
  {
    canvas.setCursor(0, 40);
    canvas.setTextSize(2);
    canvas.setTextColor(COLOR_WHITE);
    canvas.println("No lights\n  found!");
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
    return;
  }

  bool isTVStripWithSync = (lights[currentLightIndex].rid == tvStripRid && syncActive);

  if (isTVStripWithSync)
  {
    canvas.setCursor(10, 35);
    canvas.setTextSize(2);
    canvas.setTextColor(COLOR_WHITE);
    canvas.println("SYNC\nACTIVE");

    canvas.setCursor(5, 75);
    canvas.setTextSize(1);
    canvas.println("Press SYNC\nbutton to\ncontrol lights");

    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
    return;
  }

  // --- ROOM NAME ---
  canvas.setCursor(2, 2);
  canvas.setTextSize(1);
  canvas.setTextColor(0x7BEF); // Gray color (RGB565)
  canvas.print("(");
  canvas.print(lights[currentLightIndex].room);
  canvas.print(")");

  // --- LIGHT NAME ---
  canvas.setTextSize(2);
  canvas.setTextColor(COLOR_WHITE);

  // Calculate center position for light name
  centerText(lights[currentLightIndex].name, "println", 20, COLOR_WHITE);

  // --- LIGHT OFF ---
  if (!lights[currentLightIndex].isOn)
  {
    canvas.setCursor(10, 60);
    canvas.setTextSize(1);
    canvas.setTextColor(COLOR_WHITE);
    canvas.println("Long press dial\n   to turn on");

    display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
    return;
  }

  // --- LIGHT ON ---
  if (currentMode == BRIGHTNESS)
  {
    int brightness = (int)lights[currentLightIndex].brightness;

    // Progress bar (white/gray)
    int barX = 14;
    int barY = 55;
    int barWidth = 100;
    int barHeight = 12;

    drawProgressBar(barX, barY, barWidth, barHeight, brightness,
                    COLOR_WHITE, 0x2104); // White fill, dark gray background

    // Percentage value (centered below bar)
    canvas.setTextSize(2);
    canvas.setTextColor(COLOR_WHITE);

    String valueStr = String(brightness) + "%";
    centerText(valueStr, "print", 75, COLOR_WHITE);

    // Mode label (centered at bottom)
    canvas.setTextSize(1);
    centerText("Brightness", "print", 110, 0x7BEF);
  }
  // --- COLOR MODE (placeholder for now) ---
  else if (currentMode == COLOR)
  {
    canvas.setCursor(20, 60);
    canvas.setTextSize(1);
    canvas.setTextColor(COLOR_WHITE);
    canvas.print("Color: ");
    canvas.print((int)(pendingValue * 3.6));
    canvas.println(" deg");

    canvas.setCursor(45, 110);
    canvas.print("Color");
  }
  // --- TEMPERATURE MODE (placeholder for now) ---
  else
  {
    int kelvin = map(pendingValue, 0, 100, 2000, 6500);

    canvas.setCursor(30, 60);
    canvas.setTextSize(1);
    canvas.setTextColor(COLOR_WHITE);
    canvas.print("Temp: ");
    canvas.print(kelvin);
    canvas.println("K");

    canvas.setCursor(30, 110);
    canvas.print("Temperature");
  }

  // Push entire canvas to display in one operation
  display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
}
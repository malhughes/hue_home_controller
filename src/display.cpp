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

void setModeLabel(const String &mode)
{
  centerText(mode, "print", 110, 0x7BEF);
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

// Helper to convert hue (0-360) to RGB565 color
uint16_t hueToRGB565(float hue)
{
  // Normalize hue to 0-360
  while (hue < 0)
    hue += 360;
  while (hue >= 360)
    hue -= 360;

  float r, g, b;

  if (hue < 120)
  {
    r = (120 - hue) / 120.0;
    g = hue / 120.0;
    b = 0;
  }
  else if (hue < 240)
  {
    r = 0;
    g = (240 - hue) / 120.0;
    b = (hue - 120) / 120.0;
  }
  else
  {
    r = (hue - 240) / 120.0;
    g = 0;
    b = (360 - hue) / 120.0;
  }

  // Convert to RGB565
  uint8_t r5 = (uint8_t)(r * 31);
  uint8_t g6 = (uint8_t)(g * 63);
  uint8_t b5 = (uint8_t)(b * 31);

  return (r5 << 11) | (g6 << 5) | b5;
}

// Draw a color gradient bar showing the hue
void drawColorBar(int x, int y, int width, int height, float currentHue)
{
  // Draw the full color spectrum as background
  for (int i = 0; i < width; i++)
  {
    float hue = (i * 360.0) / width;
    uint16_t color = hueToRGB565(hue);
    canvas.drawFastVLine(x + i, y, height, color);
  }

  // Draw white border
  canvas.drawRect(x, y, width, height, COLOR_WHITE);

  // Draw indicator line at current hue position
  int indicatorX = x + (int)((currentHue * width) / 360.0);
  canvas.drawFastVLine(indicatorX, y - 2, height + 4, COLOR_WHITE);
  canvas.drawFastVLine(indicatorX - 1, y - 2, height + 4, COLOR_WHITE);
  canvas.drawFastVLine(indicatorX + 1, y - 2, height + 4, COLOR_WHITE);
}

// Helper to get color for temperature gradient
uint16_t getTempColor(float position)
{
  // position: 0.0 (warm/orange) to 1.0 (cool/blue)

  uint8_t r, g, b;

  if (position < 0.5)
  {
    // Warm side: Orange to White
    float t = position * 2.0; // 0 to 1
    r = 255;
    g = (uint8_t)(165 + (255 - 165) * t); // 165 (orange) to 255 (white)
    b = (uint8_t)(0 + 255 * t);           // 0 to 255
  }
  else
  {
    // Cool side: White to Blue
    float t = (position - 0.5) * 2.0; // 0 to 1
    r = (uint8_t)(255 - 255 * t);     // 255 to 0
    g = (uint8_t)(255 - 105 * t);     // 255 to 150 (light blue)
    b = 255;
  }

  // Convert RGB888 to RGB565
  uint8_t r5 = r >> 3;
  uint8_t g6 = g >> 2;
  uint8_t b5 = b >> 3;

  return (r5 << 11) | (g6 << 5) | b5;
}

// Draw temperature gradient bar
void drawTempBar(int x, int y, int width, int height, int percentage)
{
  // Draw gradient from warm (orange) to cool (blue)
  for (int i = 0; i < width; i++)
  {
    float position = (float)i / width;
    uint16_t color = getTempColor(position);
    canvas.drawFastVLine(x + i, y, height, color);
  }

  // Draw white border
  canvas.drawRect(x, y, width, height, COLOR_WHITE);

  // Draw indicator line at current temperature position
  int indicatorX = x + (percentage * width) / 100;
  canvas.drawFastVLine(indicatorX, y - 2, height + 4, COLOR_WHITE);
  canvas.drawFastVLine(indicatorX - 1, y - 2, height + 4, COLOR_WHITE);
  canvas.drawFastVLine(indicatorX + 1, y - 2, height + 4, COLOR_WHITE);
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

    // Progress bar
    int barX = 14;
    int barY = 65;
    int barWidth = 100;
    int barHeight = 12;

    drawProgressBar(barX, barY, barWidth, barHeight, brightness,
                    COLOR_WHITE, 0x2104);

    // Percentage text INSIDE the bar (centered)
    canvas.setTextSize(1);
    String valueStr = String(brightness) + "%";

    // Choose text color based on brightness for contrast
    // Dark text on bright bar, light text on dark bar
    uint16_t textColor = (brightness > 50) ? COLOR_BLACK : COLOR_WHITE;

    int16_t x1, y1;
    uint16_t w, h;
    canvas.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
    int centerX = (128 - w) / 2;

    canvas.setCursor(centerX, barY + 2); // +2 to vertically center in 12px bar
    canvas.setTextColor(textColor);
    canvas.print(valueStr);

    // Mode label
    canvas.setTextSize(1);
    setModeLabel("Brightness");
  }
  // --- COLOR MODE ---
  else if (currentMode == COLOR)
  {
    float hueDegrees = pendingValue * 3.6; // Convert 0-100 to 0-360

    // Color gradient bar showing full spectrum
    int barX = 14;
    int barY = 65;
    int barWidth = 100;
    int barHeight = 12;

    drawColorBar(barX, barY, barWidth, barHeight, hueDegrees);

    // Mode label
    canvas.setTextSize(1);
    setModeLabel("Color");
  }
  // --- TEMPERATURE MODE ---
  else
  {
    int kelvin = map(pendingValue, 0, 100, 2000, 6500);

    // Temperature gradient bar
    int barX = 14;
    int barY = 65;
    int barWidth = 100;
    int barHeight = 12;

    drawTempBar(barX, barY, barWidth, barHeight, pendingValue);

    // Mode label
    canvas.setTextSize(1);
    setModeLabel("Temperature");
  }

  // Push entire canvas to display in one operation
  display.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 128);
}
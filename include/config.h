#pragma once
#include <cstdint>

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 128;

constexpr int SCLK_PIN = 18;
constexpr int MOSI_PIN = 23;
constexpr int CS_PIN = 2;
constexpr int DC_PIN = 26;
constexpr int RST_PIN = 27;

constexpr int SYNC_BUTTON_PIN = 4;
constexpr int CYCLE_BUTTON_PIN = 12;
constexpr int ENCODER_BUTTON_PIN = 5;

constexpr uint16_t COLOR_BLACK = 0x0000;
constexpr uint16_t COLOR_WHITE = 0xFFFF;
constexpr uint16_t COLOR_RED = 0xF800;
constexpr uint16_t COLOR_GREEN = 0x07E0;
constexpr uint16_t COLOR_BLUE = 0x001F;

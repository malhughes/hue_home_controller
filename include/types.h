#pragma once
#include <Arduino.h>

struct Light
{
  String rid;
  String name;
  String room;
  bool isOn;
  float brightness;
};

struct Room
{
  String id;
  String name;
  String deviceRids[10];
  int numDevices;
};

enum Mode
{
  BRIGHTNESS,
  COLOR,
  TEMPERATURE
};

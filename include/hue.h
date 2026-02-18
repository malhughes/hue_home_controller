#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "types.h"
#include "secrets.h"

// Hue API Functions
void fetchRooms(
    HTTPClient &http,
    WiFiClientSecure &client,
    Room rooms[],
    int &numRooms);

String getRoomNameForDevice(
    String deviceRid,
    Room rooms[],
    int numRooms);

void fetchLights(
    HTTPClient &http,
    WiFiClientSecure &client,
    Light lights[],
    int &numLights,
    Room rooms[],
    int numRooms,
    String &tvStripRid);

void fetchLightDetails(
    HTTPClient &http,
    WiFiClientSecure &client,
    String lightRid,
    int index,
    Light lights[],
    Room rooms[],
    int numRooms,
    String &tvStripRid);

void toggleLight(
    HTTPClient &http,
    WiFiClientSecure &client,
    String lightRid,
    bool state);

void setBrightness(
    HTTPClient &http,
    WiFiClientSecure &client,
    String lightRid,
    float brightness);

void hueToXY(float hue, float &x, float &y);

void setColor(
    HTTPClient &http,
    WiFiClientSecure &client,
    String lightRid,
    float hueValue);

void setColorTemperature(
    HTTPClient &http,
    WiFiClientSecure &client,
    String lightRid,
    float tempValue);

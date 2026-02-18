#include "hue.h"
#include <ArduinoJson.h>

// Fetch all rooms
void fetchRooms(HTTPClient &http, WiFiClientSecure &client, Room rooms[], int &numRooms)
{
  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/room";

  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);

  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("Room JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    JsonArray data = doc["data"];
    numRooms = 0;

    for (JsonObject room : data)
    {
      rooms[numRooms].id = room["id"].as<String>();
      rooms[numRooms].name = room["metadata"]["name"].as<String>();

      JsonArray children = room["children"];
      rooms[numRooms].numDevices = 0;

      for (JsonObject child : children)
      {
        if (child["rtype"] == "device")
        {
          rooms[numRooms].deviceRids[rooms[numRooms].numDevices] = child["rid"].as<String>();
          rooms[numRooms].numDevices++;
        }
      }

      Serial.print("Room: ");
      Serial.print(rooms[numRooms].name);
      Serial.print(" (");
      Serial.print(rooms[numRooms].numDevices);
      Serial.println(" devices)");

      numRooms++;
      if (numRooms >= 10)
        break;
    }
  }
  else
  {
    Serial.print("fetchRooms HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
}

String getRoomNameForDevice(String deviceRid, Room rooms[], int numRooms)
{
  for (int i = 0; i < numRooms; i++)
  {
    for (int j = 0; j < rooms[i].numDevices; j++)
    {
      if (rooms[i].deviceRids[j] == deviceRid)
      {
        return rooms[i].name;
      }
    }
  }
  return "Unknown";
}

// Fetch lights list
void fetchLights(
    HTTPClient &http,
    WiFiClientSecure &client,
    Light lights[],
    int &numLights,
    Room rooms[],
    int numRooms,
    String &tvStripRid)
{
  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/device";

  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);

  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("Device JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    numLights = 0;

    JsonArray data = doc["data"];
    for (JsonObject device : data)
    {
      JsonArray services = device["services"];

      for (JsonObject service : services)
      {
        if (service["rtype"] == "light")
        {
          String lightRid = service["rid"].as<String>();

          fetchLightDetails(
              http,
              client,
              lightRid,
              numLights,
              lights,
              rooms,
              numRooms,
              tvStripRid);

          numLights++;

          if (numLights >= 10)
            break;
        }
      }

      if (numLights >= 10)
        break;
    }
  }
  else
  {
    Serial.print("fetchLights HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
}

// Fetch light details
void fetchLightDetails(
    HTTPClient &http,
    WiFiClientSecure &client,
    String lightRid,
    int index,
    Light lights[],
    Room rooms[],
    int numRooms,
    String &tvStripRid)
{
  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/light/" + lightRid;

  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);

  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("Light detail JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    JsonObject data = doc["data"][0];

    lights[index].rid = lightRid;
    lights[index].name = data["metadata"]["name"].as<String>();
    lights[index].isOn = data["on"]["on"].as<bool>();
    lights[index].brightness = data["dimming"]["brightness"].as<float>();

    String deviceRid = data["owner"]["rid"].as<String>();
    lights[index].room = getRoomNameForDevice(deviceRid, rooms, numRooms);

    if (lights[index].name == "TV Strip")
    {
      tvStripRid = lightRid;
      Serial.println("Found TV strip!");
    }

    Serial.print("Found: ");
    Serial.print(lights[index].room);
    Serial.print(" - ");
    Serial.println(lights[index].name);
  }
  else
  {
    Serial.print("fetchLightDetails HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
}

void toggleLight(HTTPClient &http, WiFiClientSecure &client, String lightRid, bool state)
{
  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/light/" + lightRid;

  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);
  http.addHeader("Content-Type", "application/json");

  String body = state ? "{\"on\":{\"on\":true}}" : "{\"on\":{\"on\":false}}";

  int httpCode = http.PUT(body);
  Serial.print("Toggle light response: ");
  Serial.println(httpCode);

  http.end();
}

void setBrightness(HTTPClient &http, WiFiClientSecure &client, String lightRid, float brightness)
{
  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/light/" + lightRid;

  http.end();
  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");

  String body = "{\"dimming\":{\"brightness\":" + String(brightness) + "}}";

  int httpCode = http.PUT(body);

  Serial.print("Brightness update: ");
  Serial.println(httpCode);
}

void hueToXY(float hue, float &x, float &y)
{
  float hueDegrees = hue * 3.6;
  float hueRadians = hueDegrees * PI / 180.0;

  float r, g, b;

  if (hueDegrees < 120)
  {
    r = 1.0 - (hueDegrees / 120.0);
    g = hueDegrees / 120.0;
    b = 0;
  }
  else if (hueDegrees < 240)
  {
    r = 0;
    g = 1.0 - ((hueDegrees - 120) / 120.0);
    b = (hueDegrees - 120) / 120.0;
  }
  else
  {
    r = (hueDegrees - 240) / 120.0;
    g = 0;
    b = 1.0 - ((hueDegrees - 240) / 120.0);
  }

  float X = r * 0.649926 + g * 0.103455 + b * 0.197109;
  float Y = r * 0.234327 + g * 0.743075 + b * 0.022598;
  float Z = r * 0.000000 + g * 0.053077 + b * 1.035763;

  float sum = X + Y + Z;
  if (sum == 0)
  {
    x = 0;
    y = 0;
  }
  else
  {
    x = X / sum;
    y = Y / sum;
  }
}

void setColor(HTTPClient &http, WiFiClientSecure &client, String lightRid, float hueValue)
{
  float x, y;
  hueToXY(hueValue, x, y);

  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/light/" + lightRid;

  http.end();
  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");

  String body = "{\"color\":{\"xy\":{\"x\":" + String(x, 4) + ",\"y\":" + String(y, 4) + "}}}";

  int httpCode = http.PUT(body);
  Serial.print("Color update: ");
  Serial.println(httpCode);
}

void setColorTemperature(HTTPClient &http, WiFiClientSecure &client, String lightRid, float tempValue)
{
  int mirek = map(tempValue, 0, 100, 500, 153);

  String url = "https://" + String(HUE_IP) + "/clip/v2/resource/light/" + lightRid;

  http.end();
  http.begin(client, url);
  http.addHeader("hue-application-key", HUE_APP_KEY);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");

  String body = "{\"color_temperature\":{\"mirek\":" + String(mirek) + "}}";

  int httpCode = http.PUT(body);
  Serial.print("Temp update: ");
  Serial.println(httpCode);
}

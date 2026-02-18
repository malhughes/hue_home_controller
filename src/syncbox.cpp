#include "syncbox.h"
#include <ArduinoJson.h>

bool fetchSyncState(HTTPClient &http, WiFiClientSecure &client)
{
  String url = "https://" + String(SYNCBOX_IP) + "/api/v1/execution/";

  http.begin(client, url);
  http.addHeader("Authorization", "Bearer " + String(SYNCBOX_TOKEN));

  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("Sync JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }

    bool active = doc["syncActive"].as<bool>();

    Serial.print("Sync state: ");
    Serial.println(active ? "ON" : "OFF");

    http.end();
    return active;
  }

  Serial.print("Failed to get sync state: ");
  Serial.println(httpCode);

  http.end();
  return false;
}

void toggleSync(HTTPClient &http, WiFiClientSecure &client, bool &syncActive)
{
  syncActive = !syncActive;

  String url = "https://" + String(SYNCBOX_IP) + "/api/v1/execution/";

  http.begin(client, url);
  http.addHeader("Authorization", "Bearer " + String(SYNCBOX_TOKEN));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");

  String body = syncActive ? "{\"syncActive\":true}" : "{\"syncActive\":false}";

  int httpCode = http.PUT(body);

  Serial.print("Sync toggled to: ");
  Serial.print(syncActive ? "ON" : "OFF");
  Serial.print(" (");
  Serial.print(httpCode);
  Serial.println(")");

  http.end();
}

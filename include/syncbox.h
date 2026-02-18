#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

// Sync box functions
bool fetchSyncState(
    HTTPClient &http,
    WiFiClientSecure &client);

void toggleSync(
    HTTPClient &http,
    WiFiClientSecure &client,
    bool &syncActive);

/*
 * WiFi Management Module Implementation
 */

#include "global.h"

void loadWiFiCredentials() {
  savedSSID = preferences.getString("ssid", "");
  savedPassword = preferences.getString("password", "");
  
  if (savedSSID.length() > 0) {
    debugPrintln("Loaded WiFi credentials from flash: " + savedSSID);
  }
}

void saveWiFiCredentials(const String& ssid, const String& password) {
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  savedSSID = ssid;
  savedPassword = password;
  debugPrintln("WiFi credentials saved to flash");
}

void clearWiFiCredentials() {
  preferences.clear();
  savedSSID = "";
  savedPassword = "";
  debugPrintln("WiFi credentials cleared from flash");
}

bool connectToWiFi(const String& ssid, const String& password) {
  debugPrintln("Attempting to connect to WiFi: " + ssid);
  SerialBT.println("Connecting to " + ssid + "...");
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
    delay(500);
    SerialBT.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    reconnectRetries = 0;
    SerialBT.println("\nWiFi connected successfully!");
    SerialBT.println("IP Address: " + WiFi.localIP().toString());
    debugPrintln("WiFi connected. IP: " + WiFi.localIP().toString());
    return true;
  } else {
    wifiConnected = false;
    SerialBT.println("\nFailed to connect to WiFi");
    debugPrintln("WiFi connection failed");
    return false;
  }
}

void scanWiFiNetworks() {
  SerialBT.println("Scanning for WiFi networks...");
  debugPrintln("Starting WiFi scan");
  
  wifiNetworkCount = WiFi.scanNetworks();
  
  if (wifiNetworkCount == 0) {
    SerialBT.println("No networks found");
    return;
  }
  
  // Store network information
  int displayCount = min(wifiNetworkCount, MAX_NETWORKS);
  for (int i = 0; i < displayCount; i++) {
    wifiNetworks[i] = WiFi.SSID(i);
    wifiRSSI[i] = WiFi.RSSI(i);
    wifiSecurity[i] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
  }
  
  wifiNetworkCount = displayCount;
  debugPrintln("Found " + String(wifiNetworkCount) + " networks");
}

void displayWiFiNetworks() {
  if (wifiNetworkCount == 0) {
    SerialBT.println("No networks available. Use option 2 to scan for networks.");
    return;
  }
  
  SerialBT.println("\n=== Available WiFi Networks ===");
  
  for (int i = 0; i < wifiNetworkCount; i++) {
    SerialBT.println(String(i + 1) + ". " + wifiNetworks[i]);
    String security = wifiSecurity[i] ? "Secured" : "Open";
    SerialBT.println("   Security: " + security + 
                     " | Signal: " + getSignalStrength(wifiRSSI[i]) + 
                     " (" + String(wifiRSSI[i]) + " dBm)");
    SerialBT.println();
  }
  
  SerialBT.println("================================");
}

void checkWiFiConnection() {
  bool currentStatus = (WiFi.status() == WL_CONNECTED);
  
  if (currentStatus != wifiConnected) {
    wifiConnected = currentStatus;
    if (wifiConnected) {
      debugPrintln("WiFi reconnected");
      SerialBT.println("WiFi reconnected!");
      reconnectRetries = 0;
    } else {
      debugPrintln("WiFi disconnected");
      SerialBT.println("WiFi connection lost");
    }
  }
}

void autoReconnectWiFi() {
  if (reconnectRetries >= MAX_RETRIES) {
    return; // Max retries reached, wait for reset
  }
  
  if (millis() - lastReconnectAttempt < RECONNECT_DELAY) {
    return; // Too soon to retry
  }
  
  lastReconnectAttempt = millis();
  reconnectRetries++;
  
  debugPrintln("Auto-reconnect attempt " + String(reconnectRetries) + "/" + String(MAX_RETRIES));
  connectToWiFi(savedSSID, savedPassword);
}

String getSignalStrength(int rssi) {
  if (rssi > -50) return "Excellent";
  else if (rssi > -65) return "Good";
  else if (rssi > -80) return "Fair";
  else return "Poor";
}

String getSecurityType(bool isOpen) {
  return isOpen ? "Open" : "WPA2";
}
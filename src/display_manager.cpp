#include "global.h"

// Display update intervals
unsigned long lastDisplayUpdate = 0;

void initializeDisplay() {
    Serial.println(F("Display Manager: Initializing display hardware..."));
    Serial.println(F("Display Manager: Display hardware initialized"));
    clearDisplay();
    displayWelcomeMessage();
}

void clearDisplay() {
    Serial.println(F("Display cleared"));
}

void displayWelcomeMessage() {
    Serial.println(F("=== Islamic Prayer Times System ==="));
    Serial.println(F("    Initializing..."));
    delay(2000);
}

void updateDisplay() {
    unsigned long currentMillis = millis();
    
    // Update display every second
    if (currentMillis - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = currentMillis;
        
        DateTime now = rtc.now();
        if (now.year() > 2000) { // Valid time check
            displayCurrentInfo(now);
        } else {
            displayError(F("RTC Error"));
        }
    }
}

void displayCurrentInfo(DateTime now) {
    // Format current time and date
    char timeStr[9];
    char dateStr[11];
    
    sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    sprintf(dateStr, "%02d/%02d/%04d", now.day(), now.month(), now.year());
    
    // Display to Serial (for debugging) - only every 10 seconds to avoid spam
    static unsigned long lastSerialUpdate = 0;
    if (millis() - lastSerialUpdate >= 10000) {
        lastSerialUpdate = millis();
        Serial.println(F("=== Current Display Info ==="));
        Serial.printf("Time: %s\n", timeStr);
        Serial.printf("Date: %s\n", dateStr);
        Serial.printf("City: %s (%s)\n", currentCity.c_str(), currentTimezone.c_str());
    }
}

void displayPrayerAlert(const String& prayerName) {
    Serial.printf("PRAYER ALERT: %s TIME!\n", prayerName.c_str());
    
    // Flash display for 10 seconds
    for (int i = 0; i < 10; i++) {
        delay(500);
        delay(500);
    }
}

void displayWarningAlert(const String& prayerName, int minutesLeft) {
    Serial.printf("PRAYER WARNING: %s in %d minutes\n", prayerName.c_str(), minutesLeft);
}

void displayError(const String& errorMsg) {
    Serial.printf("DISPLAY ERROR: %s\n", errorMsg.c_str());
}

void displaySystemStatus() {
    Serial.println(F("=== System Status ==="));
    
    extern bool wifiConnected;
    extern bool rtcInitialized;
    extern bool sdCardInitialized;
    
    Serial.printf("WiFi: %s\n", wifiConnected ? "Connected" : "Disconnected");
    Serial.printf("RTC: %s\n", rtcInitialized ? "OK" : "Error");
    Serial.printf("SD Card: %s\n", sdCardInitialized ? "OK" : "Error");
}
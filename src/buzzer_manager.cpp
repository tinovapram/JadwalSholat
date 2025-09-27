#include "global.h"

// Buzzer control variables
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
unsigned long lastBuzzerCheck = 0;

// Prayer alert tracking
String lastAlertPrayer = "";
int lastAlertDay = -1;
String lastWarningPrayer = "";
int lastWarningDay = -1;

void initializeBuzzer() {
    Serial.println(F("Buzzer Manager: Initializing buzzer..."));
    
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off
    
    buzzerInitialized = true;
    Serial.printf("Buzzer Manager: Buzzer initialized on pin %d\n", BUZZER_PIN);
}

void updateBuzzer() {
    if (!buzzerInitialized) return;
    
    unsigned long currentMillis = millis();
    
    // Check buzzer status every second
    if (currentMillis - lastBuzzerCheck >= 1000) {
        lastBuzzerCheck = currentMillis;
        checkPrayerAlerts();
    }
    
    // Handle active buzzer patterns
    handleBuzzerPattern(currentMillis);
}

void checkPrayerAlerts() {
    DateTime now = rtc.now();
    if (now.year() <= 2000) return; // Invalid time
    
    String todayPrayerTimes = getPrayerTimesFromCache(getCurrentDateString());
    if (todayPrayerTimes == "") return;
    
    // Parse prayer times and check for alerts
    checkPrayerTimeAlerts(now, todayPrayerTimes);
}

void checkPrayerTimeAlerts(DateTime now, const String& prayerTimesJson) {
    if (prayerTimesJson.length() < 10) return; // Invalid data
    
    // Parse JSON to extract prayer times
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, prayerTimesJson);
    
    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.f_str());
        return;
    }
    
    // Check if we have the timings object
    if (!doc["data"]["timings"]) {
        Serial.println("No timings data found in JSON");
        return;
    }
    
    JsonObject timings = doc["data"]["timings"];
    
    struct PrayerTime {
        String name;
        int hour;
        int minute;
    };
    
    // Extract prayer times from JSON
    PrayerTime prayers[5];
    int prayerCount = 0;
    
    // Helper function to parse time string "HH:MM" to hour and minute
    auto parseTime = [](const String& timeStr, int& hour, int& minute) {
        int colonPos = timeStr.indexOf(':');
        if (colonPos > 0) {
            hour = timeStr.substring(0, colonPos).toInt();
            minute = timeStr.substring(colonPos + 1, colonPos + 3).toInt();
            return true;
        }
        return false;
    };
    
    // Parse each prayer time
    if (timings["Fajr"]) {
        String fajrTime = timings["Fajr"].as<String>();
        if (parseTime(fajrTime, prayers[prayerCount].hour, prayers[prayerCount].minute)) {
            prayers[prayerCount].name = "Fajr";
            prayerCount++;
        }
    }
    
    if (timings["Dhuhr"]) {
        String dhuhrTime = timings["Dhuhr"].as<String>();
        if (parseTime(dhuhrTime, prayers[prayerCount].hour, prayers[prayerCount].minute)) {
            prayers[prayerCount].name = "Dhuhr";
            prayerCount++;
        }
    }
    
    if (timings["Asr"]) {
        String asrTime = timings["Asr"].as<String>();
        if (parseTime(asrTime, prayers[prayerCount].hour, prayers[prayerCount].minute)) {
            prayers[prayerCount].name = "Asr";
            prayerCount++;
        }
    }
    
    if (timings["Maghrib"]) {
        String maghribTime = timings["Maghrib"].as<String>();
        if (parseTime(maghribTime, prayers[prayerCount].hour, prayers[prayerCount].minute)) {
            prayers[prayerCount].name = "Maghrib";
            prayerCount++;
        }
    }
    
    if (timings["Isha"]) {
        String ishaTime = timings["Isha"].as<String>();
        if (parseTime(ishaTime, prayers[prayerCount].hour, prayers[prayerCount].minute)) {
            prayers[prayerCount].name = "Isha";
            prayerCount++;
        }
    }
    
    int currentHour = now.hour();
    int currentMinute = now.minute();
    int currentDay = now.day();
    
    // Check each prayer time
    for (int i = 0; i < prayerCount; i++) {
        PrayerTime prayer = prayers[i];
        
        // Calculate minutes difference
        int prayerMinutes = prayer.hour * 60 + prayer.minute;
        int currentMinutes = currentHour * 60 + currentMinute;
        int timeDiff = prayerMinutes - currentMinutes;
        
        // Check for exact prayer time (on-off buzzer for 10 seconds)
        if (timeDiff == 0) {
            if (lastAlertPrayer != prayer.name || lastAlertDay != currentDay) {
                Serial.printf("PRAYER TIME ALERT: %s at %02d:%02d\n", prayer.name.c_str(), prayer.hour, prayer.minute);
                startPrayerTimeBuzzer(prayer.name);
                lastAlertPrayer = prayer.name;
                lastAlertDay = currentDay;
            }
        }
        
        // Check for 10 minutes warning (continuous buzz for 1 second)
        else if (timeDiff == 10) {
            if (lastWarningPrayer != prayer.name || lastWarningDay != currentDay) {
                Serial.printf("PRAYER WARNING: %s in 10 minutes (%02d:%02d)\n", prayer.name.c_str(), prayer.hour, prayer.minute);
                startPrayerWarningBuzzer(prayer.name);
                lastWarningPrayer = prayer.name;
                lastWarningDay = currentDay;
            }
        }
    }
}

void startPrayerTimeBuzzer(const String& prayerName) {
    Serial.printf("Starting prayer time buzzer for %s\n", prayerName.c_str());
    currentBuzzerMode = BUZZER_PRAYER_TIME;
    buzzerStartTime = millis();
    buzzerActive = true;
    
    // Display alert as well
    displayPrayerAlert(prayerName);
}

void startPrayerWarningBuzzer(const String& prayerName) {
    Serial.printf("Starting prayer warning buzzer for %s\n", prayerName.c_str());
    currentBuzzerMode = BUZZER_WARNING;
    buzzerStartTime = millis();
    buzzerActive = true;
    
    // Display warning as well
    displayWarningAlert(prayerName, 10);
}

void handleBuzzerPattern(unsigned long currentMillis) {
    if (!buzzerActive) return;
    
    unsigned long elapsed = currentMillis - buzzerStartTime;
    
    switch (currentBuzzerMode) {
        case BUZZER_PRAYER_TIME:
            handlePrayerTimeBuzzer(elapsed);
            break;
            
        case BUZZER_WARNING:
            handleWarningBuzzer(elapsed);
            break;
            
        case BUZZER_ALARM:
            handleAlarmBuzzer(elapsed);
            break;
            
        default:
            stopBuzzer();
            break;
    }
}

void handlePrayerTimeBuzzer(unsigned long elapsed) {
    // On-off pattern for 10 seconds total
    // 0.5s ON, 0.5s OFF, repeat for 10 cycles = 10 seconds
    
    if (elapsed >= 10000) { // 10 seconds total
        stopBuzzer();
        return;
    }
    
    unsigned long cycle = elapsed % 1000; // 1 second cycles
    
    if (cycle < 500) {
        digitalWrite(BUZZER_PIN, HIGH); // ON for 0.5 seconds
    } else {
        digitalWrite(BUZZER_PIN, LOW);  // OFF for 0.5 seconds
    }
}

void handleWarningBuzzer(unsigned long elapsed) {
    // Continuous buzz for 1 second
    
    if (elapsed >= 1000) { // 1 second total
        stopBuzzer();
        return;
    }
    
    digitalWrite(BUZZER_PIN, HIGH); // Continuous ON
}

void handleAlarmBuzzer(unsigned long elapsed) {
    // Custom alarm pattern (can be used for other alerts)
    // Fast beeping: 0.1s ON, 0.1s OFF
    
    if (elapsed >= 5000) { // 5 seconds total
        stopBuzzer();
        return;
    }
    
    unsigned long cycle = elapsed % 200; // 0.2 second cycles
    
    if (cycle < 100) {
        digitalWrite(BUZZER_PIN, HIGH); // ON for 0.1 seconds
    } else {
        digitalWrite(BUZZER_PIN, LOW);  // OFF for 0.1 seconds
    }
}

void stopBuzzer() {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    currentBuzzerMode = BUZZER_OFF;
    buzzerStartTime = 0;
    Serial.println(F("Buzzer stopped"));
}

void testBuzzer() {
    Serial.println(F("Testing buzzer..."));
    
    // Test different patterns
    Serial.println(F("Testing prayer time pattern..."));
    startPrayerTimeBuzzer("Test");
    delay(11000);
    
    Serial.println(F("Testing warning pattern..."));
    startPrayerWarningBuzzer("Test");
    delay(2000);
    
    Serial.println(F("Testing alarm pattern..."));
    currentBuzzerMode = BUZZER_ALARM;
    buzzerStartTime = millis();
    buzzerActive = true;
    delay(6000);
    
    Serial.println(F("Buzzer test complete"));
}
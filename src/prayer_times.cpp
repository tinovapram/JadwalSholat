/*
 * Prayer Times Manager Implementation
 * Enhanced version with SD card prioritization and 7-day caching
 */

#include "global.h"

String lastPrayerData = "";

void fetchPrayerTimes() {
  // First try to load from SD card
  if (loadPrayerTimesFromSD()) {
    SerialBT.println("✅ Prayer times loaded from SD card");
    debugPrintln("Prayer times loaded from SD card (current date)");
    return;
  }
  
  if (!wifiConnected) {
    SerialBT.println("❌ WiFi not connected and no cached data available.");
    debugPrintln("Prayer times fetch failed - no WiFi and no cache");
    return;
  }
  
  debugPrintln("Fetching prayer times from Aladhan API...");
  SerialBT.println("🔄 Fetching prayer times from API...");
  
  SerialBT.println("Fetching prayer times for " + currentCity + "...");
  debugPrintln("Fetching prayer times for " + currentCity);
  
  // Get current date for API call
  String currentDate = getCurrentDateString();
  
  String url = String(ALADHAN_API_BASE) + "/" + currentDate + "?city=" + currentCity + 
               "&country=" + String(DEFAULT_COUNTRY) + "&method=" + String(PRAYER_METHOD);
  
  debugPrintln("API URL: " + url);
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT);
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    displayPrayerTimes(payload, true); // true = from API
    lastPrayerData = payload;
    
    // Save to SD card with current date
    savePrayerTimesToSD(payload, currentDate);
    
    SerialBT.println("✅ Prayer times updated successfully!");
    debugPrintln("Prayer times fetch completed successfully");
    
    // If connected to internet during boot, cache 7 days ahead
    if (isFirstBoot && wifiConnected) {
      fetchPrayerTimesForDays(PRAYER_CACHE_DAYS);
    }
  } else {
    SerialBT.println("Failed to fetch prayer times. HTTP code: " + String(httpCode));
    debugPrintln("HTTP request failed: " + String(httpCode));
    debugPrintln("URL used: " + url);
    
    // Try to load from SD card as fallback
    if (loadPrayerTimesFromSD()) {
      SerialBT.println("Using cached prayer times from SD card");
    }
  }
  
  http.end();
}

void fetchPrayerTimesForDays(int days) {
  if (!wifiConnected) {
    debugPrintln("Cannot cache future days - no WiFi connection");
    return;
  }
  
  debugPrintln("Caching prayer times for next " + String(days) + " days...");
  SerialBT.println("💾 Caching prayer times for " + String(days) + " days...");
  
  DateTime now = rtc.now();
  int cachedCount = 0;
  
  for (int i = 1; i <= days; i++) {
    DateTime futureDate = DateTime(now.unixtime() + (i * 86400L)); // Add i days
    String dateStr = String(futureDate.day()) + "-" + 
                     String(futureDate.month()) + "-" + 
                     String(futureDate.year());
    
    // Check if already cached
    String filePath = "/" + currentCity + "/" + String(futureDate.year()) + "/" + 
                     String(futureDate.month()) + "/" + dateStr + ".json";
    
    if (fileExists(filePath)) {
      debugPrintln("Skipping " + dateStr + " - already cached");
      continue;
    }
    
    // Fetch prayer times for this date
    String url = String(ALADHAN_API_BASE) + "/" + dateStr + 
                "?city=" + currentCity + 
                "&country=" + String(DEFAULT_COUNTRY) + 
                "&method=" + String(PRAYER_METHOD);
    
    debugPrintln("Caching: " + url);
    
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      savePrayerTimesToSD(payload, dateStr);
      cachedCount++;
      debugPrintln("Cached prayer times for " + dateStr);
    } else {
      debugPrintln("Failed to cache " + dateStr + " - HTTP " + String(httpCode));
    }
    
    http.end();
    delay(1000); // Rate limiting
  }
  
  SerialBT.println("💾 Cached " + String(cachedCount) + " days of prayer times");
  debugPrintln("Prayer times caching completed: " + String(cachedCount) + " days cached");
}

void displayPrayerTimes(const String& jsonResponse, bool fromAPI) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonResponse);
  
  if (error) {
    SerialBT.println("Error parsing prayer times data");
    debugPrintln("JSON parsing error: " + String(error.c_str()));
    return;
  }
  
  JsonObject data = doc["data"];
  JsonObject timings = data["timings"];
  JsonObject date = data["date"];
  JsonObject meta = data["meta"];
  
  String readable = date["readable"];
  
  // Update timezone from API response
  if (meta["timezone"].is<const char*>()) {
    String apiTimezone = meta["timezone"];
    String oldTimezone = currentTimezone;
    updateTimezoneFromAPI(apiTimezone);
    
    // Only sync NTP if timezone changed
    if (oldTimezone != currentTimezone) {
      debugPrintln("Timezone changed from " + oldTimezone + " to " + currentTimezone + ", syncing NTP...");
      syncTimeWithNTP(true);
    } else {
      debugPrintln("Timezone unchanged (" + currentTimezone + "), skipping NTP sync");
    }
  }
  
  String tzAbbr = getTimezoneAbbreviation(timezoneOffset);
  
  SerialBT.println("\n=== Prayer Times for " + currentCity + " ===");
  SerialBT.println("Date: " + readable);
  SerialBT.println("Timezone: " + tzAbbr + " (GMT+" + String(timezoneOffset) + ")");
  SerialBT.println("Fajr    : " + String((const char*)timings["Fajr"]) + " " + tzAbbr);
  SerialBT.println("Dhuhr   : " + String((const char*)timings["Dhuhr"]) + " " + tzAbbr);
  SerialBT.println("Asr     : " + String((const char*)timings["Asr"]) + " " + tzAbbr);
  SerialBT.println("Maghrib : " + String((const char*)timings["Maghrib"]) + " " + tzAbbr);
  SerialBT.println("Isha    : " + String((const char*)timings["Isha"]) + " " + tzAbbr);
  SerialBT.println("================================\n");
  
  // Save prayer times to SD card only if data came from API
  if (fromAPI && sdCardInitialized) {
    String currentDate = getCurrentDateString();
    savePrayerTimesToSD(jsonResponse, currentDate);
  }
}

bool loadPrayerTimesFromSD(const String& date) {
  if (!sdCardInitialized) {
    debugPrintln("SD card not initialized for prayer times loading");
    return false;
  }
  
  String filePath = "/" + currentCity + "/" + date.substring(6, 10) + "/" + 
                   date.substring(3, 5) + "/" + date + ".json";
  
  debugPrintln("Trying to load prayer times from: " + filePath);
  
  if (!fileExists(filePath)) {
    debugPrintln("Prayer times file not found: " + filePath);
    return false;
  }
  
  String jsonData = readFile(filePath);
  if (jsonData.length() == 0) {
    debugPrintln("Empty or corrupted prayer times file: " + filePath);
    return false;
  }
  
  displayPrayerTimes(jsonData, false); // false = from SD card
  debugPrintln("Prayer times loaded successfully from SD card");
  return true;
}

bool loadPrayerTimesFromSD() {
  if (!sdCardInitialized) {
    debugPrintln("SD card not initialized for prayer times loading");
    return false;
  }
  
  // Get current date string
  String currentDate = getCurrentDateString();
  return loadPrayerTimesFromSD(currentDate);
}

void updateTimezoneFromAPI(const String& apiTimezone) {
  String oldTimezone = currentTimezone; // Store old timezone
  currentTimezone = apiTimezone;
  
  // Map API timezone to GMT offset
  if (apiTimezone == "Asia/Jakarta") {
    timezoneOffset = 7;
  } else if (apiTimezone == "Asia/Makassar") {
    timezoneOffset = 8;
  } else if (apiTimezone == "Asia/Jayapura") {
    timezoneOffset = 9;
  } else {
    // Default to WIB if unknown timezone
    timezoneOffset = 7;
    debugPrintln("Unknown timezone from API, defaulting to GMT+7");
  }
  
  debugPrintln("Timezone updated to: " + currentTimezone + " (GMT+" + String(timezoneOffset) + ")");
  
  // Save timezone to preferences
  preferences.putString("timezone", currentTimezone);
  preferences.putInt("tz_offset", timezoneOffset);
  
  // Re-sync time with new timezone
  // Sync NTP only if timezone changed
  if (oldTimezone != currentTimezone) {
    debugPrintln("Timezone changed, syncing NTP...");
    syncTimeWithNTP(true);
  }
}

String getTimezoneAbbreviation(int offset) {
  switch(offset) {
    case 7: return "WIB";
    case 8: return "WITA";
    case 9: return "WIT";
    default: return "WIB";
  }
}

String getPrayerTimesFromCache(const String& dateKey) {
  // Convert date key from dd-mm-yyyy format to dd-mm format if needed
  String searchKey = dateKey;
  if (dateKey.length() > 5) {
    // Extract dd-mm from dd-mm-yyyy
    searchKey = dateKey.substring(0, 5);
  }
  
  // Load prayer times from SD card using the correct function
  String filename = searchKey + ".json";
  String prayerData = loadPrayerDataFromSD(filename);
  return prayerData;
}

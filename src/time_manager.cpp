/*
 * Time Management Module Implementation
 */

#include "global.h"

void initializeRTC() {
  if (rtc.begin()) {
    rtcInitialized = true;
    debugPrintln("RTC DS3231 initialized successfully");
    
    if (rtc.lostPower()) {
      debugPrintln("RTC lost power, will sync with NTP when WiFi connects");
    }
  } else {
    rtcInitialized = false;
    debugPrintln("ERROR: Could not initialize RTC DS3231");
  }
}

void syncTimeWithNTP() {
  syncTimeWithNTP(false);
}

void syncTimeWithNTP(bool forceSync) {
  if (!wifiConnected) {
    SerialBT.println("WiFi not connected. Cannot sync time.");
    return;
  }
  
  SerialBT.println("Syncing time with NTP server...");
  debugPrintln("Starting NTP sync with timezone: " + currentTimezone + " (GMT+" + String(timezoneOffset) + ")");
  
  // Configure NTP with multiple servers for redundancy
  configTime(timezoneOffset * 3600, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < NTP_SYNC_ATTEMPTS) {
    delay(1000);
    attempts++;
    debugPrintln("NTP sync attempt " + String(attempts) + "/" + String(NTP_SYNC_ATTEMPTS));
    if (attempts % 5 == 0) {
      SerialBT.println("NTP sync attempt " + String(attempts) + "/" + String(NTP_SYNC_ATTEMPTS) + "...");
    }
  }
  
  if (attempts < NTP_SYNC_ATTEMPTS) {
    SerialBT.println("Time synchronized successfully");
    // updateRTCFromNTP(); // Inline implementation below
    
    // Update RTC with NTP time if available
    if (rtcInitialized) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        debugPrintln("RTC updated from NTP time");
      }
    }
    debugPrintln("NTP sync successful after " + String(attempts) + " attempts");
  } else {
    SerialBT.println("Failed to sync time with NTP after 15 attempts");
    debugPrintln("NTP sync failed - check internet connection");
  }
}

void updateRTCFromNTP() {
  if (!rtcInitialized) return;
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    DateTime now(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    rtc.adjust(now);
    debugPrintln("RTC updated from NTP");
  }
}

String getCurrentTime() {
  String tzAbbr = "WIB";
  if (timezoneOffset == 8) tzAbbr = "WITA";
  else if (timezoneOffset == 9) tzAbbr = "WIT";
  
  if (rtcInitialized) {
    DateTime now = rtc.now();
    char timeBuffer[25];
    sprintf(timeBuffer, "%02d/%02d/%04d %02d:%02d:%02d %s",
            now.day(), now.month(), now.year(),
            now.hour(), now.minute(), now.second(), tzAbbr.c_str());
    return String(timeBuffer);
  } else {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeBuffer[25];
      sprintf(timeBuffer, "%02d/%02d/%04d %02d:%02d:%02d %s",
              timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
              timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tzAbbr.c_str());
      return String(timeBuffer);
    }
    return "Time not available";
  }
}

String getCurrentDateForAPI() {
  if (rtcInitialized) {
    DateTime now = rtc.now();
    char dateStr[12];
    sprintf(dateStr, "%02d-%02d-%04d", now.day(), now.month(), now.year());
    return String(dateStr);
  } else {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char dateStr[12];
      sprintf(dateStr, "%02d-%02d-%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      return String(dateStr);
    } else {
      // Fallback to today's date
      return "27-09-2025";
    }
  }
}

String getCurrentDateString() {
  if (rtcInitialized) {
    DateTime now = rtc.now();
    char dateStr[12];
    sprintf(dateStr, "%02d-%02d-%04d", now.day(), now.month(), now.year());
    return String(dateStr);
  } else {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char dateStr[12];
      sprintf(dateStr, "%02d-%02d-%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      return String(dateStr);
    } else {
      // Fallback to today's date
      return "27-09-2025";
    }
  }
}
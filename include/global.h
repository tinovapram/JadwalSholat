/*
 * Global Header File for ESP32 Prayer Times Controller
 * Contains all global variables, objects, and function declarations
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <WiFi.h>
#include <BluetoothSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>
#include <Preferences.h>
#include <time.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include "config.h"

// Global objects
extern BluetoothSerial SerialBT;
extern RTC_DS3231 rtc;
extern Preferences preferences;
extern HTTPClient http;

// Global variables - WiFi Management
extern String savedSSID;
extern String savedPassword;
extern String currentCity;
extern String currentTimezone;
extern int timezoneOffset;
extern bool wifiConnected;
extern unsigned long lastReconnectAttempt;
extern unsigned long lastRetryReset;
extern unsigned long lastWiFiCheck;
extern int reconnectRetries;
extern int wifiNetworkCount;
extern String wifiNetworks[MAX_NETWORKS];
extern int wifiRSSI[MAX_NETWORKS];
extern bool wifiSecurity[MAX_NETWORKS];

// Global variables - System Status
extern bool bluetoothConnected;
extern bool rtcInitialized;
extern bool sdCardInitialized;
extern String lastCommand;
extern unsigned long commandTimeout;
extern bool waitingForInput;
extern bool isFirstBoot;
extern String inputPrompt;

// Midnight caching variables
extern unsigned long lastMidnightCheck;
extern int lastCacheDay;
extern bool midnightCacheComplete;

// Buzzer enums and variables
enum BuzzerMode {
    BUZZER_OFF,
    BUZZER_PRAYER_TIME,    // On-off pattern for 10 seconds at prayer time
    BUZZER_WARNING,        // Continuous buzz for 1 second (10 minutes before)
    BUZZER_ALARM           // Custom alarm pattern
};

extern bool buzzerInitialized;
extern BuzzerMode currentBuzzerMode;

// WiFi Manager Functions
void loadWiFiCredentials();
void saveWiFiCredentials(const String& ssid, const String& password);
void clearWiFiCredentials();
bool connectToWiFi(const String& ssid, const String& password);
void checkWiFiConnection();
void autoReconnectWiFi();
void scanWiFiNetworks();
void displayWiFiNetworks();
String getSignalStrength(int rssi);

// Prayer Times Functions
void fetchPrayerTimes();
void fetchPrayerTimesForDays(int days);
void displayPrayerTimes();
void displayPrayerTimes(const String& jsonData, bool fromAPI = false);
bool loadPrayerTimesFromSD(const String& date);
bool loadPrayerTimesFromSD();
void savePrayerTimesToSD(const String& jsonData, const String& date);
void updateTimezoneFromAPI(const String& apiTimezone);
String getTimezoneAbbreviation(int offset);
void displayDate();
void displayClock();
void displayFajr();
void displayDhuhr();
void displayAsr();
void displayMaghrib();
void displayIsha();
String formatTime(const String& time24);
String getCurrentDateString();

// Time Manager Functions
void initializeRTC();
void syncTimeWithNTP();
void syncTimeWithNTP(bool forceSync);
void syncTimeWithNTP(int timezoneOffset, const String& timezone);
void updateRTCFromNTP();
String getCurrentTime();
String getCurrentDate();
void setSystemTime(int year, int month, int day, int hour, int minute, int second);
void showTime();
void showMenu();
String getSecurityType(bool isOpen);

// SD Manager Functions
void initializeSDCard();
bool writeFile(const String& path, const String& message);
String readFile(const String& path);
bool fileExists(const String& path);
void listDir(const String& dirname, uint8_t levels);
void createDir(const String& path);
void deleteFile(const String& path);
String loadPrayerDataFromSD(const String& filename);
void savePrayerTimesToSD(const String& jsonData, const String& date);

// Midnight Caching Functions
void checkMidnightCaching();
void performMidnightCache();

// Display Manager Functions
void initializeDisplay();
void updateDisplay();
void clearDisplay();
void displayWelcomeMessage();
void displaySystemStatus();
void displayCurrentInfo(DateTime now);
void displayPrayerAlert(const String& prayerName);
void displayWarningAlert(const String& prayerName, int minutesLeft);
void displayError(const String& errorMsg);

// Buzzer Manager Functions
void initializeBuzzer();
void updateBuzzer();
void testBuzzer();
void stopBuzzer();
void startPrayerTimeBuzzer(const String& prayerName);
void startPrayerWarningBuzzer(const String& prayerName);
void checkPrayerAlerts();
void checkPrayerTimeAlerts(DateTime now, const String& prayerTimesJson);
void handleBuzzerPattern(unsigned long currentMillis);
void handlePrayerTimeBuzzer(unsigned long elapsed);
void handleWarningBuzzer(unsigned long elapsed);
void handleAlarmBuzzer(unsigned long elapsed);

// Prayer Times Helper Functions
String getPrayerTimesFromCache(const String& dateKey);

// Debug Utils Functions
void debugPrint(const String& message);
void debugPrintln(const String& message);
void debugPrintf(const char* format, ...);

#endif // GLOBAL_H
/*
 * ESP32 Prayer Times Controller - Main Program
 * Modular Architecture with Numbered Menu System and Midnight Caching
 */

#include "global.h"

// Global variable definitions (extern in global.h)
String savedSSID = "";
String savedPassword = "";
String currentCity = DEFAULT_CITY;
String currentTimezone = DEFAULT_TIMEZONE;
int timezoneOffset = DEFAULT_TIMEZONE_OFFSET;
bool wifiConnected = false;
int reconnectRetries = 0;
unsigned long lastReconnectAttempt = 0;
int wifiNetworkCount = 0;

String wifiNetworks[MAX_NETWORKS];
int wifiRSSI[MAX_NETWORKS];
bool wifiSecurity[MAX_NETWORKS];

// System Status
bool bluetoothConnected = false;
bool rtcInitialized = false;
bool sdCardInitialized = false;
String lastCommand = "";
unsigned long commandTimeout = 0;
bool waitingForInput = false;
bool isFirstBoot = false;
String inputPrompt = "";

// Midnight caching variables
unsigned long lastMidnightCheck = 0;
int lastCacheDay = -1;
bool midnightCacheComplete = false;

// Buzzer variables
bool buzzerInitialized = false;
BuzzerMode currentBuzzerMode = BUZZER_OFF;

// Global Objects
BluetoothSerial SerialBT;
RTC_DS3231 rtc;
HTTPClient http;
Preferences preferences;

// Function declarations for main.cpp only functions
void initializeSystem();
void checkFirstBoot();
void handleFirstBootSetup();
void processBluetoothCommands();
void handleBluetoothCommand(const String& command);
void handleMenuSelection(int selection);
void showMainMenu();
void showStatus();
void showHelp();
void restartDevice();
void showTime();
void showMenu();
void checkMidnightCaching();
void performMidnightCache();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  debugPrintln("\n=== ESP32 Prayer Times Controller Starting ===");

  // Initialize all subsystems
  initializeSystem();
  
  // Check if this is the first boot
  checkFirstBoot();
  
  if (isFirstBoot) {
    handleFirstBootSetup();
  } else {
    // Load saved settings and try to reconnect
    loadWiFiCredentials();
    
    if (savedSSID.length() > 0) {
      connectToWiFi(savedSSID, savedPassword);
      if (wifiConnected) {
        syncTimeWithNTP();
        fetchPrayerTimes();
      }
    }
    
    showMainMenu();
  }
  
  debugPrintln("=== Setup Complete ===\n");
}

void loop() {
  // Check for midnight prayer times caching
  checkMidnightCaching();
  
  // Update display
  updateDisplay();
  
  // Update buzzer
  updateBuzzer();
  
  // Handle Bluetooth commands
  processBluetoothCommands();
  
  // Auto-reconnect WiFi if needed
  if (!wifiConnected && savedSSID.length() > 0) {
    autoReconnectWiFi();
  }
  
  // Clear command timeout
  if (waitingForInput && (millis() - commandTimeout > COMMAND_TIMEOUT)) {
    SerialBT.println(F("\nTimeout. Command cancelled."));
    waitingForInput = false;
    inputPrompt = "";
  }
  
  delay(100); // Prevent watchdog reset
}

void initializeSystem() {
  // Initialize RTC
  initializeRTC();
  
  // Initialize SD Card
  initializeSDCard();
  
  // Initialize Bluetooth
  if (SerialBT.begin(BLUETOOTH_NAME)) {
    bluetoothConnected = true;
    debugPrintln("Bluetooth initialized: " + String(BLUETOOTH_NAME));
  } else {
    debugPrintln("ERROR: Bluetooth initialization failed");
  }
  
  // Initialize Preferences
  preferences.begin("prayer_times", false);
  
  // Initialize display
  initializeDisplay();
  
  // Initialize buzzer
  initializeBuzzer();
  
  debugPrintln("System initialization complete");
}

void checkFirstBoot() {
  isFirstBoot = preferences.getBool("first_boot", true);
  if (isFirstBoot) {
    preferences.putBool("first_boot", false);
    debugPrintln("First boot detected");
  }
}

void handleFirstBootSetup() {
  SerialBT.println(F("\n========================"));
  SerialBT.println(F("ESP32 Prayer Times Controller"));
  SerialBT.println(F("First Boot Setup"));
  SerialBT.println(F("========================"));
  SerialBT.println(F("Welcome! This is the first boot."));
  SerialBT.println(F(""));
  SerialBT.println(F("Quick Setup Steps:"));
  SerialBT.println(F("1. Connect to WiFi (option 3 from main menu)"));
  SerialBT.println(F("2. Time will be synchronized automatically"));
  SerialBT.println(F("3. Prayer times will be downloaded & cached for 7 days"));
  SerialBT.println(F(""));
  SerialBT.println(F("Please select option 3 to configure WiFi first."));
  SerialBT.println(F("========================"));
}

void processBluetoothCommands() {
  if (SerialBT.available()) {
    String input = SerialBT.readStringUntil('\n');
    input.trim();
    
    if (input.length() == 0) return;
    
    debugPrintln("BT Command received: " + input);
    
    if (waitingForInput) {
      if (inputPrompt == "network_selection") {
        int networkIndex = input.toInt() - 1;
        if (networkIndex >= 0 && networkIndex < wifiNetworkCount) {
          String selectedSSID = wifiNetworks[networkIndex];
          SerialBT.println("Selected: " + selectedSSID);
          SerialBT.println("Enter password (or press enter if open network):");
          waitingForInput = true;
          inputPrompt = "wifi_password:" + selectedSSID;
          commandTimeout = millis();
        } else {
          SerialBT.println("Invalid selection. Please try again.");
          displayWiFiNetworks();
          waitingForInput = true;
          commandTimeout = millis();
        }
      } else if (inputPrompt.startsWith("wifi_password:")) {
        String ssid = inputPrompt.substring(14);
        String password = input;
        SerialBT.println("Connecting to " + ssid + "...");
        
        if (connectToWiFi(ssid, password)) {
          saveWiFiCredentials(ssid, password);
          syncTimeWithNTP();
          fetchPrayerTimes();
        }
        
        waitingForInput = false;
        inputPrompt = "";
      } else if (inputPrompt == "city_name") {
        currentCity = input;
        preferences.putString("city", currentCity);
        SerialBT.println("City changed to: " + currentCity);
        fetchPrayerTimes();
        waitingForInput = false;
        inputPrompt = "";
      }
    } else {
      handleBluetoothCommand(input);
    }
  }
}

void handleBluetoothCommand(const String& command) {
  // Convert to lowercase for consistency
  String cmd = command;
  cmd.toLowerCase();
  lastCommand = cmd;  // Store for debugging
  commandTimeout = millis();
  
  // Check if input is a number (menu selection)
  int cmdInt = cmd.toInt();
  if (cmd.length() <= 2 && cmdInt >= 1 && cmdInt <= 14) {
    handleMenuSelection(cmdInt);
    return;
  }
  
  // Check for text commands
  if (cmd == "menu") {
    showMenu();
    return;
  }
  
  // Invalid command
  SerialBT.println(F("Invalid command. Please enter a number 1-14."));
  SerialBT.println(F("Type 'menu' for options or '14' for help"));
}

void handleMenuSelection(int selection) {
  switch (selection) {
    case 1:
      showStatus();
      break;
    case 2:
      scanWiFiNetworks();
      displayWiFiNetworks();
      if (wifiNetworkCount > 0) {
        SerialBT.println("Enter network number (1-" + String(wifiNetworkCount) + "):");
        waitingForInput = true;
        inputPrompt = "network_selection";
        commandTimeout = millis();
      }
      break;
    case 3:
      scanWiFiNetworks();
      displayWiFiNetworks();
      break;
    case 4:
      if (savedSSID.length() > 0) {
        connectToWiFi(savedSSID, savedPassword);
        if (wifiConnected) {
          syncTimeWithNTP();
          fetchPrayerTimes();
        }
      } else {
        SerialBT.println(F("No saved credentials. Use option 2 to configure WiFi."));
      }
      break;
    case 5:
      WiFi.disconnect();
      wifiConnected = false;
      SerialBT.println(F("Disconnected from WiFi"));
      debugPrintln(F("WiFi manually disconnected"));
      break;
    case 6:
      WiFi.disconnect();
      clearWiFiCredentials();
      wifiConnected = false;
      SerialBT.println(F("WiFi credentials forgotten"));
      break;
    case 7:
      fetchPrayerTimes();
      break;
    case 8:
      showTime();
      break;
    case 9:
      SerialBT.println("Current city: " + currentCity);
      SerialBT.println("Enter new city name (or press enter to keep current):");
      waitingForInput = true;
      inputPrompt = "city_name";
      commandTimeout = millis();
      break;
    case 10:
      syncTimeWithNTP();
      break;
    case 11:
      SerialBT.println(F("Testing display system..."));
      displaySystemStatus();
      clearDisplay();
      displayWelcomeMessage();
      break;
    case 12:
      SerialBT.println(F("Testing buzzer system..."));
      testBuzzer();
      break;
    case 13:
      restartDevice();  // This will restart the device
      break;
    case 14:
      showHelp();
      break;
    default:
      SerialBT.println(F("Invalid selection. Please choose 1-14."));
      break;
  }
}

void showMenu() {
  SerialBT.println(F("\n=== ESP32 Prayer Times Controller ==="));
  SerialBT.println(F("Select an option (1-14):"));
  SerialBT.println(F("1.  Show system status"));
  SerialBT.println(F("2.  Setup WiFi connection"));
  SerialBT.println(F("3.  Scan WiFi networks"));
  SerialBT.println(F("4.  Connect using saved WiFi"));
  SerialBT.println(F("5.  Disconnect from WiFi"));
  SerialBT.println(F("6.  Forget saved WiFi"));
  SerialBT.println(F("7.  Show prayer times"));
  SerialBT.println(F("8.  Show current time"));
  SerialBT.println(F("9.  Change city"));
  SerialBT.println(F("10. Sync time with NTP"));
  SerialBT.println(F("11. Test display"));
  SerialBT.println(F("12. Test buzzer"));
  SerialBT.println(F("13. Restart device"));
  SerialBT.println(F("14. Show detailed help"));
  SerialBT.println(F("===================================="));
  SerialBT.println(F("Enter your choice (1-14):\n"));
}

void showMainMenu() {
  showMenu();
}

void showStatus() {
  SerialBT.println(F("\n=== System Status ==="));
  
  // WiFi Status
  SerialBT.print(F("WiFi Status: "));
  SerialBT.println(wifiConnected ? F("Connected") : F("Disconnected"));
  if (wifiConnected) {
    SerialBT.print(F("SSID: "));
    SerialBT.println(WiFi.SSID());
    SerialBT.print(F("IP Address: "));
    SerialBT.println(WiFi.localIP().toString());
    SerialBT.print(F("Signal Strength: "));
    SerialBT.print(getSignalStrength(WiFi.RSSI()));
    SerialBT.print(F(" ("));
    SerialBT.print(WiFi.RSSI());
    SerialBT.println(F(" dBm)"));
  } else if (savedSSID.length() > 0) {
    SerialBT.print(F("Saved SSID: "));
    SerialBT.println(savedSSID);
    SerialBT.print(F("Reconnect attempts: "));
    SerialBT.print(reconnectRetries);
    SerialBT.print(F("/"));
    SerialBT.println(MAX_RETRIES);
  }
  
  // Bluetooth Status
  SerialBT.print(F("Bluetooth: Active ("));
  SerialBT.print(BLUETOOTH_NAME);
  SerialBT.println(F(")"));
  
  // RTC Status
  SerialBT.print(F("RTC DS3231: "));
  SerialBT.println(rtcInitialized ? F("Connected") : F("Not found"));
  
  // SD Card Status
  SerialBT.print(F("SD Card: "));
  SerialBT.println(sdCardInitialized ? F("Mounted") : F("Not found"));
  
  // Current Settings
  SerialBT.print(F("City: "));
  SerialBT.println(currentCity);
  SerialBT.print(F("Timezone: "));
  SerialBT.print(currentTimezone);
  SerialBT.print(F(" (GMT+"));
  SerialBT.print(timezoneOffset);
  SerialBT.println(F(")"));
  
  // Current Time
  if (rtcInitialized) {
    SerialBT.print(F("Current Time: "));
    SerialBT.println(getCurrentTime());
  }
  
  // Memory info
  SerialBT.print(F("Free Heap: "));
  SerialBT.print(ESP.getFreeHeap());
  SerialBT.println(F(" bytes"));
  SerialBT.println(F("===============\n"));
}

void showTime() {
  String timeStr = getCurrentTime();
  SerialBT.print(F("Current Time: "));
  SerialBT.println(timeStr);
}

void showHelp() {
  SerialBT.println(F("\n=== Command Help ==="));
  SerialBT.println(F("Enter a number (1-14) to select an option:"));
  SerialBT.println(F(""));
  SerialBT.println(F("1  - Show main menu"));
  SerialBT.println(F("2  - Show system status"));
  SerialBT.println(F("3  - Setup WiFi connection"));
  SerialBT.println(F("4  - Scan WiFi networks"));
  SerialBT.println(F("5  - Connect using saved WiFi"));
  SerialBT.println(F("6  - Disconnect from WiFi"));
  SerialBT.println(F("7  - Clear saved WiFi credentials"));
  SerialBT.println(F("8  - Display today's prayer times"));
  SerialBT.println(F("9  - Show current time from RTC"));
  SerialBT.println(F("10 - Change city for prayer times"));
  SerialBT.println(F("11 - Force NTP time sync and update RTC"));
  SerialBT.println(F("12 - Reboot ESP32"));
  SerialBT.println(F("13 - Show this help"));
  SerialBT.println(F(""));
  SerialBT.println(F("TIP: After each command, you'll return to the main menu!"));
  SerialBT.println(F("🌙 Midnight auto-caching: Prayer times are automatically"));
  SerialBT.println(F("   cached every night at midnight for 8 days ahead!"));
  SerialBT.println(F("====================\n"));
}

void restartDevice() {
  SerialBT.println(F("Restarting device in 3 seconds..."));
  debugPrintln(F("Device restart requested"));
  delay(3000);
  ESP.restart();
}

void checkMidnightCaching() {
  // Only check every 30 seconds to avoid excessive checking
  if (millis() - lastMidnightCheck < 30000) {
    return;
  }
  
  lastMidnightCheck = millis();
  
  if (!rtcInitialized || !wifiConnected) {
    return; // Need both RTC and WiFi for caching
  }
  
  DateTime now = rtc.now();
  int currentDay = now.day();
  int currentHour = now.hour();
  int currentMinute = now.minute();
  
  // Check if it's midnight (00:00 to 00:05) and we haven't cached today yet
  if (currentHour == 0 && currentMinute < 5) {
    if (currentDay != lastCacheDay || !midnightCacheComplete) {
      debugPrintln(F("Midnight detected - starting prayer times cache"));
      SerialBT.println(F("🌙 Midnight auto-cache starting..."));
      performMidnightCache();
      lastCacheDay = currentDay;
      midnightCacheComplete = true;
    }
  } else if (currentHour > 0) {
    // Reset the flag after midnight window
    midnightCacheComplete = false;
  }
}

void performMidnightCache() {
  if (!wifiConnected || !rtcInitialized) {
    debugPrintln(F("Cannot perform midnight cache - missing WiFi or RTC"));
    return;
  }
  
  debugPrintln(F("Starting midnight prayer times caching for 8 days (today + 7 ahead)"));
  SerialBT.println(F("📦 Caching prayer times for 8 days..."));
  
  DateTime now = rtc.now();
  int cachedCount = 0;
  int skippedCount = 0;
  
  // Cache for today (day 0) plus next 7 days (days 1-7)
  for (int i = 0; i <= 7; i++) {
    DateTime targetDate = DateTime(now.unixtime() + (i * 86400L)); // Add i days
    
    // Format date as DD-MM-YYYY
    char dateStr[12];
    sprintf(dateStr, "%02d-%02d-%04d", targetDate.day(), targetDate.month(), targetDate.year());
    
    // Create file path with proper month/year handling
    char filePath[100];
    sprintf(filePath, "/%s/%04d/%02d/%s.json", 
            currentCity.c_str(), targetDate.year(), targetDate.month(), dateStr);
    
    // Check if file already exists
    if (fileExists(String(filePath))) {
      debugPrintln("Skipping " + String(dateStr) + " - already cached");
      skippedCount++;
      continue;
    }
    
    // Fetch prayer times for this date
    String url = String(ALADHAN_API_BASE) + "/" + String(dateStr) + 
                "?city=" + currentCity + 
                "&country=" + String(DEFAULT_COUNTRY) + 
                "&method=" + String(PRAYER_METHOD);
    
    debugPrintln("Midnight caching: " + String(dateStr));
    
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      // Create directory structure if it doesn't exist
      String dirPath = "/" + currentCity + "/" + String(targetDate.year()) + "/" + String(targetDate.month());
      createDir(dirPath);
      
      // Save to SD card
      savePrayerTimesToSD(payload, String(dateStr));
      cachedCount++;
      debugPrintln("Cached prayer times for " + String(dateStr));
    } else {
      debugPrintln("Failed to cache " + String(dateStr) + " - HTTP " + String(httpCode));
    }
    
    http.end();
    delay(1000); // Rate limiting between API calls
  }
  
  // Report results
  String resultMsg = "🌙 Midnight cache complete: " + String(cachedCount) + " new, " + String(skippedCount) + " skipped";
  SerialBT.println(resultMsg);
  debugPrintln("Midnight caching completed: " + String(cachedCount) + " days cached, " + String(skippedCount) + " days skipped");
}
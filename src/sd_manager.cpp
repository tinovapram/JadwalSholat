/*
 * SD Card Management Module Implementation
 */

#include "global.h"

void initializeSDCard() {
  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  
  if (SD.begin(SD_CS_PIN)) {
    sdCardInitialized = true;
    debugPrintln("SD Card initialized successfully");
    
    // Check card type
    uint8_t cardType = SD.cardType();
    if (cardType != CARD_NONE) {
      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      debugPrintln("SD Card Size: " + String((uint32_t)cardSize) + " MB");
      SerialBT.println("SD Card ready (" + String((uint32_t)cardSize) + " MB)");
    }
  } else {
    sdCardInitialized = false;
    debugPrintln("ERROR: Could not initialize SD Card");
    SerialBT.println("SD Card initialization failed");
  }
}

String createSDCardPath(const String& city, const String& date) {
  // Extract year, month, day from date (format: DD-MM-YYYY)
  int firstDash = date.indexOf('-');
  int secondDash = date.lastIndexOf('-');
  
  String day = date.substring(0, firstDash);
  String month = date.substring(firstDash + 1, secondDash);
  String year = date.substring(secondDash + 1);
  
  String path = "/" + city + "/" + year + "/" + month;
  return path;
}

void savePrayerTimesToSD(const String& jsonResponse) {
  if (!sdCardInitialized) {
    debugPrintln("SD Card not available for saving");
    return;
  }
  
  // Parse date from JSON to create proper path
  JsonDocument tempDoc;
  deserializeJson(tempDoc, jsonResponse);
  JsonObject data = tempDoc["data"];
  JsonObject date = data["date"];
  JsonObject gregorian = date["gregorian"];
  
  String dateStr = gregorian["date"]; // Format: DD-MM-YYYY
  String path = createSDCardPath(currentCity, dateStr);
  
  debugPrintln("Creating SD card path: " + path);
  
  // Create directory structure
  if (!SD.exists(path)) {
    String currentPath = "";
    int start = 1; // Skip the leading slash
    int slashIndex;
    
    while ((slashIndex = path.indexOf('/', start)) != -1) {
      currentPath += "/" + path.substring(start, slashIndex);
      if (!SD.exists(currentPath)) {
        if (!SD.mkdir(currentPath)) {
          debugPrintln("Failed to create directory: " + currentPath);
          return;
        }
        debugPrintln("Created directory: " + currentPath);
      }
      start = slashIndex + 1;
    }
    
    currentPath = path;
    if (!SD.exists(currentPath)) {
      if (!SD.mkdir(currentPath)) {
        debugPrintln("Failed to create final directory: " + currentPath);
        return;
      }
      debugPrintln("Created directory: " + currentPath);
    }
  }
  
  // Create filename with date
  String day = dateStr.substring(0, dateStr.indexOf('-'));
  String filename = path + "/" + day + ".json";
  
  debugPrintln("Saving prayer times to: " + filename);
  
  // Write JSON file
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.print(jsonResponse);
    file.close();
    debugPrintln("Prayer times saved successfully to SD card");
    SerialBT.println("Prayer times saved to SD card: " + filename);
  } else {
    debugPrintln("Failed to write to SD card file: " + filename);
    SerialBT.println("Failed to save prayer times to SD card");
  }
}

String loadPrayerDataFromSD(const String& filename) {
  if (!sdCardInitialized) {
    return "";
  }
  
  if (!SD.exists(filename)) {
    debugPrintln("Prayer data file not found: " + filename);
    return "";
  }
  
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    debugPrintln("Failed to open prayer data file: " + filename);
    return "";
  }
  
  String data = file.readString();
  file.close();
  
  debugPrintln("Loaded prayer data from SD card: " + filename);
  return data;
}

void savePrayerTimesToSD(const String& jsonData, const String& date) {
  if (!sdCardInitialized) {
    debugPrintln("SD card not initialized, cannot save prayer times");
    return;
  }
  
  // Parse original JSON
  JsonDocument originalDoc;
  DeserializationError error = deserializeJson(originalDoc, jsonData);
  
  if (error) {
    debugPrintln("Error parsing JSON for filtering: " + String(error.c_str()));
    return;
  }
  
  // Create filtered JSON with only required fields
  JsonDocument filteredDoc;
  
  // Copy basic structure
  filteredDoc["code"] = originalDoc["code"];
  filteredDoc["status"] = originalDoc["status"];
  
  // Create data object
  JsonObject data = filteredDoc["data"].to<JsonObject>();
  
  // Only save timings
  if (originalDoc["data"]["timings"]) {
    data["timings"] = originalDoc["data"]["timings"];
  }
  
  // Only save date.readable and date.timestamp
  if (originalDoc["data"]["date"]) {
    JsonObject dateObj = data["date"].to<JsonObject>();
    dateObj["readable"] = originalDoc["data"]["date"]["readable"];
    dateObj["timestamp"] = originalDoc["data"]["date"]["timestamp"];
  }
  
  // Only save meta.timezone
  if (originalDoc["data"]["meta"]) {
    JsonObject metaObj = data["meta"].to<JsonObject>();
    metaObj["timezone"] = originalDoc["data"]["meta"]["timezone"];
  }
  
  // Convert filtered JSON to string
  String filteredJsonString;
  serializeJson(filteredDoc, filteredJsonString);
  
  // Create directory structure: /city/year/month/
  String year = date.substring(6, 10);
  String month = date.substring(3, 5);
  
  String baseDir = "/" + currentCity;
  String yearDir = baseDir + "/" + year;
  String monthDir = yearDir + "/" + month;
  
  createDir(baseDir);
  createDir(yearDir);
  createDir(monthDir);
  
  // Save filtered file as /city/year/month/dd-mm-yyyy.json
  String filePath = monthDir + "/" + date + ".json";
  
  if (writeFile(filePath, filteredJsonString)) {
    debugPrintln("Filtered prayer times saved to SD: " + filePath);
    debugPrintln("Saved fields: timings, date.readable, date.timestamp, meta.timezone");
  } else {
    debugPrintln("Failed to save filtered prayer times to SD: " + filePath);
  }
}

// Utility functions for file operations
bool writeFile(const String& path, const String& message) {
  if (!sdCardInitialized) {
    return false;
  }

  File file = SD.open(path.c_str(), FILE_WRITE);
  if (!file) {
    debugPrintln("Failed to open file for writing: " + path);
    return false;
  }

  if (file.print(message)) {
    debugPrintln("File written: " + path);
    file.close();
    return true;
  } else {
    debugPrintln("Write failed: " + path);
    file.close();
    return false;
  }
}

String readFile(const String& path) {
  if (!sdCardInitialized) {
    return "";
  }

  File file = SD.open(path.c_str());
  if (!file) {
    debugPrintln("Failed to open file for reading: " + path);
    return "";
  }

  String result = file.readString();
  file.close();
  return result;
}

bool fileExists(const String& path) {
  if (!sdCardInitialized) {
    return false;
  }
  return SD.exists(path.c_str());
}

void createDir(const String& path) {
  if (!sdCardInitialized) {
    return;
  }
  
  if (!SD.exists(path.c_str())) {
    if (SD.mkdir(path.c_str())) {
      debugPrintln("Directory created: " + path);
    } else {
      debugPrintln("Failed to create directory: " + path);
    }
  }
}

void deleteFile(const String& path) {
  if (!sdCardInitialized) {
    return;
  }
  
  if (SD.remove(path.c_str())) {
    debugPrintln("File deleted: " + path);
  } else {
    debugPrintln("Failed to delete file: " + path);
  }
}

void listDir(const String& dirname, uint8_t levels) {
  if (!sdCardInitialized) {
    return;
  }

  File root = SD.open(dirname.c_str());
  if (!root) {
    debugPrintln("Failed to open directory: " + dirname);
    return;
  }
  
  if (!root.isDirectory()) {
    debugPrintln("Not a directory: " + dirname);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      debugPrintln("DIR: " + String(file.name()));
      if (levels) {
        listDir(String(file.name()), levels - 1);
      }
    } else {
      debugPrintln("FILE: " + String(file.name()) + " SIZE: " + String(file.size()));
    }
    file = root.openNextFile();
  }
}
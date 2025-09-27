# 🕌 ESP32 Prayer Times Controller

![ESP32](https://img.shields.io/badge/Platform-ESP32-blue) ![Arduino](https://img.shields.io/badge/Framework-Arduino-00979D) ![License](https://img.shields.io/badge/License-MIT-green) ![Version](https://img.shields.io/badge/Version-2.0-orange)

A comprehensive Islamic prayer times controller for ESP32 with automatic scheduling, display output, and audio alerts. Features real-time clock synchronization, WiFi connectivity, SD card caching, and Bluetooth configuration interface.

## ✨ Features

### 🕰️ **Prayer Times Management**
- **5 Daily Prayers**: Fajr, Dhuhr, Asr, Maghrib, Isha
- **Automatic API Integration**: Aladhan.com API with Indonesian Kemenag method
- **Offline Capability**: SD card caching for 8 days ahead
- **Midnight Auto-Sync**: Automatically caches next 7 days at midnight
- **Multi-City Support**: Easy city switching with timezone handling

### 📱 **Smart Interface**
- **Bluetooth Configuration**: Full system setup via smartphone
- **Streamlined Menu**: 14 intuitive options (no redundancy)
- **Real-time Display**: Clock, date, and prayer times
- **System Status**: WiFi, RTC, and SD card monitoring

### 🔊 **Audio Alerts**
- **10-Minute Warnings**: Gentle 1-second buzz before each prayer
- **Prayer Time Alerts**: 10-second on/off pattern at exact prayer time
- **Smart Scheduling**: Automatic daily reset, no duplicate alerts
- **Configurable Hardware**: GPIO 23 default (customizable)

### 🖥️ **Display Support**
- **Multiple Hardware Types**: LCD (16x2, 20x4), OLED (128x64), TFT, LED Matrix
- **Real-time Updates**: Time, date, city, timezone display
- **Prayer Notifications**: Special alerts during prayer times
- **Hardware Agnostic**: Easy integration with any display type

### 💾 **Data Management**
- **Efficient Storage**: Filtered JSON saves only required fields (~60% reduction)
- **Smart Caching**: Skip existing files, handle month boundaries
- **Structured Organization**: `/city/year/month/date.json` hierarchy
- **Data Integrity**: Automatic validation and error handling

## 🚀 Quick Start

### **Hardware Requirements**
- ESP32 Development Board
- DS3231 RTC Module (I2C)
- SD Card Module (SPI)
- Buzzer (Active/Passive)
- Display (Optional: LCD/OLED/TFT)

### **Pin Configuration**
```cpp
// RTC (I2C)
#define RTC_SDA_PIN 21
#define RTC_SCL_PIN 22

// SD Card (SPI)
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18

// Buzzer
#define BUZZER_PIN 23
```

### **Installation**
1. **Clone Repository**:
   ```bash
   git clone https://github.com/tinovapram/JadwalSholat.git
   cd JadwalSholat
   ```

2. **Install PlatformIO**:
   ```bash
   pip install platformio
   ```

3. **Build & Upload**:
   ```bash
   pio run -t upload
   ```

4. **Monitor Serial**:
   ```bash
   pio device monitor
   ```

## 📋 Usage Guide

### **First Boot Setup**
1. **Connect via Bluetooth**: Pair with "ESP32_Prayer_Times"
2. **Configure WiFi**: Use menu option 2 to scan and connect
3. **Set Location**: Use menu option 9 to change city
4. **Sync Time**: Automatic NTP synchronization after WiFi connection

### **Menu Commands**
| Option | Function | Description |
|--------|----------|-------------|
| 1 | System Status | WiFi, RTC, SD card status |
| 2 | WiFi Setup | Scan networks and connect |
| 3 | WiFi Scan | Show available networks |
| 4 | Saved WiFi | Connect using stored credentials |
| 5 | WiFi Disconnect | Disconnect from current network |
| 6 | Forget WiFi | Clear stored credentials |
| 7 | Prayer Times | Show today's prayer schedule |
| 8 | Current Time | Display current time/date |
| 9 | Change City | Update location for prayer times |
| 10 | Sync NTP | Force time synchronization |
| 11 | Test Display | Demo display functionality |
| 12 | Test Buzzer | Demo buzzer patterns |
| 13 | Restart | System restart |
| 14 | Help | Detailed command help |

### **Text Commands**
- `menu` - Show main menu anytime
- `1-14` - Direct menu selection

## 🏗️ Architecture

### **Modular Design**
```
├── include/
│   ├── config.h      # Hardware & API configuration
│   └── global.h      # Global variables & declarations
├── src/
│   ├── main.cpp      # Main program & menu system
│   ├── prayer_times.cpp  # API communication & parsing
│   ├── wifi_manager.cpp  # Network connectivity
│   ├── sd_manager.cpp    # File system operations
│   ├── time_manager.cpp  # RTC & NTP synchronization
│   ├── display_manager.cpp # Display output control
│   ├── buzzer_manager.cpp  # Audio alert system
│   └── debug_utils.cpp    # Logging & diagnostics
```

### **Memory Optimization**
- **RAM Usage**: 18.5% (60,692 bytes) - Excellent efficiency
- **Flash Usage**: 54.7% (1,719,397 bytes) - Well optimized
- **F() Macro**: Static strings stored in flash memory
- **Filtered JSON**: Only essential data cached to SD card

### **Smart Features**
- **Midnight Caching**: Automatically downloads next 7 days
- **Month Boundary Handling**: Seamlessly handles date transitions
- **Duplicate Prevention**: Avoids unnecessary API calls
- **Error Recovery**: Graceful handling of network/hardware issues

## ⚙️ Configuration

### **WiFi & Network**
```cpp
#define MAX_NETWORKS 20
#define WIFI_TIMEOUT 20000
#define HTTP_TIMEOUT 10000
#define RECONNECT_INTERVAL 30000
```

### **Prayer Times API**
```cpp
#define ALADHAN_API_BASE "http://api.aladhan.com/v1/timingsByCity"
#define PRAYER_METHOD 20  // Kemenag Indonesia
#define DEFAULT_CITY "Nganjuk"
#define DEFAULT_COUNTRY "Indonesia"
#define DEFAULT_TIMEZONE "Asia/Jakarta"
```

### **Display Settings**
```cpp
#define DISPLAY_UPDATE_INTERVAL 1000  // Update every second
#define DISPLAY_ENABLED true
```

### **Buzzer Configuration**
```cpp
#define BUZZER_PIN 23
#define PRAYER_WARNING_MINUTES 10
#define PRAYER_ALERT_DURATION 10000  // 10 seconds
#define WARNING_BUZZ_DURATION 1000   // 1 second
```

## 🔧 Hardware Integration

### **Display Integration**
The system supports multiple display types with hardware abstraction:

#### **LCD Display (16x2 / 20x4)**
```cpp
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

void initializeDisplay() {
    lcd.begin(16, 2);
    lcd.backlight();
}
```

#### **OLED Display (128x64)**
```cpp
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void initializeDisplay() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}
```

#### **TFT Display**
```cpp
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void initializeDisplay() {
    tft.init();
    tft.setRotation(1);
}
```

### **Buzzer Types**
- **Active Buzzer**: Simple on/off control
- **Passive Buzzer**: Tone generation with frequency control
- **PWM Control**: Variable volume/intensity

## 📊 Performance Metrics

### **Memory Usage**
- **RAM**: 60,692 / 327,680 bytes (18.5%) ✅ Excellent
- **Flash**: 1,719,397 / 3,145,728 bytes (54.7%) ✅ Good
- **Free Heap**: ~267KB available for operations

### **Timing Performance**
- **Prayer Time Check**: Every second (minimal CPU impact)
- **Display Update**: Every second (configurable)
- **Midnight Cache**: Once daily (automated)
- **WiFi Reconnect**: Every 30 seconds when disconnected

### **Storage Efficiency**
- **Original JSON**: ~2-3KB per day
- **Filtered JSON**: ~800-1200 bytes per day (~60% reduction)
- **8-Day Cache**: ~6-10KB total storage

## 🛠️ Development

### **Build Requirements**
- PlatformIO Core 6.0+
- ESP32 Arduino Framework 2.0.14
- Libraries: ArduinoJson, RTClib, BluetoothSerial

### **Development Setup**
```bash
# Install PlatformIO
pip install platformio

# Initialize project (if starting fresh)
pio project init --board esp32dev

# Install dependencies
pio lib install "bblanchon/ArduinoJson@^7.4.2"
pio lib install "adafruit/RTClib@^2.1.4"

# Build project
pio run

# Upload firmware
pio run -t upload

# Monitor output
pio device monitor
```

### **Testing**
```bash
# Compile only
pio run

# Upload and monitor
pio run -t upload -t monitor

# Clean build
pio run -t clean
```

## 🐛 Troubleshooting

### **Common Issues**

#### **WiFi Connection Problems**
- Check SSID/password accuracy
- Verify network compatibility (2.4GHz only)
- Monitor signal strength (use WiFi scan)

#### **RTC Sync Issues**
- Verify DS3231 wiring (SDA/SCL pins)
- Check I2C pull-up resistors (4.7kΩ)
- Ensure RTC battery is installed

#### **SD Card Problems**
- Format SD card as FAT32
- Check SPI wiring connections
- Verify CS pin configuration

#### **Prayer Times Not Loading**
- Ensure internet connectivity
- Check Aladhan API status
- Verify city name spelling
- Monitor API response in serial output

### **Debug Mode**
Enable detailed logging by setting:
```cpp
#define DEBUG_ENABLED true
```

## 📈 Future Enhancements

### **Planned Features**
- [ ] **Web Interface**: Browser-based configuration
- [ ] **Multiple Locations**: Support for multiple cities
- [ ] **Custom Prayer Methods**: User-defined calculation methods
- [ ] **Qibla Direction**: Compass integration
- [ ] **Prayer Reminders**: Customizable alert schedules
- [ ] **Statistics**: Prayer time tracking and analytics

### **Hardware Expansions**
- [ ] **GPS Module**: Automatic location detection
- [ ] **Temperature Sensor**: Environmental monitoring
- [ ] **Motion Sensor**: Presence detection
- [ ] **Speaker Module**: Adhan playback
- [ ] **LED Strip**: Visual prayer time indicators

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 👥 Authors

- **Tinova Pram** - *Initial work* - [tinovapram](https://github.com/tinovapram)

## 🙏 Acknowledgments

- [Aladhan API](https://aladhan.com/prayer-times-api) for prayer times data
- [ArduinoJson](https://arduinojson.org/) for efficient JSON processing
- [RTClib](https://github.com/adafruit/RTClib) for real-time clock support
- ESP32 community for extensive documentation and examples

## 📞 Support

For support and questions:
- Open an [Issue](https://github.com/tinovapram/JadwalSholat/issues)
- Check [Discussions](https://github.com/tinovapram/JadwalSholat/discussions)
- Review [Wiki](https://github.com/tinovapram/JadwalSholat/wiki) for detailed guides

---

**⭐ Star this repository if it helps you in your Islamic journey! ⭐**
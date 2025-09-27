/*
 * Configuration Header for ESP32 Prayer Times Controller
 * Contains all constants, definitions, and configuration parameters
 */

#ifndef CONFIG_H
#define CONFIG_H


// Hardware Pin Definitions
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18
#define RTC_SDA_PIN 21
#define RTC_SCL_PIN 22

// Network Configuration
#define MAX_NETWORKS 20
#define MAX_RETRIES 5
#define WIFI_TIMEOUT 20000
#define BT_TIMEOUT 30000
#define RETRY_RESET_INTERVAL 300000  // 5 minutes
#define WIFI_CHECK_INTERVAL 5000
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 63
#define HTTP_TIMEOUT 10000
#define RECONNECT_INTERVAL 30000

// NTP Configuration
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define NTP_SERVER3 "time.google.com"
#define NTP_TIMEOUT 15000
#define NTP_SYNC_ATTEMPTS 15

// API Configuration
#define ALADHAN_API_BASE "http://api.aladhan.com/v1/timingsByCity"
#define PRAYER_METHOD 20  // Kemenag Indonesia
#define DEFAULT_CITY "Nganjuk"
#define DEFAULT_COUNTRY "Indonesia"
#define DEFAULT_TIMEZONE "Asia/Jakarta"
#define DEFAULT_TIMEZONE_OFFSET 7

// SD Card Configuration
#define SD_MOUNT_POINT "/sd"
#define PRAYER_DATA_DIR "/prayer_times"
#define MAX_FILE_SIZE 8192
#define PRAYER_CACHE_DAYS 7

// Debug Configuration
#define DEBUG_ENABLED true
#define SERIAL_BAUD_RATE 115200
#define DEBUG_PREFIX "[DEBUG] "

// System Limits
#define MAX_JSON_SIZE 4096
#define MAX_COMMAND_LENGTH 64
#define MAX_CITY_NAME_LENGTH 32
#define MAX_TIMEZONE_LENGTH 32

// Display Configuration
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8

// Prayer Times Display Format
#define TIME_FORMAT_12H true
#define SHOW_SECONDS false
#define DATE_FORMAT_DMY true

// Bluetooth Configuration
#define BT_DEVICE_NAME "Jadwal sholat"
#define BLUETOOTH_NAME "Jadwal sholat"
#define BT_PIN "1234"
#define COMMAND_TIMEOUT 30000

// WiFi Auto-reconnect Settings
#define AUTO_RECONNECT_ENABLED true
#define RECONNECT_DELAY 5000
#define MAX_RECONNECT_ATTEMPTS 3

// Power Management
#define DEEP_SLEEP_ENABLED false
#define LIGHT_SLEEP_ENABLED false
#define CPU_FREQ_MHZ 240

// Memory Management
#define STACK_SIZE 8192
#define HEAP_SIZE 32768

// Display Configuration
#define DISPLAY_UPDATE_INTERVAL 1000  // Update every second
#define DISPLAY_ENABLED true

// Buzzer Configuration
#define BUZZER_PIN 23  // GPIO pin for buzzer
#define BUZZER_CHECK_INTERVAL 1000  // Check every second
#define PRAYER_WARNING_MINUTES 10   // Warning before prayer time
#define PRAYER_ALERT_DURATION 10000 // 10 seconds on-off pattern
#define WARNING_BUZZ_DURATION 1000  // 1 second continuous buzz

// Error Codes
#define ERROR_WIFI_CONNECTION -1
#define ERROR_API_REQUEST -2
#define ERROR_JSON_PARSE -3
#define ERROR_SD_CARD -4
#define ERROR_RTC_INIT -5
#define ERROR_NTP_SYNC -6

// Success Codes
#define SUCCESS 0
#define SUCCESS_CACHED 1
#define SUCCESS_OFFLINE 2

#endif // CONFIG_H
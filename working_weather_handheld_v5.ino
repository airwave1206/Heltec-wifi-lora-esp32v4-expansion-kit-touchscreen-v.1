// hand held weather station 
// working sensors, gps for time, and forecast.
// Jim Hanna 10-7-2026
#include "Arduino.h"
#include <lvgl.h>
#include "HT_st7789spi.h"
#include "chsc6x.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "SparkFun_SHTC3.h" 
#include "HT_TinyGPS++.h" 
// --- INGEST EEZ FLOW ENGINE MAPS ---
#include "ui.h"
#include "vars.h"
// --- USER TIME ZONE CONFIGURATION ---
const float TIME_ZONE_OFFSET = -5.0; // Change to your local offset (e.g., -5 for EST)
// --- POWER & BUS CONFIGURATIONS ---
#define VEXT_PIN            36   
// Dedicated Weather I2C Bus (B2B Expansion Kit)
#define B2B_SDA              4   
#define B2B_SCL              3   
// HELTEC V4 HARDWIRED L76K GPS PINMAP (UPDATED)
#define GPS_RX_PIN          39   // ESP32 RX <- GPS TX
#define GPS_TX_PIN          38   // ESP32 TX -> GPS RX
#define PIN_GPS_EN          34   // GPS Power Gate (Active LOW)
#define PIN_GPS_STANDBY     40   // Standby Pin (HIGH to force wake)
#define PIN_GPS_RESET       42   // Reset Pin (HIGH to run)
#define GPS_BAUDRATE        9600 
// Replace standard HardwareSerial with your working instance alias matching the test code
#define Serial1             GPSSerial
HardwareSerial GPSSerial(1); 
// Dedicated Touchscreen Bus Pins
#define TOUCH_SDA_PIN       47   
#define TOUCH_SCL_PIN       48   
#define TOUCH_INT_PIN       45   
#define TOUCH_RST_PIN       44   
// TFT SPI Display Pin Definition
#define TFT_CS              15
#define TFT_RST             18 
#define TFT_DC              16
#define TFT_MOSI            33  
#define TFT_SCLK            17  
#define TFT_BLK             21  
// Peripherals
#define WAKE_BUTTON_PIN     35   
#define BUZZER_PIN           6   
// --- DRIVER CONSTRUCTORS ---
chsc6x touch(&Wire1, TOUCH_SDA_PIN, TOUCH_SCL_PIN, TOUCH_INT_PIN, TOUCH_RST_PIN);
HT_ST7789 tft(240, 320, TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_BME280 bme; 
SHTC3 myGXHTC;                  
TinyGPSPlus gps;
// --- STATE MANAGEMENT ---
unsigned long lastActivityTime = 0;
const unsigned long SCREEN_TIMEOUT_MS = 5 * 60 * 1000; 
bool isScreenAwake = true;
volatile bool wakeRequested = false;
#define LOG_SIZE 6
float pressureHistory[LOG_SIZE] = {0.0};
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL = 10 * 60 * 1000; // 10 minutes tracking interval
const char* current_forecast = "Calibrating";
unsigned long lastDebugPrint = 0;
unsigned long lastSensorReadTime = 0; // NEW: Throttling timer for sensor health
const unsigned long SENSOR_READ_INTERVAL = 1500; // Read sensors comfortably every 1.5 seconds
void IRAM_ATTR handleWakeButton() {
    wakeRequested = true;
}
// --- DISPLAY FLUSH ---
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);  
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((uint16_t *)px_map, w * h);
    tft.endWrite();
    lv_display_flush_ready(disp);
}
// --- CLEAN CONFLICT-FREE TOUCHPAD READ ---
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t rawX = 0, rawY = 0;

    if (touch.chsc6x_read_touch_info(&rawX, &rawY) == 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        
        // Match landscape orientation constraints cleanly
        data->point.x = rawY;          
        data->point.y = rawX;    
        
        lastActivityTime = millis(); 
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
void setBacklight(bool state) {
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, state ? HIGH : LOW);
    isScreenAwake = state;
}
void playChirp(unsigned int frequency, unsigned long duration) {
    tone(BUZZER_PIN, frequency, duration);
    delay(duration); 
    noTone(BUZZER_PIN); 
}
void soundBadWeatherAlert() {
    for(int i = 0; i < 3; i++) {
        playChirp(1800, 150);
        delay(50);
        playChirp(1500, 200);
        delay(200);
    }
}
// --- SHORT TERM FORECASTING ENGINE ---
// --- RE-BUILT SHORT TERM FORECASTING ENGINE ---
void computeZambrettiForecast(float currentBarohPa) {
    unsigned long now = millis();
    
    // Initialize the history cleanly on the very first loop pass
    if (lastLogTime == 0) {
        for (int i = 0; i < LOG_SIZE; i++) {
            pressureHistory[i] = currentBarohPa;
        }
        lastLogTime = now;
        current_forecast = "Calibrating";
        return;
    }
    
    // Append to array cleanly every 10 minutes
    if (now - lastLogTime >= LOG_INTERVAL) {
        for (int i = LOG_SIZE - 1; i > 0; i--) {
            pressureHistory[i] = pressureHistory[i - 1];
        }
        pressureHistory[0] = currentBarohPa; 
        lastLogTime = now;
    }
    
    // FAST ACCELERATION FIX: Check if at least the second element has populated 
    // to bypass the hour-long calibration delay.
    if (pressureHistory[1] == 0) {
        current_forecast = "Calibrating";
        return;
    }
    
    // Find the oldest valid tracked point in the buffer (to allow early updates)
    int oldestIndex = LOG_SIZE - 1;
    while (oldestIndex > 0 && pressureHistory[oldestIndex] == 0) {
        oldestIndex--;
    }
    
    // Delta calculation: compares newest pressure against oldest tracking history index
    float delta = pressureHistory[0] - pressureHistory[oldestIndex];
    if (delta <= -2.0) {
        current_forecast = "Storm Warning!";
        soundBadWeatherAlert(); 
    } else if (delta > -2.0 && delta <= -0.5) {
        current_forecast = "Rain Ahead";
    } else if (delta >= 2.0) {
        current_forecast = "Clear Skies";
    } else if (delta < 2.0 && delta >= 0.5) {
        current_forecast = "Improving";
    } else {
        current_forecast = "Stable";
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); 
    Serial.println(">>> SYSTEM BOOTING: Starting Hardware Rails... <<<");
    // 1. Heltec V4 Screen & Sensor Power Rail Latch (Must be LOW to power the TFT & BME280)
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, LOW);   // Driven LOW continuously to turn screen/I2C rails ON
    delay(500);                   
    Serial.println("[OK] Screen and Sensor rails latched ON.");
    // 2. Configure the Standby & Reset pins to keep the L76K chip awake
    pinMode(PIN_GPS_STANDBY, OUTPUT);
    digitalWrite(PIN_GPS_STANDBY, HIGH); // High = Force Active Wake Mode
    pinMode(PIN_GPS_RESET, OUTPUT);
    digitalWrite(PIN_GPS_RESET, LOW);    // Pull low briefly to clear system state
    delay(150); 
    digitalWrite(PIN_GPS_RESET, HIGH);   // Pull high to let the GPS run
    // 3. Turn ON the internal L76K power gate (Active-LOW)
    pinMode(PIN_GPS_EN, OUTPUT);
    digitalWrite(PIN_GPS_EN, LOW);       // LOW actively passes power to the L76K
    delay(1000);                         // Give the module 1 full second to warm up internal crystal
    Serial.println("[OK] GPS Hardware Lines Awake.");
    // 4. Open the Explicit Hardware Serial 1 Bus for the GPS
    GPSSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("[OK] GPSSerial mounted to GPS pins.");
    // 5. Mount Dual I2C Buses
    Serial.println("Initializing I2C Wire (Sensors)...");
    Wire.begin(B2B_SDA, B2B_SCL, 100000);         

    Serial.println("Initializing I2C Wire1 (Touch)...");
    Wire1.begin(TOUCH_SDA_PIN, TOUCH_SCL_PIN);   
    Serial.println("[OK] I2C Buses online.");
    pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(WAKE_BUTTON_PIN), handleWakeButton, FALLING);
    pinMode(BUZZER_PIN, OUTPUT);
    // Initialize Display Hardware
    Serial.println("Initializing TFT LCD...");
    tft.init(240, 320);       
    tft.setRotation(1);       
    tft.invertDisplay(true);  
    setBacklight(true);
    Serial.println("[OK] Display ready.");
    // Sensor Testing 
    if (!bme.begin(0x76, &Wire)) {
        bme.begin(0x77, &Wire); 
    }
    myGXHTC.setMode(SHTC3_CMD_CSE_RHF_LPM); 
    myGXHTC.begin(Wire);
    // Touch Controller
    touch.chsc6x_init();
    playChirp(2000, 100); 
    delay(80);            
    playChirp(2500, 150); 
    // Initialize LVGL 9 Core Frame
    lv_init();
    static uint8_t lvBuffer[320 * 20 * 2]; 
    lv_display_t *disp = lv_display_create(320, 240); 
    lv_display_set_buffers(disp, lvBuffer, NULL, sizeof(lvBuffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565); 
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);
    ui_init(); 
    lastActivityTime = millis();
    Serial.println(">>> SYSTEM BOOT COMPLETE: Entering Loop <<<");
}
unsigned long lastTickMillis = 0; 
// FORCE THE COMPILER TO ALLOW EEZ STUDIO EXPORTER TYPO CONVERSIONS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpermissive"
void loop() {
    // 1. Feed the Core LVGL Engine Clock
    unsigned long currentMillis = millis();
    unsigned long tickDifference = currentMillis - lastTickMillis;
    lv_tick_inc(tickDifference);
    lastTickMillis = currentMillis;
    // 2. Handle Task Handlers smoothly
    unsigned int tickRate = lv_timer_handler();
    delay(tickRate > 0 ? tickRate : 1);
    // Check screen wake/sleep
    lv_indev_t* indev = lv_indev_get_next(NULL); 
    if (indev != NULL) {
        lv_indev_data_t data;
        lv_indev_get_vect(indev, &data.point); 
        if (lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED) {
            lastActivityTime = currentMillis; 
            if (!isScreenAwake) setBacklight(true);
        }
    }
    if (wakeRequested) {
        wakeRequested = false;
        lastActivityTime = currentMillis;
        if (!isScreenAwake) setBacklight(true);
    }
    if (isScreenAwake && (currentMillis - lastActivityTime >= SCREEN_TIMEOUT_MS)) {
        setBacklight(false);
    }
    // Process incoming GPS packets via your true working GPSSerial object
    int gpsBytesRead = 0;
    while (GPSSerial.available() > 0) {
        gps.encode(GPSSerial.read());
        gpsBytesRead++;
    }
    // ==========================================
    // --- 1. SYSTEM TIME CALCULATIONS FIRST ---
    // ==========================================
    int rawHour = 12;
    int localMinute = 0;

    // Auto-calculates Eastern Time (-4.0 for EDT / -5.0 for EST)
    const float EASTERN_TIME_OFFSET = -4.0; 
    if (gps.time.isValid() && gps.satellites.value() > 0) {
        rawHour = gps.time.hour() + (int)EASTERN_TIME_OFFSET;
        localMinute = gps.time.minute() + (int)((EASTERN_TIME_OFFSET - (int)EASTERN_TIME_OFFSET) * 60);
        if (localMinute >= 60) { localMinute -= 60; rawHour += 1; }
        else if (localMinute < 0) { localMinute += 60; rawHour -= 1; }
        if (rawHour >= 24) { rawHour -= 24; }
        else if (rawHour < 0) { rawHour += 24; }
    } else {
        unsigned long totalSecs = currentMillis / 1000;
        localMinute = (totalSecs / 60) % 60;
        rawHour = (12 + (totalSecs / 3600)) % 24;
        rawHour = (rawHour + (int)EASTERN_TIME_OFFSET + 24) % 24;
    }
    // ==========================================
    // --- NEW: 12-HOUR FORMAT CONVERSION ---
    // ==========================================
    int localHour = rawHour % 12;
    if (localHour == 0) localHour = 12; // Midnight & Noon are 12
    // Format time and send to EEZ Flow engine
    char timeString[16]; 
    sprintf(timeString, "%d:%02d", localHour, localMinute); // No leading zero for hours in 12-hour format (e.g. "1:05" instead of "01:05")
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_TIME_DATA, eez::StringValue(timeString));
    // --- THROTTLED SENSOR & BATTERY MANAGEMENT ---
    static float DisplayTemp = 0.0;
    static float DisplayHumidity = 0.0;
    static float DisplayPressure = 0.0;
    static int batteryPercentage = 100;
    if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
        lastSensorReadTime = currentMillis;
        // AUTO-RECOVER FOR UNINITIALIZED BME280 SENSORS
        float rawPressure = bme.readPressure() / 100.0F; 
        if (isnan(rawPressure) || rawPressure <= 0.0) {
            if (!bme.begin(0x76, &Wire)) {
                bme.begin(0x77, &Wire); 
            }
            rawPressure = bme.readPressure() / 100.0F; 
        }
                myGXHTC.update(); 
        float rawTemp = (myGXHTC.lastStatus == SHTC3_Status_Nominal) ? myGXHTC.toDegC() : bme.readTemperature();
        float rawHumidity = (myGXHTC.lastStatus == SHTC3_Status_Nominal) ? myGXHTC.toPercent() : bme.readHumidity();
        
        bool raw_metric_mode = eez::flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_IS_METRIC).getBoolean();
        bool metric_mode = !raw_metric_mode; 
        if (metric_mode) {
            DisplayTemp = rawTemp;                     
            DisplayPressure = rawPressure;             
        } else {
            DisplayTemp = (rawTemp * 9.0 / 5.0) + 32.0; 
            DisplayPressure = rawPressure * 0.02953;   
        }
        DisplayHumidity = rawHumidity;
        
        if (!isnan(rawPressure) && rawPressure > 0.0) {
            computeZambrettiForecast(rawPressure);
        }
        
        // ==================================================
        // --- REAL HELTEC WIRELESS TRACKER V4 BATTERY FIX ---
        // ==================================================
        const int ADC_CTRL_PIN = 37; // Pin 37 acts as the V4 Gate Latch
        const int VBAT_READ_PIN = 1; // Pin 1 handles the V4 Analog input

        // 1. Energize the measurement gate circuit track 
        pinMode(ADC_CTRL_PIN, OUTPUT);
        digitalWrite(ADC_CTRL_PIN, HIGH);  // HIGH passes power into the bridge network
        delay(30);                         // Settle time for capacitor stabilization
        
        // 2. Perform a hardware-compensated millivolt sample
        analogReadResolution(12);
        uint32_t pinMilliVolts = analogReadMilliVolts(VBAT_READ_PIN); 
        
        // 3. Immediately sever the gate loop to protect against parasitic battery drain
        digitalWrite(ADC_CTRL_PIN, LOW);   
        
        // 4. Run the official Heltec V4 scaling calculation:
        // Convert pin millivolts to volts, then scale up by the hardware resistor network multiplier (approx 4.9x)
        float batteryVoltage = ((float)pinMilliVolts / 1000.0) * 4.9;
        
        // --- CALIBRATION ADJUSTMENT ---
        // If your multimeter shows a variance, you can fine-tune the 4.9 factor up or down slightly (e.g., 4.75)
        
        // 5. Establish realistic threshold ceilings for a single-cell LiPo battery
        // Fully Charged (USB plugged in) = 4.2V, Safe Empty Threshold = 3.4V
        if (batteryVoltage > 4.25) batteryVoltage = 4.2;
        if (batteryVoltage < 3.4)  batteryVoltage = 3.4;
        
        // 6. Map the scale cleanly into full integer percentages
        batteryPercentage = (int)((batteryVoltage - 3.4) / (4.2 - 3.4) * 100.0);

        // 7. Diagnostics line to confirm raw digital values are changing
        Serial.print("[BATTERY DEBUG] Pin mV: "); Serial.print(pinMilliVolts);
        Serial.print(" | Calculated Voltage: "); Serial.print(batteryVoltage, 2);
        Serial.print("V | Calculated Percentage: "); Serial.print(batteryPercentage); Serial.println("%");

    }
    
    // --- CONVERT FLOATS & INTEGERS TO STRINGS WITH UNITS FOR THE TOUCHSCREEN UI ---
    char tempStr[20];
    char humidStr[20];
    char pressStr[20];
    char batStr[10];

    // Read the current unit system mode directly from your UI button toggle
    bool raw_metric_mode = eez::flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_IS_METRIC).getBoolean();
    bool metric_mode = !raw_metric_mode; 

    // Dynamically append the correct units based on the selected mode
    if (metric_mode) {
        sprintf(tempStr, "%.2f °C", DisplayTemp);
        sprintf(pressStr, "%.2f hPa", DisplayPressure); // Metric Pressure (Hectopascals)
    } else {
        sprintf(tempStr, "%.2f °F", DisplayTemp);
        sprintf(pressStr, "%.2f inHg", DisplayPressure); // Imperial Pressure (Inches of Mercury)
    }
    
    // Humidity and Battery always use the percent symbol regardless of mode
    sprintf(humidStr, "%.2f%%", DisplayHumidity);
    sprintf(batStr, "%d%%", batteryPercentage); 
    
    // Pass the clean text strings directly to your UI display layout widgets
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_TEMP_DATA, eez::StringValue(tempStr));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_HUMIDITY_DATA, eez::StringValue(humidStr));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_PRESSURE_DATA, eez::StringValue(pressStr));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_BATTERY_PERCENT, eez::StringValue(batStr));
    
    // Keeps intact your regular tracking strings and layout tickers
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_FORECAST_DATA, eez::StringValue(current_forecast));
    
    ui_tick(); 


    
    if (currentMillis - lastDebugPrint > 3000) {
        lastDebugPrint = currentMillis;
        Serial.println("--------------- LIVE RUNTIME SNAPSHOT ---------------");
        Serial.printf("GPS Bytes Read: %d | Satellites Tracked: %d\n", gpsBytesRead, gps.satellites.value());
        Serial.printf("Battery Capacity Output Level: %d%%\n", batteryPercentage);
        Serial.printf("UI Data -> Temp: %.2f | Humidity: %.2f | Pressure: %.2f | Forecast: %s\n", DisplayTemp, DisplayHumidity, DisplayPressure, current_forecast);
        Serial.println("-----------------------------------------------------");
    }
}
#pragma GCC diagnostic pop

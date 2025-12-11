// Include libraries for I2C communication, light sensing, and OLED display functionalities
#include <Wire.h> // Allows for I2C communication with devices
#include "BH1750.h" // Library for the BH1750 light sensor
#include <Adafruit_GFX.h> // Adafruit's graphics library for displays
#include <Adafruit_SSD1306.h> // Adafruit's library for SSD1306 OLED displays

// Definitions for the OLED display dimensions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // OLED reset pin, set to -1 as it's not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Initialize OLED display object

bool displayConnected = false; // Flag to check if OLED display is connected

// Initialize BH1750 light sensor object
BH1750 lightMeter;

// Define pin connections for sensors and relay
#define TEMT6000_PIN 1 // Analog pin for TEMT6000 light sensor
#define LIGHTS_PIN 4 // Digital pin to control the relay

// Light level thresholds and hysteresis values to prevent rapid toggling
#define BH1750_THRESHOLD 200
#define TEMT6000_THRESHOLD 0.205f // Adjusted threshold for TEMT6000
#define BH1750_HYSTERESIS 10
#define TEMT6000_HYSTERESIS 0.02

// Timing constants for delays and debounce
#define DEBOUNCE_INTERVAL 4000 // Debounce interval for light level changes
#define SAMPLE_INTERVAL 500 // Interval between sensor readings

// Variables for managing timing, light control logic, and delays
unsigned long lastSampleTime = 0;
unsigned long debounceTimer = 0;
bool lightsOn = false; // State of the lights
bool pendingStateChange = false; // Indicates if a state change is in the debounce period

// Structure to hold sensor readings
struct SensorReadings {
  float lux; // Light level from the BH1750 sensor
  float temtVoltage; // Voltage from the TEMT6000 sensor
} globalSensorReadings;

void setup() {
  Serial.begin(9600); // Start serial communication for debugging
  Wire.begin(6, 7); // Initialize I2C with default pins
  Wire.setClock(400000); // Set I2C clock speed to 400kHz

  // Attempt to start the OLED display and check its connection
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    displayConnected = true; // Mark display as connected
    display.display(); // Show initialized content
    delay(2000); // Wait for 2 seconds
    display.clearDisplay(); // Clear display for a fresh start
  } else {
    Serial.println(F("Display not found. Continuing without display."));
  }

  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE); // Start BH1750 sensor in continuous high-res mode
  pinMode(TEMT6000_PIN, INPUT); // Set TEMT6000 pin as input
  pinMode(LIGHTS_PIN, OUTPUT); // Set relay control pin as output
}

void loop() {
  unsigned long currentMillis = millis(); // Get current time in milliseconds

  // Perform sensor reading and processing at a fixed interval
  if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = currentMillis;
    readAndProcessSensors(); // Read from sensors
    if (displayConnected) {
      updateDisplay(); // Update the display only if connected
    }
    controlLights(currentMillis); // Control lights based on sensor data and logic
  }
}

// Function to read from light sensors and store their values
void readAndProcessSensors() {
  float lux = lightMeter.readLightLevel(); // Read light level from BH1750 sensor
  float temtVoltage = analogRead(TEMT6000_PIN) * (3.3 / 4095.0); // Convert ADC value to voltage for TEMT6000

  // Store sensor values in a global structure
  globalSensorReadings.lux = lux;
  globalSensorReadings.temtVoltage = temtVoltage;
}

// Function to update the OLED display with sensor values and light status
void updateDisplay() {
  if (displayConnected) {
    display.clearDisplay(); // Clear display for new content
    display.setTextSize(1); // Set text size
    display.setTextColor(SSD1306_WHITE); // Set text color
    display.setCursor(0,0); // Set text cursor position

    // Display S1 (BH1750) light level or "MAX" if above a threshold
    display.print(F("S1: "));
    if(globalSensorReadings.lux > 3500) {
      display.print(F("MAX"));
    } else {
      display.print(globalSensorReadings.lux, 2);
    }
    display.println(" lx");
    
    // Display S2 (TEMT6000) voltage or "MAX" if above a threshold
    display.print(F("S2: "));
    if(globalSensorReadings.temtVoltage > 3.25) {
      display.print(F("MAX"));
    } else {
      display.print(globalSensorReadings.temtVoltage, 3);
    }
    display.println(" V");

    // Display the current state of the relay
    display.print(F("Relay: "));
    display.println(lightsOn ? F("ON") : F("OFF"));

    display.display(); // Push the updated content to the display
  }
}

// Function to control the relay based on sensor readings, debounce logic, and ON/OFF delays
void controlLights(unsigned long currentMillis) {
  // Determine conditions for turning lights on or off
  bool shouldTurnOn = !lightsOn && globalSensorReadings.lux < (BH1750_THRESHOLD - BH1750_HYSTERESIS) &&
                      globalSensorReadings.temtVoltage < (TEMT6000_THRESHOLD - TEMT6000_HYSTERESIS);
  bool shouldTurnOff = lightsOn && globalSensorReadings.lux > (BH1750_THRESHOLD + BH1750_HYSTERESIS) &&
                       globalSensorReadings.temtVoltage > (TEMT6000_THRESHOLD + TEMT6000_HYSTERESIS);

  // Check if state change condition is met and debounce timer has not started
  if ((shouldTurnOn || shouldTurnOff) && !pendingStateChange) {
    pendingStateChange = true; // Indicate that a state change condition has been met
    debounceTimer = currentMillis; // Start the debounce timer
  } else if (pendingStateChange) {
    // Check if the debounce period has elapsed
    if (currentMillis - debounceTimer >= DEBOUNCE_INTERVAL) {
      // Perform the state change if the condition persists
      if ((shouldTurnOn && !lightsOn) || (shouldTurnOff && lightsOn)) {
        lightsOn = shouldTurnOn; // Update the state based on the condition
        digitalWrite(LIGHTS_PIN, lightsOn ? HIGH : LOW); // Apply the new state to the relay
      }
      pendingStateChange = false; // Reset the pending state change flag
    }
  } else {
    pendingStateChange = false; // Reset debounce timer and pending state change flag if conditions are not met
  }
}

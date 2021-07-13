// Adafruit IO Dashboard: https://io.adafruit.com/jpwall/dashboards/session3

#include "config.h" // Please use your own config.h file (used for Adafruit IO Dashboard)

// Include libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_MPL115A2.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// MPL115A2 sensor configurations
Adafruit_MPL115A2 mpl115a2;

// DHT22 sensor connection
#define DATA_PIN 32

// SSD1306 OLED screen configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create DHT22 instance
DHT_Unified dht(DATA_PIN, DHT22);

// Initialize 'temperature', 'humidity', and 'pressure' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *pressure = io.feed("pressure");

void setup() {
  // Define serial rate
  Serial.begin(115200);
  Serial.println();
  // Show date and time compiled
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));
  // Wait for monitor to open and print information
  while(! Serial);

  // Check for MPL115A2 sensor
  if (! mpl115a2.begin()) {
    Serial.println("Could not find MPL115A2. Please check wiring.");
    while(1); // loop forever if not connected
  }

  // Check for SSD1306 display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 display not found. Please connect it."));
    while(1); // loop forever if not connected
  }

  // Clear display
  display.clearDisplay();
  // Set parameters
  display.setTextSize(1); // 2X text size
  display.setTextColor(SSD1306_WHITE);

  // Initialize DHT22
  dht.begin();

  // Connect to Adafruit IO
  Serial.print("Conntecting to Adafruit IO");
  io.connect();

  // Wait for connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // Print IO status
  Serial.println();
  Serial.println(io.statusText());
}

void loop() {
  // Keep client connected to Adafruit IO
  io.run();

  // Initialize and get temperature event
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  // Get temperature in Celsius from the DHT22
  float celsius = event.temperature;
  // Get temperature in Celsius from the MPL115A2
  float celsiusMPL = mpl115a2.getTemperature();
  // Average the two temperature values
  float celsiusAVG = (celsius + celsiusMPL) * 0.5;
  // Convert to Fahrenheit
  float fahrenheit = (celsiusAVG * 1.8) + 32;

  // Output temperature average
  Serial.print("Average temp (F): ");
  Serial.println(fahrenheit);

  // Save temperature to Adafruit IO
  temperature -> save(fahrenheit);

  // Get humidity event
  dht.humidity().getEvent(&event);

  // Output humidity from DHT22
  Serial.print("humidity: ");
  Serial.print(event.relative_humidity);
  Serial.println("%");

  // Save humidity to Adafruit IO
  float humidityMPL = event.relative_humidity;
  humidity -> save(humidityMPL);

  // Output pressure from MPL115A2
  float pressureKPA = mpl115a2.getPressure();
  Serial.print("Pressure (kPa): ");
  Serial.println(pressureKPA);

  // Save pressure to Adafruit IO
  pressure -> save(pressureKPA);

  // Clear the display
  display.clearDisplay();
  display.setCursor(0, 0); // top left corner (x, y)
  // Print all temperature readings on one line - DHT22, MPL115A2, and average
  display.print("DHT: "); display.print(celsius);
  display.print(", MPL: "); display.print(celsiusMPL);
  display.print(", AVG: "); display.println(celsiusAVG);
  // Print converted fahrenheit reading on new line
  display.print("Temp [F]: "); display.println(fahrenheit);
  // Print humidity
  display.print("Humidity: "); display.print(humidityMPL); display.println("%");
  // Print pressure
  display.print("Press. [kPa]: "); display.println(pressureKPA);
  // Show on the screen
  display.display();
  
  // Wait 6.5 seconds to avoid going over rate limit for Adafruit IO
  delay(6500);
}

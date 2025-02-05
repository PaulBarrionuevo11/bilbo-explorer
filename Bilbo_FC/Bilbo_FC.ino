/*
 * Bilbo's integration with multiple sensors to navigate the environment
 * Author: Paul Barrionuevo
 * Description: Interaction of sensors to calibrate and position a flying vehicle
 * with the intention to replace a Flight Control System / Flight Controller
 * Sensor and components included in the project are:
 * IMU: MPU6050
 * Altimeter: BMP280
 * Microcontroller: ESP32 WROOM
 * Ultrasound 
 * RGB LED
 * Date: 1/18/2025
 */

//#include <LiquidCrystal_I2C.h> # For displaying in LCD Screen
#include <Wire.h>
#include <basicMPU6050.h>   // I2C address 0x68
#include <Adafruit_BMP280.h> // I2C address 0x76

// LCD setup: 20x4 display with I2C address 0x27
//LiquidCrystal_I2C lcd(0x27, 20, 4); 

// Create instance
basicMPU6050<> imu;
Adafruit_BMP280 bmp;

// variables for IMU and altimeter sensor
float ax, ay, az, gx, gy, gz; // acceleration and gyroscope
float pressure, altimeter;
// RGB variable intialization
const char RLed = 2;
const char GLed = 4;
const char BLed = 5;
// Ultrasound sensor pinout
const char trigPin = 18;
const char echoPin = 19;
long duration;
float distance;

// variables for time intervals
unsigned long previousMillisBMP = 0;
unsigned long previousMillisMPU = 0;
unsigned long previousMillisUltrasound = 0;
const unsigned long intervalBMP = 1000; // Read BMP280 every 1 seconds
const unsigned long intervalMPU = 500;  // Read MPU6050 every 500 ms
const unsigned long intervalUltrasound = 1200;

int baudrate = 115200;

void setup() {
  // # To initialize the LCD Screen
  //lcd.init();
  //lcd.backlight();
  //lcd.setCursor(0, 0);
  //lcd.print("Initializing MPU6050");
  //lcd.setCursor(0, 1);
  
  // Start Serial for debugging
  Serial.begin(baudrate);
  // RGB LEDs
  pinMode(RLed, OUTPUT);
  pinMode(GLed, OUTPUT);
  pinMode(BLed, OUTPUT);
  // Ultrasound
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  
  // Initialize MPU6050
  imu.setup();
  imu.setBias();  // Initial bias values - calibration of the gyro
  //lcd.print("MPU6050 Ready");
  delay(1000);  // Display initialization message
  //lcd.clear(); 

  unsigned status;
  status = bmp.begin(0x76);  // bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); 
    Serial.println(bmp.sensorID(),16);
    Serial.print("ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("ID of 0x60 represents a BME 280.\n");
    Serial.print("ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }
  
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() { 

  unsigned long currentMillis = millis();

  // Read MPU6050 data
  if (currentMillis - previousMillisMPU >= intervalMPU) {
    previousMillisMPU = currentMillis;
    imu.updateBias(); // Compensate for drifts or environmental changes
    // Accelerometer data initalization
    ax = imu.ax();
    ay = imu.ay();
    az = imu.az();
    // Gyroscope data initalization
    gx = imu.gx();
    gy = imu.gy();
    gz = imu.gz();
    // #MPU6050 also has tempt
    //float tempImu = imu.temp();
    Serial.println("Acceleration: ");
    Serial.print("X: ");
    Serial.print(ax);
    Serial.print(" - ");
    Serial.print("Y: ");
    Serial.print(ay);
    Serial.print(" - ");
    Serial.print("Z: ");
    Serial.println(az);
  
    Serial.println("Gyroscope: ");
    Serial.print("X: ");
    Serial.print(gx);
    Serial.print(" - ");
    Serial.print("Y: ");
    Serial.print(gy);
    Serial.print(" - ");
    Serial.print("Z: ");
    Serial.println(gz);
    //Serial.print(" Temperature   ");
    //Serial.println(tempImu);
  }

  // Read BMP280 data
  if(currentMillis - previousMillisBMP >= intervalBMP)
  {
    previousMillisBMP = currentMillis;
    // Altimeter data
    pressure = bmp.readPressure();
    altimeter = bmp.readAltitude(1013.25);
    float tempBmp = bmp.readTemperature();
    
    Serial.print(F("Temperature = "));
    Serial.print(tempBmp);
    Serial.println(" °C");
    Serial.print(F("Pressure = "));
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.print(F("Approx altitude = "));
    Serial.print(altimeter); /* Adjusted to local forecast! */
    Serial.println(" m");
  }

  if(currentMillis - previousMillisUltrasound >= intervalUltrasound)
  {
    previousMillisUltrasound = currentMillis;
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);  // Short stabilization delay
  
    // Triggers the sensor
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the duration
    duration = pulseIn(echoPin, HIGH, 30000); // Timeout after 30 ms
    if (duration == 0) {
      Serial.println("No object detected or timeout");
      return;
    }

    // Calculate the distance in cm
    distance = duration * 0.034 / 2;
  
    // Print the distance
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // lcd.setCursor(0, 0);
    // lcd.print("Distance: ");
    // lcd.setCursor(0, 1);
    // lcd.print(distance);
    // lcd.print(" cm");
  
    // Control the RGB LED
    if (distance > 0 && distance <= 10) { // Very close
      analogWrite(RLed, 255);
      analogWrite(GLed, 0);
      analogWrite(BLed, 0);
    } else if (distance > 10 && distance <= 20) { // Medium range
      analogWrite(RLed, 0);
      analogWrite(GLed, 255);
      analogWrite(BLed, 0);
    } else if (distance > 20) { // Far
      analogWrite(RLed, 0);
      analogWrite(GLed, 0);
      analogWrite(BLed, 0);
    } else { // No valid reading
      analogWrite(RLed, 100);
      analogWrite(GLed, 100);
      analogWrite(BLed, 100);
    }
    delay(200); // Short delay for better performance
  }
}

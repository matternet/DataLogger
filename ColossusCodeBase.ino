//***************************************************//
//Code base for Project Colossus™
// Matternet, Inc. © Copyright 2018
// All rights reserved
// Amir Khan
//***************************************************//

#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include <SD.h>
#include "Adafruit_MAX31855.h"

#define VBATPIN 9
#define MAXCS1 0
#define MAXCS2 1
#define MAXCS3 30
#define MAXCS4 4

Adafruit_MAX31855 thermocouple1(MAXCS1);
Adafruit_MAX31855 thermocouple2(MAXCS2);
Adafruit_MAX31855 thermocouple3(MAXCS3);
Adafruit_MAX31855 thermocouple4(MAXCS4);
RTC_DS3231 rtc;
const int chipSelect = 11;
double temp1, temp2, temp3, temp4;
//char filename[] = "YYYYMMDD_HHMM_XX.CSV"; Placeholder for filename convention
File logfile;

// error handling using LED
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}

void InitRTC() {
    // setup for the RTC
  
  if (!rtc.IsActive) {
    Serial.print("RTC not active.....");
    Serial.println("Beginning Initiliazation sequence");
    rtc.begin();
    if (! rtc.begin()) {
      Serial.println("Couldn't Init RTC");
      error(2);
      while(1); //do nothing
    }
    else {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      Serial.print("RTC initialized and set to compile time");
    }
  }
  else {
    Serial.print("RTC Active and running and the current date and time are -");
    Serial.println(rtc.now());
  }
}

void InitSDCard() {
  pinMode(11, OUTPUT);
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    error(2);
    // don't do anything more:
    while (1);
  }
  Serial.println("SD card initialized.");
}

void MeasureBattVoltage() {
  // Measure the battery voltage
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // Adafruit divided by 2, so multiplying back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " );
  Serial.println(measuredvbat);

  //Raise an error flag if battery voltage drops below 3.3V
  if (measuredvbat < 3.3) {
    Serial.println("Battery voltage below accepted level");
    digitalWrite(13, HIGH);
    delay(100);
  }
}



void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) delay(1); {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  InitSDCard();
  InitRTC();
  File logfile = SD.open("Example01.CSV", FILE_WRITE);
}

void loop() {

  MeasureBattVoltage();
  delay(1000);

  temp1 = thermocouple1.readCelsius();
  temp2 = thermocouple2.readCelsius();
  temp3 = thermocouple3.readCelsius();
  temp4 = thermocouple4.readCelsius();

  logfile.print("TC1 = ");
  logfile.println(temp1);
  logfile.print("TC2 = ");
  logfile.println(temp2);
  logfile.print("TC3 = ");
  logfile.println(temp3);
  logfile.print("TC4 = ");
  logfile.println(temp4);
  logfile.close();
  Serial.print(temp1);
  Serial.print(temp2);
  Serial.print(temp3);
  Serial.print(temp4);
  }
}


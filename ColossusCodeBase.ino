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
#define SDA 2
#define SCL 3

Adafruit_MAX31855 thermocouple1(MAXCS1);
Adafruit_MAX31855 thermocouple2(MAXCS2);
Adafruit_MAX31855 thermocouple3(MAXCS3);
Adafruit_MAX31855 thermocouple4(MAXCS4);
RTC_DS3231 rtc;
const int chipSelect = 4;
char filename[] = "YYYYMMDD_HHMM_XX.CSV";
File logfile;
DateTime now;
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
  while(!Serial); // for Leonardo/Micro/Zero
    if(! rtc.begin()) {
      Serial.println("Couldn't Init RTC");
      while (1);
    }
    else {
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      Serial.print("RTC Initialized");
    }
    if(!rtc.lostPower()) {
      Serial.println("RTC is NOT running!");
    }
    DateTime now = rtc.now();
}

void InitSDCard() {
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
  Serial.print("VBat: " ); Serial.println(measuredvbat);

  //Raise an error flag if battery voltage drops below 3.3V
  if (measuredvbat < 3.3) {
    Serial.println("Battery voltage below accepted level");
    digitalWrite(13, HIGH);
    delay(100);
  }
}

void initFileName(DateTime time1) {
  char buf[5];
  // integer to ascii function itoa(), supplied with numeric year value,
  // a buffer to hold output, and the base for the conversion (base 10 here)
  itoa(time1.year(), buf, 10);
  // copy the ascii year into the filename array
  for (byte i = 0; i <= 4; i++){
    filename[i] = buf[i];
  }
  // Insert the month value
  if (time1.month() < 10) {
    filename[4] = '0';
    filename[5] = time1.month() + '0';
  } else if (time1.month() >= 10) {
    filename[4] = (time1.month() / 10) + '0';
    filename[5] = (time1.month() % 10) + '0';
  }
  // Insert the day value
  if (time1.day() < 10) {
    filename[6] = '0';
    filename[7] = time1.day() + '0';
  } else if (time1.day() >= 10) {
    filename[6] = (time1.day() / 10) + '0';
    filename[7] = (time1.day() % 10) + '0';
  }
  // Insert an underscore between date and time
  filename[8] = '_';
  // Insert the hour
  if (time1.hour() < 10) {
    filename[9] = '0';
    filename[10] = time1.hour() + '0';
  } else if (time1.hour() >= 10) {
    filename[9] = (time1.hour() / 10) + '0';
    filename[10] = (time1.hour() % 10) + '0';
  }
  // Insert minutes
    if (time1.minute() < 10) {
    filename[11] = '0';
    filename[12] = time1.minute() + '0';
  } else if (time1.minute() >= 10) {
    filename[11] = (time1.minute() / 10) + '0';
    filename[12] = (time1.minute() % 10) + '0';
  }
  // Insert another underscore after time
  filename[13] = '_';
    
  // Next change the counter on the end of the filename
  // (digits 14+15) to increment count for files generated on
  // the same day. This shouldn't come into play
  // during a normal data run, but can be useful when 
  // troubleshooting.
  for (uint8_t i = 0; i < 100; i++) {
    filename[14] = i / 10 + '0';
    filename[15] = i % 10 + '0';
    filename[16] = '.';
    filename[17] = 'c';
    filename[18] = 's';
    filename[19] = 'v';
        
    if (!SD.exists(filename)) {
      // when SD.exists() returns false, this block
      // of code will be executed to open the file
      if (!SD.open(filename, FILE_WRITE)) {
        // If there is an error opening the file, notify the
        // user. Otherwise, the file is open and ready for writing
        Serial.print(F("File open failed"));
      }
      break; // Break out of the for loop when the
      // statement if(!logfile.exists())
      // is finally false (i.e. you found a new file name to use).
    } // end of if(!SD.exists())
  } // end of file-naming for loop
  
  //Write the output file header row to our file so that we can identify the data later
  logfile.println(F("datetime,Ch1,Ch2,Ch3,Ch4,Ch5,Ch6,Ch7,Ch8"));

  delay(350);

}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) delay(1); {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(11, OUTPUT);
  InitSDCard();
  InitRTC();
  initFileName(now);
  
}

void loop() {

  MeasureBattVoltage();
  double temp1, temp2, temp3, temp4;
  delay(1000);

  temp1 = thermocouple1.readCelsius();
  temp2 = thermocouple2.readCelsius();
  temp3 = thermocouple3.readCelsius();
  temp4 = thermocouple4.readCelsius();

  File logfile = SD.open(filename, FILE_WRITE);

  if (logfile) {
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
  else {
    Serial.print("Error opening File");
  }
}


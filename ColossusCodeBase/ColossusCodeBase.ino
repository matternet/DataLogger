//***************************************************//
//Code base for Project Colossus™
// Matternet, Inc. © Copyright 2018
// All rights reserved
// Amir Khan
//***************************************************//

#include <Wire.h>
#include <SPI.h>
#include <DS3231.h>
#include <SD.h>
#include "Adafruit_MAX31855.h"

//SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);

#define VBATPIN 9
#define MAXCS1 0
#define MAXCS2 1
#define MAXCS3 30
#define MAXCS4 4

Adafruit_MAX31855 thermocouple1(MAXCS1);
Adafruit_MAX31855 thermocouple2(MAXCS2);
Adafruit_MAX31855 thermocouple3(MAXCS3);
Adafruit_MAX31855 thermocouple4(MAXCS4);
DS3231 RTC;
DateTime now;
char filename[8 + 1 + 3 + 1];
char datetime[2 + 1 + 2 + 1 + 4 + 3 + 2 + 1 + 2 + 1 + 2 + 1];
const int chipSelect = 11;
double temp1, temp2, temp3, temp4;
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

void gentime()
{
  int idx;
  now = RTClib::now();
  idx = sprintf(datetime, "%02u/%02u/%04u , ", now.month(), now.day(), now.year());
  sprintf(&datetime[idx], "%02u:%02u:%02u", now.hour(), now.minute(), now.second());
}

void InitRTC() {
    // setup for the RTC
  now = RTClib::now();
  if (now.year() < 2018 || now.year() > 2100 || RTC.oscillatorCheck() == false) {
    Serial.print("RTC not active.....");
    Serial.println("Beginning Initiliazation sequence");
    while (RTC.oscillatorCheck() == false) {
      RTC.enableOscillator(true, true, 0);
      RTC.setSecond(0);
      RTC.setMinute(0);
      RTC.setHour(0);
      RTC.setDoW(2);
      RTC.setDate(1);
      RTC.setMonth(1);
      RTC.setYear(18);
      RTC.setClockMode(false);
    }
    Serial.println("Oscillator Started");
    }
   else {
    Serial.print("RTC Active and Running");    
  }
  Serial.println("The current time is....");
  gentime();
  Serial.println(datetime);
}

void InitSDCard() {
  pinMode(11, OUTPUT);
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    //error(2);
    // don't do anything more:
    //while (1);
  }
  else
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

bool genfilename() {
  int i;
    for (i = 1; i < 1000; i++)
    {
        sprintf(filename, "%08u.csv", i);
        if (SD.exists(filename) == false) {
          return true; 
        }
    }
    return false;
 }


void setup() {
  Wire.begin();
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) delay(1); {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  InitSDCard();
  InitRTC();


  if (genfilename() == false) {}  
  
  logfile = SD.open(filename, FILE_WRITE);  
  if (!logfile) {
    error(2);
    while(1){
      Serial.print("File can not be opened for writing...Terminating");
      delay(1000);
    }
  }
  else {
    Serial.print("File opened for writing...Beginning logging data");
    digitalWrite(8, HIGH);
    delay(100);
    digitalWrite(8, LOW);
    delay(100);
  }
  Serial.println(filename);
  logfile.print(F("Date"));
  logfile.print(F(","));
  logfile.print(F("Time"));
  logfile.print(F(","));
  logfile.print(F("TC1"));
  logfile.print(F(","));
  logfile.print(F("TC2"));
  logfile.print(F(","));
  logfile.print(F("TC3"));
  logfile.print(F(","));
  logfile.print(F("TC4"));
  logfile.print(F(","));
  logfile.flush();
}
void loop() {

  MeasureBattVoltage();
  delay(1000);

  temp1 = thermocouple1.readCelsius();
  temp2 = thermocouple2.readCelsius();
  temp3 = thermocouple3.readCelsius();
  temp4 = thermocouple4.readCelsius();

  now = RTClib::now();

  logfile.println();
  gentime();
  logfile.print(datetime);
  logfile.print(F(","));
  logfile.print(temp1);
  logfile.print(F(","));
  logfile.print(temp2);
  logfile.print(F(","));
  logfile.print(temp3);
  logfile.print(F(","));
  logfile.print(temp4);
  logfile.flush();
  Serial.print(temp1);
  Serial.println(" ");
  Serial.print(temp2);
  Serial.println(" ");
  Serial.print(temp3);
  Serial.println(" ");
  Serial.print(temp4);
  Serial.println(" ");
  }

/*
 * temphumid.ino
 *
 * Temperature/relative humidity data logger.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MCP9808.h>

#include <RTClib.h>
#include <DS1307.h>
#include <Time.h>
#include <Timezone.h>

#include "usec.h"
#include "temphumid.h"

/***********************************************************************
 * Constants & Globals
 ***********************************************************************/

// DISPLAY
Adafruit_SSD1306 display(DISPLAY_RST);

// Sensors
Adafruit_MCP9808 sensor_temp = Adafruit_MCP9808();
Adafruit_HTU21DF sensor_rh = Adafruit_HTU21DF();

// RTC
RTC_DS1307 rtc = RTC_DS1307();

// Logging
File logfile;
const uint8_t SD_CS   = 6;
const uint8_t SD_MOSI = 5;
const uint8_t SD_MISO = 3;
const uint8_t SD_SCK  = 2;
const uint8_t LOGMODE = O_WRITE | O_CREAT | O_APPEND;

float t  = 0.0;
float rh = 0.0;

#define DEBUG 1
#define SERIAL_TIME_SET 1
#define LOGFILE "temp_rh.log"

#if DEBUG
// Serial debugging setup
usec last_blat = 0;
usec BLAT_INT = 1;
#endif

/***********************************************************************
 * "Helping"
 ***********************************************************************/

/*
 * Set time based on UNIX timestamps shat out on the serial port.
 */
#if SERIAL_TIME_SET
void timeset() {
  time_t newnow = 0;

  Serial.begin(9600);
  Serial.setTimeout(3000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("timeset");
  display.display();

  if (Serial.find("T")) {
    newnow = Serial.parseInt();
  }

  if (newnow != 0) {
    rtc.adjust(DateTime(newnow));
  }

  display.println(newnow);
  display.display();
  delay(1000);

}
#endif

/*
 * Barf error info to microview display.
 */
void error(const char *msg, int pause=5000) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
  delay(pause);
}

//
// Display current temp, setpoint temp, SSR level, and time since boot
//
void refreshDisplay() {

  // Format strings, stored in program memory to avoid wasting precious
  // precious RAMz.
  PGM_P FMT = PSTR(
         "Temp %#5.1f\367C"
    "\n" "RH     %#3.2f%"
    "\n" "  %2.2u:%2.2u:%2.2u"
  );

  // String-ify display data in a C-ish style way since the Arduino libs don't
  // really give us any way to handle this well (which is annoying). Note that
  // special linker flags are required to bring in the full vprintf
  // implementation for floating point number. Check the Makefile for the
  // voodoo.
  char buff[21*4+1];
  uint32_t now = millis() / 1000;
  int h = now / 60 / 60 % 100;
  int m = now / 60 % 60;
  int s = now % 60;
  snprintf_P(buff, sizeof(buff), FMT, t, rh, h, m, s);
  buff[sizeof(buff)-1] = '\0';

  // Actually shart chars to thingy
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(buff);
  display.display();

#if DEBUG
  if (USEC_DIFF(now, last_blat) > BLAT_INT) {
    last_blat = now;
    Serial.print(now); Serial.print(",");
    Serial.print(t); Serial.print(",");
    Serial.println(rh);
  }
#endif

}

/***********************************************************************
 * setup() - this is done once
 ***********************************************************************/

void setup() {

  // Boot up display
  Wire.begin();
  display.begin(DISPLAY_MODE, DISPLAY_ADDR);
  display.clearDisplay();
  display.setTextColor(DISPLAY_COLOR);
  display.setTextSize(DISPLAY_TEXTSIZE);
  display.println("Hello!");
  display.display();

  // Init sensors
  if (!sensor_temp.begin()) {
    error("Could not find MCP9808!");
  }
  if (!sensor_rh.begin()) {
    error("Could not find HTU21DF!");
  }

  // Init RTC
  rtc.begin();

  // Initalize SD card for logging
  pinMode(SD_CS, OUTPUT);
  while (!SD.begin(SD_CS, SD_MOSI, SD_MISO, SD_SCK)) {
    error("Could not connect microSD card!");
  }
  logfile = SD.open(LOGFILE, LOGMODE);
  while (!logfile) {
    error("Error opening " LOGFILE "!");
  }


#if DEBUG
  Serial.begin(9600);
  Serial.println("time,temp,rh");
#endif

}

/***********************************************************************
 * loop() - this is done all the time (like your mom)
 ***********************************************************************/

void loop() {
  
  // Update display
  refreshDisplay();

}

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
const uint8_t SD_CS   = 10;
const uint8_t SD_MOSI = 11;
const uint8_t SD_MISO = 12;
const uint8_t SD_SCK  = 13;
const uint8_t LOGMODE = O_READ | O_WRITE | O_CREAT | O_APPEND;
File logfile;

#define SERIAL_TIME_SET 0

#define LOGFILE "temp_rh.log"
#define LOG_INTERVAL_MS 1000


/***********************************************************************
 * "Helping"
 ***********************************************************************/

/*
 * Set time based on UNIX timestamps shat out on the serial port.
 */
#if SERIAL_TIME_SET
void timeset() {
  time_t newnow = 0;

  //Serial.setTimeout(3000);
  if (Serial.available()) {
    if (Serial.find("T")) {
      newnow = Serial.parseInt();
    }
  }

  if (newnow != 0) {
    rtc.adjust(DateTime(newnow));
  }

}
#endif

/*
 * Barf error info to display
 */
void error(const char *msg, int pause=5000) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
  Serial.println(msg);
  delay(pause);
}

/*
 * Display current temp, humidity, date and time.
 */
void refreshDisplay(float t, float rh, DateTime *dt) {

  // Format strings, stored in program memory to avoid wasting precious
  // precious RAMz.
  PGM_P FMT = PSTR(
         "Temp %#5.1f\367C"
    "\n" "RH    %#5.2f%%"
    "\n" "%04d-%02d-%02d" " " "%02d:%02d:%02d"
  );

  // String-ify display data in a C-ish style way since the Arduino libs don't
  // really give us any way to handle this well (which is annoying). Note that
  // special linker flags are required to bring in the full vprintf
  // implementation for floating point number. Check the Makefile for the
  // voodoo.
  char buff[64];
  snprintf_P(buff, sizeof(buff), FMT, t, rh,
    dt->year(),
    dt->month(),
    dt->day(),
    dt->hour(),
    dt->minute(),
    dt->second()
  );
  buff[sizeof(buff)-1] = '\0';

  // Actually shart chars to thingy
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(buff);
  display.display();

}

/*
 * String-ify display data in a C-ish style way since the Arduino libs don't
 * really give us any way to handle this well (which is annoying). Note that
 * special linker flags are required to bring in the full vprintf
 * implementation for floating point number. Check the Makefile for the
 * voodoo.
 */
void formatString(float temperature, float rh, DateTime *dt, char *buff, size_t bufflen) {

  // Format strings, stored in program memory to avoid wasting precious
  // precious RAMz.
  PGM_P FMT = PSTR(
    "%04d-%02d-%02d %02d:%02d:%02d," //timestamp
    "%#6.1f,"                        //temperature
    "%#6.2f,"                        //relative humidity
    "\r\n"
  );

  snprintf_P(buff, bufflen, FMT,
    dt->year(),
    dt->month(),
    dt->day(),
    dt->hour(),
    dt->minute(),
    dt->second(),
    temperature,
    rh
  );
  buff[bufflen-1] = '\0';

}

/***********************************************************************
 * setup() - this is done once
 ***********************************************************************/

void setup() {

  // For remote error reporting
  delay(500);
  Serial.begin(9600);

  // Boot up display
  Wire.begin();
  display.begin(DISPLAY_MODE, DISPLAY_ADDR);
  display.clearDisplay();
  display.setTextColor(DISPLAY_COLOR);
  display.setTextSize(DISPLAY_TEXTSIZE);
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
  /*
  pinMode(SD_CS, OUTPUT);
  while (!SD.begin(SD_CS, SD_MOSI, SD_MISO, SD_SCK)) {
    error("Could not connect microSD card!");
  }
  logfile = SD.open(LOGFILE, LOGMODE);
  while (!logfile) {
    error("Error opening " LOGFILE "!");
  }
  */

  // Setup serial logging, too, while we're at it
  Serial.println("timestamp,temperature,relative_humidity");

}

/***********************************************************************
 * loop() - this is done all the time (like your mom)
 ***********************************************************************/

void loop() {
  
#if SERIAL_TIME_SET
  // Set time if compiled in
  timeset();
#endif

  float temperature = sensor_temp.readTempC();
  float rh = sensor_rh.readHumidity();
  DateTime now = rtc.now();
  
  // Update display
  refreshDisplay(temperature, rh, &now);

  // Log to logfile & shart to serial
  char buff[64];
  formatString(temperature, rh, &now, buff, sizeof(buff));
  //logfile.write(buff);
  //logfile.flush();
  Serial.print(buff);
  Serial.flush();

  // Pause for a sec before running again
  delay(LOG_INTERVAL_MS);

}

ARDUINO_LIBS  := Adafruit_GFX Adafruit_SSD1306 Adafruit_Sensor Adafruit_SD \
                 SPI Wire RTClib RTClib/DS1307 Time Timezone \
								 Adafruit_MCP9808 Adafruit_HTU21DF
BOARD_TAG     := uno
ARDUINO_PORT  := /dev/ttyUSB0
USER_LIB_PATH := $(realpath ./lib)
CXXFLAGS      += -std=gnu++11 -Wl,-u,vfprintf
CFLAGS        += -std=gnu++11 -Wl,-u,vfprintf
LDFLAGS       += -lprintf_flt -lm -Wl,-u,vfprintf

include $(realpath ./Arduino.mk)

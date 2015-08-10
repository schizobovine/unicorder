#!/usr/bin/env python

from datetime import datetime
import serial
import sys
import time

SERIAL_BAUD = 9600
SERIAL_PORT = '/dev/ttyUSB0'
TIME_FORMAT = "T%s"

# Reset device to activate time setting routine
DO_RST = True

# Open serial dong
print 'opening serial port %s...' % SERIAL_PORT
uart = serial.Serial(
    port=SERIAL_PORT,
    baudrate=SERIAL_BAUD,
    dsrdtr=DO_RST,
)

# Frobulate the DTR pin to reset the target
if DO_RST:
    print 'twiddling DTR to reset'
    uart.setRTS(False)
    uart.setDTR(False)
    uart.flush()
    time.sleep(0.2)
    uart.flushInput()
    uart.setRTS(True)
    uart.setDTR(True)
    time.sleep(1)
    print 'reset done'

# Send start command to begin cycle
time.sleep(1)
for i in xrange(0, 30):
    time.sleep(0.1)
    now = datetime.now().strftime(TIME_FORMAT)
    uart.write(now + "\r\n")
    uart.flush()
uart.close()

print 'done!'
sys.exit(0)

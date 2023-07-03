comp:
	arduino-cli compile -b m5stack:esp32:m5stick-c-plus multifunctional-clock-M5.ino --warnings default

upload:
	arduino-cli upload -p /dev/ttyUSB0 -b m5stack:esp32:m5stick-c-plus multifunctional-clock-M5.ino

run:
	arduino-cli compile -p /dev/ttyUSB0 -b m5stack:esp32:m5stick-c-plus multifunctional-clock-M5.ino -u --warnings all

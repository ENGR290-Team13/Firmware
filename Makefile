MCU ?= atmega328p
F_CPU ?= 16000000UL
PORT ?= COM9
UPLOAD_BAUD ?= 57600
SOURCES := src/main.c src/mpu6050.c src/TWI_290.c
HEADERS := $(wildcard include/*.h)
CC := avr-gcc
CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -std=gnu11 -Wall -Wextra -Iinclude

.PHONY: all clean flash
all: build/firmware.hex

build:
	mkdir -p build

build/firmware.elf: $(SOURCES) $(HEADERS) | build
	$(CC) $(CFLAGS) $(SOURCES) -o $@ -lm

build/firmware.hex: build/firmware.elf
	avr-objcopy -O ihex -R .eeprom $< $@
	avr-size --format=avr --mcu=$(MCU) build/firmware.elf

flash: build/firmware.hex
	@test -n "$(PORT)" || (echo "Usage: mingw32-make flash PORT=COM9"; exit 1)
	avrdude -p m328p -c arduino -P "$(PORT)" -b $(UPLOAD_BAUD) -D -U flash:w:build/firmware.hex:i

clean:
	rm -rf build

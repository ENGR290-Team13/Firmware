#source: https://siliconwit.com/education/embedded-programming-atmega328p/avr-toolchain-bare-metal-setup/#project-structure


MCU ?= atmega328p
AVRDUDE_MCU ?= m328p
F_CPU ?= 16000000UL

UPLOAD_BAUD ?= 57600
PORT ?=

# Keep MSYS2 tools and their DLLs ahead of other Windows GCC installations.
ifeq ($(OS),Windows_NT)
MSYS2_BIN ?= C:/msys64/ucrt64/bin
ifneq ($(wildcard $(MSYS2_BIN)/avr-gcc.exe),)
export PATH := $(MSYS2_BIN);C:/msys64/usr/bin;$(PATH)
AVR_BIN := $(MSYS2_BIN)/
endif
# Recipes below use POSIX shell syntax, including the flash port check.
ifneq ($(wildcard C:/msys64/usr/bin/sh.exe),)
SHELL := C:/msys64/usr/bin/sh.exe
else
SHELL := sh
endif
endif

AVR_CC ?= $(AVR_BIN)avr-gcc
AVR_OBJCOPY ?= $(AVR_BIN)avr-objcopy
AVR_SIZE ?= $(AVR_BIN)avr-size
AVRDUDE ?= $(AVR_BIN)avrdude

BUILD_DIR := build
TARGET := $(BUILD_DIR)/firmware
ELF := $(TARGET).elf
HEX := $(TARGET).hex

# One entry point: the ultrasonic assignment in src/main.c.
# Reference/ and archived assignment files are not firmware sources.
SOURCES := src/main.c src/init_290.c
HEADERS := $(wildcard include/*.h)

CFLAGS := -mmcu=$(MCU) \
          -DF_CPU=$(F_CPU) \
          -Os \
          -Wall \
          -Wextra \
          -Iinclude

.PHONY: all clean flash

all: $(HEX)

$(BUILD_DIR):
	$(SHELL) -c "mkdir -p $(BUILD_DIR)"

$(ELF): $(SOURCES) $(HEADERS) Makefile | $(BUILD_DIR)
	$(AVR_CC) $(CFLAGS) $(SOURCES) -o $@

$(HEX): $(ELF)
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@
	$(AVR_SIZE) --format=avr --mcu=$(MCU) $(ELF)

flash: $(HEX)
	@if [ -z "$(PORT)" ]; then echo "Set PORT, for example: make flash PORT=COM4"; exit 2; fi
	$(AVRDUDE) -p $(AVRDUDE_MCU) -c arduino -P "$(PORT)" -b $(UPLOAD_BAUD) -D -U flash:w:$(HEX):i

clean:
	rm -rf $(BUILD_DIR)

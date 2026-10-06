#source: https://siliconwit.com/education/embedded-programming-atmega328p/avr-toolchain-bare-metal-setup/#project-structure


MCU ?= atmega328p
AVRDUDE_MCU ?= m328p
F_CPU ?= 16000000UL

UPLOAD_BAUD ?= 115200
PORT ?=

AVR_CC ?= avr-gcc
AVR_OBJCOPY ?= avr-objcopy
AVR_SIZE ?= avr-size
AVRDUDE ?= avrdude

BUILD_DIR := build
TARGET := $(BUILD_DIR)/firmware
ELF := $(TARGET).elf
HEX := $(TARGET).hex

# One entry point: the ultrasonic assignment in src/main.c.
# Reference/ and archived assignment files are not firmware sources.
SOURCES := src/main.c
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
	mkdir -p $(BUILD_DIR)

$(ELF): $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(AVR_CC) $(CFLAGS) $(SOURCES) -o $@

$(HEX): $(ELF)
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@
	$(AVR_SIZE) --format=avr --mcu=$(MCU) $(ELF)

flash: $(HEX)
	@if [ -z "$(PORT)" ]; then echo "Set PORT, for example: make flash PORT=COM4"; exit 2; fi
	$(AVRDUDE) -p $(AVRDUDE_MCU) -c arduino -P "$(PORT)" -b $(UPLOAD_BAUD) -D -U flash:w:$(HEX):i

clean:
	rm -rf $(BUILD_DIR)
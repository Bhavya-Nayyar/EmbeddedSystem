SHELL := cmd.exe

MCU = atmega328p
F_CPU = 16000000UL
PORT = COM6     
BAUD = 115200

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

CFLAGS = -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU)

TARGET = src/main

all: $(TARGET).hex

$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).c

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

upload: $(TARGET).hex
	$(AVRDUDE) -F -V -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$(TARGET).hex:i

clean:
	del /q /f src\main.elf src\main.hex
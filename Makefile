MCU = atmega328p
PROGRAMMER = usbtiny

CFLAGS += -mmcu=$(MCU) -Wall -Wextra -Wfatal-errors -I src

SOURCES = $(shell find src -name '*.c') $(shell find src -name '*.s')
HEADERS = $(shell find src -name '*.h')

main: $(SOURCES) $(HEADERS)
	avr-gcc -O3 $(CFLAGS) $(SOURCES) -o main

.PHONY: clean
clean:
	rm -f main

.PHONY: avrdude-write
avrdude-write: main
	avrdude -c $(PROGRAMMER) -p $(MCU) -U flash:w:main

.PHONY: avrdude-ping
avrdude-ping:
	avrdude -c $(PROGRAMMER) -p $(MCU)

DEVICE = attiny85
F_CPU = 16500000

AVRCC = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

OBJECTS = main.o r433.o timer.o tx_uart.o w1.o

all: main.hex

main.elf: $(OBJECTS)
	$(AVRCC) -o main.elf $(OBJECTS)

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size main.hex

clean:
	rm -f $(OBJECTS) main.elf main.hex

# Build rules to use AVRCC rather than the host CC

.c.o:
	$(AVRCC) -c $< -o $@

.S.o:
	$(AVRCC) -x assembler-with-cpp -c $< -o $@

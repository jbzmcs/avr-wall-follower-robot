DEVICE     = atmega328p
CLOCK      = 16000000
PROGRAMMER = -c arduino -b 115200 -P COM6
FUSES      = -U hfuse:w:0xde:m -U lfuse:w:0xff:m -U efuse:w:0x05:m

OUT_DIR = out

# File names
SRC = $(wildcard *.c) $(wildcard drivers/*.c)

# Add Linker Flags
LDFLAGS = -Wl,-u,vfprintf -lprintf_flt -lm

# Automatically prefix the output directory to the object files
OBJECTS = $(addprefix $(OUT_DIR)/, $(notdir $(SRC:.c=.o)))

# Tune the lines below only if you know what you are doing:
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -Idrivers

# symbolic targets:
all:	$(OUT_DIR)/main.hex

# Target to create the output directory
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Pattern rule for compiling C files into the out directory
$(OUT_DIR)/%.o: %.c | $(OUT_DIR)
	$(COMPILE) -c $< -o $@

# Pattern rule for compiling driver C files into the out directory
$(OUT_DIR)/%.o: drivers/%.c | $(OUT_DIR)
	$(COMPILE) -c $< -o $@

# Pattern rule for compiling Assembly files into the out directory
$(OUT_DIR)/%.o: %.S | $(OUT_DIR)
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

# Pattern rule for generating assembly output from C files
$(OUT_DIR)/%.s: %.c | $(OUT_DIR)
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:$(OUT_DIR)/main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID $(OUT_DIR)/main.hex

clean:
	rm -rf $(OUT_DIR)

# file targets:
$(OUT_DIR)/main.elf: $(OBJECTS)
	$(COMPILE) -o $@ $(OBJECTS) $(LDFLAGS)

$(OUT_DIR)/main.hex: $(OUT_DIR)/main.elf
	rm -f $@
	avr-objcopy -j .text -j .data -O ihex $< $@
	avr-size --format=avr --mcu=$(DEVICE) $<

# Targets for code debugging and analysis:
disasm:	$(OUT_DIR)/main.elf
	avr-objdump -d $<

cpp:
	$(COMPILE) -E main.c
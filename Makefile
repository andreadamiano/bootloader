MCU   = atmega328p        
F_CPU = 16000000UL        
BAUD  = 9600UL     
LIBDIR = src     
BOOTLOADER_DIR = src/bootloader
APPLICATION_DIR = src/application
UTILS_DIR = src/utils
PROGRAMMER_TYPE = usbasp    
PROGRAMMER_ARGS = 	
CC = avr-gcc             
OBJCOPY = avr-objcopy    
OBJDUMP = avr-objdump    
AVRSIZE = avr-size       
AVRDUDE = avrdude        
TARGET = main
APPLICATION_TARGET = main_app
BOOTLOADER_TARGET = main_boot
BOOTLOADER_INIT = 0x7000
APPLICATION_INIT = 0x800200


BOOTLOADER_SOURCES := $(shell find $(UTILS_DIR) -name '*.c') $(shell find $(BOOTLOADER_DIR) -name '*.c')
APPLICATION_SOURCES := $(shell find $(APPLICATION_DIR) -name '*.c') $(shell find $(UTILS_DIR) -name '*.c')
INC_LIST = $(addprefix -I, $(sort $(dir $(shell find $(UTILS_DIR) -name '*.h'))))
BOOTLOADER_OBJECTS=$(BOOTLOADER_SOURCES:src/%.c=$(BUILD_DIR)/%.o)   # Convert .c filenames to .o filenames
APPLICATION_OBJECTS=$(APPLICATION_SOURCES:src/%.c=$(BUILD_DIR)/%.o)    # Convert .c filenames to .o filenames
APPLICATION_DEPENDENCIES = $(APPLICATION_OBJECTS:.o=.d) 
BOOTLOADER_DEPENDENCIES = $(BOOTLOADER_OBJECTS:.o=.d) 


CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I. -I$(LIBDIR)  # Defines F_CPU & BAUD macros, adds include paths
CFLAGS = -Os -g -std=gnu99 -Wall   
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += -ffunction-sections -fdata-sections 
LDFLAGS += -Wl,--gc-sections        # Remove unused code sections (smaller final program) 
TARGET_ARCH = -mmcu=$(MCU)
BUILD_DIR = build

# BOOTLOADER_LDFLAGS += $(LDFLAGS) -Wl,--section-start=.text=0x7800
BOOTLOADER_LDFLAGS += $(LDFLAGS) -Wl,--section-start=.text=$(BOOTLOADER_INIT)
BOOTLOADER_LDFLAGS += -Wl,--section-start=.noinit=$(APPLICATION_INIT)  #after the first 200 hex (since i dont't increase the heap is safe, if i would it would override this section
BOOTLOADER_LDFLAGS += -Wl,-Map,$(BUILD_DIR)/$(BOOTLOADER_TARGET).map # Generate memory map file showing memory layout
APPLICATION_LDFLAGS = $(LDFLAGS)
APPLICATION_LDFLAGS += -Wl,--section-start=.noinit=$(APPLICATION_INIT)
APPLICATION_LDFLAGS += -Wl,-Map,$(BUILD_DIR)/$(APPLICATION_TARGET).map # Generate memory map file showing memory layout


application: $(BUILD_DIR)/$(APPLICATION_TARGET).hex 
bootloader: $(BUILD_DIR)/$(BOOTLOADER_TARGET).hex 

# Object file creation
$(BUILD_DIR)/%.o: src/%.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $(INC_LIST) -MMD -MP -c -o $@ $<

## LINKING: Combines all object files into single ELF executable
$(BUILD_DIR)/$(APPLICATION_TARGET).elf: $(APPLICATION_OBJECTS)
	$(CC) $(APPLICATION_LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/$(BOOTLOADER_TARGET).elf: $(BOOTLOADER_OBJECTS)
	$(CC) $(BOOTLOADER_LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

## ELF TO HEX CONVERSION: Creates Intel HEX file for programming flash memory
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

# Merge both hex files into one for flashing
$(BUILD_DIR)/combined.hex: $(BUILD_DIR)/$(APPLICATION_TARGET).hex $(BUILD_DIR)/$(BOOTLOADER_TARGET).hex
	@echo "Creating combined hex file..."
	@srec_cat $(BUILD_DIR)/$(APPLICATION_TARGET).hex -Intel \
	          $(BUILD_DIR)/$(BOOTLOADER_TARGET).hex -Intel \
	          -o $(BUILD_DIR)/combined.hex -Intel
	@echo "Combined hex file created: $(BUILD_DIR)/combined.hex"

-include $(APPLICATION_DEPENDENCIES)
-include $(BOOTLOADER_DEPENDENCIES)


## EEPROM EXTRACTION: Creates separate file for EEPROM programming
%.eeprom: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@  

## ASSEMBLY LISTING: Creates human-readable assembly code from ELF
%.lst: %.elf
	$(OBJDUMP) -S $< > $@

## PHONY TARGETS: These targets don't create files with the same names
.PHONY: all disassemble disasm eeprom size clean squeaky_clean flash fuses

all: application bootloader

# ## DEVELOPMENT/DEBUGGING TARGETS
# debug:    # Shows current configuration - useful for troubleshooting
# 	@echo
# 	@echo "Source files:"   $(SOURCES)
# 	@echo "MCU, F_CPU, BAUD:"  $(MCU), $(F_CPU), $(BAUD)
# 	@echo	

# Creates assembly listing - useful for debugging timing-critical code
# and verifying the compiler does what you expect
disassemble: $(TARGET).lst

disasm: disassemble    # Short alias for disassemble

# Shows memory usage - critical for embedded systems with limited resources
size:  $(BUILD_DIR)/$(TARGET).elf
	$(AVRSIZE) -C --mcu=$(MCU) $(BUILD_DIR)/$(TARGET).elf

## CLEANUP TARGETS
clean:    # Remove build files but keep source code
	rm -rf $(BUILD_DIR)/*

squeaky_clean:    # Remove ALL generated files (more thorough than clean)
	rm -f *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom

##########------------------------------------------------------##########
##########              Programmer-specific details             ##########
##########           Flashing code to AVR using avrdude         ##########
##########------------------------------------------------------##########
clean_flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -e -U flash:w:$<
	
## MAIN PROGRAMMING TARGETS
# flash: $(BUILD_DIR)/$(TARGET).hex     # Program the flash memory with your compiled code
# 	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<


# Program application (low addresses) first, then bootloader (high addresses)
flash: $(BUILD_DIR)/$(APPLICATION_TARGET).hex $(BUILD_DIR)/$(BOOTLOADER_TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) \
	-U flash:w:$(BUILD_DIR)/$(APPLICATION_TARGET).hex:i \
	-U flash:w:$(BUILD_DIR)/$(BOOTLOADER_TARGET).hex:i

application_binary:
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(APPLICATION_TARGET).elf $(BUILD_DIR)/$(APPLICATION_TARGET).bin

program: flash    # Alias for flash - same functionality

flash_eeprom: $(TARGET).eeprom    # Program EEPROM memory (persistent data storage)
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U eeprom:w:$<

avrdude_terminal:    # Interactive terminal mode for direct AVR communication
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -nt

## MULTIPLE PROGRAMMER SUPPORT - Easy switching between different programmers
## Usage: make flash_usbtiny, make flash_usbasp, etc.
flash_usbtiny: PROGRAMMER_TYPE = usbtiny    # USBTiny - simple, cheap programmer
flash_usbtiny: PROGRAMMER_ARGS =  # USBTiny works with no further arguments
flash_usbtiny: flash

flash_usbasp: PROGRAMMER_TYPE = usbasp    # USBasp - another popular choice
flash_usbasp: PROGRAMMER_ARGS =  # USBasp works with no further arguments
flash_usbasp: flash

flash_arduinoISP: PROGRAMMER_TYPE = avrisp    # Use Arduino as In-System Programmer
flash_arduinoISP: PROGRAMMER_ARGS = -b 19200 -P /dev/ttyACM0 
## (for windows) flash_arduinoISP: PROGRAMMER_ARGS = -b 19200 -P com5
flash_arduinoISP: flash

flash_109: PROGRAMMER_TYPE = avr109    # Bootloader-based programming
flash_109: PROGRAMMER_ARGS = -b 9600 -P /dev/ttyUSB0
flash_109: flash

##########------------------------------------------------------##########
##########       Fuse settings and suitable defaults            ##########
##########    WARNING: Wrong fuse settings can brick your AVR!  ##########
##########   Consult http://www.engbedded.com/fusecalc first!    ##########
##########------------------------------------------------------##########

## DEFAULT FUSE VALUES for ATmega48, 88, 168, 328 series
LFUSE = 0x62    # Low fuse: Internal 1MHz RC oscillator, slowly rising power
HFUSE = 0xdf    # High fuse: Standard settings, no bootloader
EFUSE = 0x00    # Extended fuse: No brown-out detection

## FUSE PROGRAMMING COMMANDS
FUSE_STRING = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m 

fuses:     # DANGEROUS: Program fuse bytes - can brick your AVR if wrong!
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) \
	           $(PROGRAMMER_ARGS) $(FUSE_STRING)
	           
read_fuses:    # Safe: Read and display current fuse settings
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

write_fuses:
	@echo "lfuse:"; \
	read lfuse; \
	echo "hfuse:"; \
	read hfuse; \
	echo "efuse:"; \
	read efuse; \
	avrdude -c usbasp -p atmega328p -U lfuse:w:$$lfuse:m -U hfuse:w:$$hfuse:m -U efuse:w:$$efuse:m


## PRESET FUSE CONFIGURATIONS
set_default_fuses:  FUSE_STRING = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m 
set_default_fuses:  fuses    # Reset to factory defaults

## Enable external 16MHz crystal - much more accurate timing than internal RC
set_fast_fuse: LFUSE = 0xE2    # External crystal/resonator, fast startup
set_fast_fuse: FUSE_STRING = -U lfuse:w:$(LFUSE):m 
set_fast_fuse: fuses

## Preserve EEPROM data during chip erase/reprogramming
set_eeprom_save_fuse: HFUSE = 0xD7    # EESAVE bit cleared
set_eeprom_save_fuse: FUSE_STRING = -U hfuse:w:$(HFUSE):m
set_eeprom_save_fuse: fuses

## Allow EEPROM to be erased during programming (default behavior)
clear_eeprom_save_fuse: FUSE_STRING = -U hfuse:w:$(HFUSE):m
clear_eeprom_save_fuse: fuses


reset:
	echo "send r" | avrdude -p m328p -c usbasp -t


connect_h05:
	sudo echo "Getting sudo access..."
	sudo rfcomm connect /dev/rfcomm0 98:D3:31:F7:15:AA

bind_h05:
	sudo rfcomm bind /dev/rfcomm0 98:D3:31:F7:15:AA

release_h05:
	sudo rfcomm release /dev/rfcomm0

debug:
	python3 debug.py
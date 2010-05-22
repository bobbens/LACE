#########################################
#
#       CHIP DETAILS + AVRDUDE
#
AVRDUDE              := avrdude
MCU                  := atmega168p
PROGRAMMER_MCU       := m168p
AVRDUDE_PROGRAMMERID := usbasp
#AVRDUDE_PORT         := /dev/parport0


#########################################
#
#       INTERNAL USAGE
#
AVRDUDE_FLAGS := -p $(PROGRAMMER_MCU) -c $(AVRDUDE_PROGRAMMERID) -v
ifdef AVRDUDE_PORT
AVRDUDE_FLAGS +=-P $(AVRDUDE_PORT)
endif



#########################################
#
#       FUSES
#
EFUSE := 0xFC
#       7 6 5 4 3 2 1 0
# efuse 1 1 1 1 1 1 0 0 = 0xFC
#       | | | | | | | |
#       | | | | | | | +-> BODLEVEL0 \
#       | | | | | | |               |
#       | | | | | | +---> BODLEVEL1 |-> BOD Enabled
#       | | | | | |                 |   Min 4.1V  Typical 4.3V  Max 4.5V
#       | | | | | +-----> BODLEVEL2 /
#       | | | | |                
#       +-+-+-+-+-------> Unused
#
HFUSE := 0xD9
#       7 6 5 4 3 2 1 0
# hfuse 1 1 0 1 1 0 0 1 = 0xD9
#       | | | | | | | |
#       | | | | | | | +-> BOOTRST --> Unprogrammed
#       | | | | | | |
#       | | | | | | +---> BOOTSZ0 --> Unprogrammed
#       | | | | | |
#       | | | | | +-----> BOOTSZ1 --> Unprogrammed
#       | | | | |
#       | | | | +-------> EESAVE ---> Unprogrammed
#       | | | |
#       | | | +---------> WDTON ----> Unprogrammed, not always on
#       | | |
#       | | +-----------> SPIEN ----> SPI enabled
#       | |
#       | +-------------> JTAGEN ---> JTAG disalbed
#       |
#       +---------------> OCDEN ----> OCD Disabled
#
LFUSE := 0xD7
#       7 6 5 4 3 2 1 0
# lfuse 1 1 0 1 0 1 1 1 = 0xD7
#       | | | | | | | |
#       | | | | | | | +-> CKSEL0 \
#       | | | | | | |            |
#       | | | | | | +---> CKSEL1 |
#       | | | | | |              |--> External Crystal Oscillator at 8MHz<
#       | | | | | +-----> CKSEL2 |
#       | | | | |                |
#       | | | | +-------> CKSEL3 /
#       | | | |
#       | | | +---------> SUT0  \
#       | | |                   |---> Crystal Oscillator, BOD enabled
#       | | +-----------> SUT1  /
#       | |
#       | +-------------> CKOUT ----> No clock output
#       |
#       +---------------> CKDIV8 ---> Don't divide by 8
AVRDUDE_READ_FUSE := \
	-U lfuse:r:-:i \
	-U hfuse:r:-:i \
	-U efuse:r:-:i \
	-U lock:r:-:i 
AVRDUDE_WRITE_FUSE := \
	-U lfuse:w:$(LFUSE):m \
	-U hfuse:w:$(HFUSE):m \
	-U efuse:w:$(EFUSE):m



#########################################
#
#	COMPILING C
#
CC			:= avr-gcc
OPTLEVEL := 2
CFLAGS	:= \
			-g 						\
			-mmcu=$(MCU)			\
			-O$(OPTLEVEL)			\
			-fpack-struct			\
			-fshort-enums			\
			-funsigned-bitfields	\
			-funsigned-char		\
			-W							\
			-Wall						\
			-Wextra              \
			-Wstrict-prototypes	\
			-I.                  \
			-I..



#########################################
#
#       MISC VARIABLES
#
OBJCOPY  := avr-objcopy
OBJDUMP  := avr-objdump
HEXFORMAT := ihex
COPYOPTS :=		-j .text			\
		         -j .data			\
		         -O $(HEXFORMAT)
SIZE     := avr-size
AVRSIM   := simulavr
GDB      := avr-gdb
PORT     := 1212


#########################################
#
#	PROJECT DETAILS
#
PROJECT  := lace_hbridge

PRG      := $(PROJECT)

SRC      := adc.c comm.c uart.c spis.c core.c encoder.c motors.c
ASRC     := 
GCCLIB   :=
OBJS     := $(SRC:.c=.o)

GDBINITFILE := gdbinit-$(PRG)

#########################################
#
#       MAKE FUNCTIONS
#
.PHONY: all hex elf writeflash fuses verify install sim clean


all: $(PRG).hex

hex: $(PRG).hex
elf: $(PRG).elf

%.o:		%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PRG).elf:	$(OBJS)
	$(CC) $(CFLAGS) -o $(PRG).elf $(OBJS) $(GCCLIB)

$(PRG).hex:	$(PRG).elf
	$(OBJCOPY) $(COPYOPTS) $< $@

writeflash:	hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
			-e \
			-U flash:w:$(PROJECT).hex:i

verify:		hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
			-n	\
			-D	\
			-U flash:v:$(PROJECT).hex:i

fuses-read:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
			-n	\
			$(AVRDUDE_READ_FUSE)

fuses-write:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
			-u \
			$(AVRDUDE_WRITE_FUSE)

install:	writeflash

sim:		hex
	$(AVRSIM) --gdbserver --device $(MCU) --port $(PORT) &
	
	@echo "file $(PRG).elf" > $(GDBINITFILE)
	@echo "target remote localhost:$(PORT)" \
		>> $(GDBINITFILE)
	@echo "load"        >> $(GDBINITFILE) 
	@echo "break main"  >> $(GDBINITFILE)
	@echo "continue"    >> $(GDBINITFILE)

	$(GDB) -x $(GDBINITFILE)

clean:
	$(RM) $(OBJS) $(PRG).elf $(PRG).hex

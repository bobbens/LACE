#########################################
#
#		 CHIP DETAILS + AVRDUDE
#
MCU				:= atmega644p
PROGRAMMER_MCU := m644p
AVRDUDE_PROGRAMMERID := usbasp
AVRDUDE_PORT	:= /dev/parport0


#########################################
#
#	COMPILING C
#
CC				 := avr-gcc
OPTLEVEL		 := s
CFLAGS			:= -mmcu=$(MCU)			\
						-O$(OPTLEVEL)			\
						-fpack-struct			\
						-fshort-enums			\
						-funsigned-bitfields	\
						-funsigned-char		\
						-W							\
						-Wall						\
						-Wextra				  \
						-Wstrict-prototypes	\
						-I.						\
						-I..					  \
						-I../modules


#########################################
#
#  		AVRDUDE
#
AVRDUDE_FLAGS := -p $(PROGRAMMER_MCU) -c $(AVRDUDE_PROGRAMMERID) -v
ifdef AVRDUDE_PORT
AVRDUDE_FLAGS += -P $(AVRDUDE_PORT)
endif


#########################################
#
#       FUSES
#
EFUSE := 0xFD
#       7 6 5 4 3 2 1 0
# efuse 1 1 1 1 1 1 0 1 = 0xFD
#       | | | | | | | |
#       | | | | | | | +-> BODLEVEL0 \
#       | | | | | | |               |
#       | | | | | | +---> BODLEVEL1 |-> BOD Enabled
#       | | | | | |                 |   Min 2.5V  Typical 2.7V  Max 2.9V
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
#	CLEANING UP
#
REMOVE			:= $(RM)



#########################################
#
#		 MISC VARIABLES
#
AVRDUDE			:= avrdude
OBJCOPY		  := avr-objcopy
OBJDUMP		  := avr-objdump
HEXFORMAT		:= ihex
COPYOPTS		 :=		-j .text			\
							-j .data			\
							-O $(HEXFORMAT)
SIZE			  := avr-size
AVRSIM			:= simulavr
GDB				:= avr-gdb
PORT			  := 1212


#########################################
#
#	PROJECT DETAILS
#
PROJECT		  := lace_motherboard

PRG				:= $(PROJECT)

SRC				:= core.c uart.c comm.c event.c timer.c pwm.c adc.c spim.c i2cm.c fsm.c mod.c mod/dhb.c wmp.c

OBJS			  := $(SRC:.c=.o) $(AVRLIB:.c=.o) 

GDBINITFILE	 := gdbinit-$(PRG)

#########################################
#
#		 MAKE FUNCTIONS
#
.PHONY: all hex elf writeflash verify install fuses-read fuses-write sim clean

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
	$(AVRDUDE) -p $(PROGRAMMER_MCU)	\
			-e			\
			-c $(AVRDUDE_PROGRAMMERID) \
			-P $(AVRDUDE_PORT)	\
			-v			\
			-U flash:w:$(PROJECT).hex:i

verify:		hex
	$(AVRDUDE) -p $(PROGRAMMER_MCU)			\
			-c $(AVRDUDE_PROGRAMMERID) \
			-P $(AVRDUDE_PORT)		\
			-n			\
			-v							 \
			-D			\
			-U flash:v:$(PROJECT).hex:i


install:	writeflash

fuses-read:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
			-n	\
			$(AVRDUDE_READ_FUSE)

fuses-write:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
			-u \
			$(AVRDUDE_WRITE_FUSE)

sim:		hex
	$(AVRSIM) --gdbserver --device $(MCU) --port $(PORT) &
	
	@echo "file $(PRG).elf" > $(GDBINITFILE)
	@echo "target remote localhost:$(PORT)" \
		>> $(GDBINITFILE)
	@echo "load"		  >> $(GDBINITFILE) 
	@echo "break main"  >> $(GDBINITFILE)
	@echo "continue"	 >> $(GDBINITFILE)

	$(GDB) -x $(GDBINITFILE)

clean:
	$(REMOVE) $(OBJS) $(PRG).elf $(PRG).hex

dox:
	@doxygen
	@make -C latex refman.pdf
	@mv latex/refman.pdf doc/rBrain.pdf
	@rm -r latex/


#########################################
#
#		 CHIP DETAILS + AVRDUDE
#
MCU				:= atmega324p
PROGRAMMER_MCU := m324p
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

SRC				:= core.c uart.c comm.c event.c timer.c pwm.c adc.c spim.c mod.c mod/dhb.c


OBJS			  := $(SRC:.c=.o) $(AVRLIB:.c=.o) 

GDBINITFILE	 := gdbinit-$(PRG)

#########################################
#
#		 MAKE FUNCTIONS
#
.PHONY: all hex elf writeflash verify install sim clean

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


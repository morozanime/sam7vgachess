
PLATFORM = SAM7X
BUILD = build
TOOLCHAIN_BASE_DIR = "C:/Program Files (x86)/IAR Systems/Embedded Workbench 6.4 Kickstart/arm"
SRC_DIR = .

TARGET = $(BUILD)/$(PLATFORM)/main

ASRC =
ASRC += $(SRC_DIR)/at91lib/board/board_cstartup_iar.s

CSRC =
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/irq/aic.c
CSRC	+=	$(SRC_DIR)/at91lib/board/board_lowlevel.c
CSRC	+=	$(SRC_DIR)/at91lib/board/board_memories.c
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/dbgu/dbgu.c
CSRC	+=	$(SRC_DIR)/main.c
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/pio/pio.c
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/pio/pio_it.c
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/pit/pit.c
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/tc/tc.c
CSRC	+=	$(SRC_DIR)/at91lib/utility/trace.c
CSRC	+=	$(SRC_DIR)/at91lib/peripherals/spi/spi.c
#CSRC	+=	$(SRC_DIR)/life.c
#CSRC	+=	$(SRC_DIR)/myLCD.c
CSRC	+=	$(SRC_DIR)/8X8WIN1251.c
CSRC	+=	$(SRC_DIR)/chess.c
CSRC	+=	$(SRC_DIR)/chess48.c
CSRC	+=	$(SRC_DIR)/validate.c
CSRC	+=	$(SRC_DIR)/ps2mouse.c

#--------------------------------------------------------------------
OBJ	= $(CSRC:.c=.o) $(ASRC:.s=.o)

INCLUDE_DIRS =
INCLUDE_DIRS +=	|$(SRC_DIR)/at91lib/peripherals/
INCLUDE_DIRS += |$(SRC_DIR)/at91lib/components/
INCLUDE_DIRS += |$(SRC_DIR)/at91lib/
#INCLUDE_DIRS += |$(TOOLCHAIN_BASE_DIR)/examples/Atmel/external_libs/
INCLUDE_DIRS += |$(SRC_DIR)/at91lib/board/

DEFINES =
DEFINES += |at91sam7x128
DEFINES += |flash
DEFINES += |TRACE_LEVEL=4

CFLAGS	=
CFLAGS	+= $(subst |,-D,$(DEFINES))
CFLAGS	+= --remarks
CFLAGS	+= --diag_suppress Pe826,Pe1375
CFLAGS	+= --endian=little
CFLAGS	+= --cpu=ARM7TDMI
CFLAGS	+= -e
CFLAGS	+= --fpu=None
CFLAGS	+= --dlib_config $(TOOLCHAIN_BASE_DIR)/INC/c/DLib_Config_Full.h
CFLAGS	+= $(subst |,-I,$(INCLUDE_DIRS))
CFLAGS	+= --interwork
CFLAGS	+= --cpu_mode thumb
CFLAGS	+= -Ohs
CFLAGS	+= --use_c++_inline
CFLAGS	+= -lC $(BUILD)/$(PLATFORM)/

LFLAGS  = 
LFLAGS	+= --redirect _Printf=_PrintfFull
LFLAGS	+= --redirect _Scanf=_ScanfFull
LFLAGS  += --strip
LFLAGS	+= --map $(BUILD)/$(PLATFORM)/
LFLAGS	+= --config $(SRC_DIR)/at91lib/board/at91sam7x128/flash
LFLAGS	+= --diag_suppress Lp005,Lp006
LFLAGS	+= --semihosting
LFLAGS	+= --entry resetHandler
LFLAGS	+= --redirect __write=__write_buffered
LFLAGS	+= --vfe

ASFLAGS =
ASFLAGS += -s+
#ASFLAGS += -M<>
ASFLAGS += -w+
ASFLAGS += -r
ASFLAGS	+= $(subst |,-D,$(DEFINES))
ASFLAGS += -t8
ASFLAGS += --cpu ARM7TDMI
ASFLAGS += --fpu None
ASFLAGS	+= $(subst |,-I,$(INCLUDE_DIRS))

CC	= $(TOOLCHAIN_BASE_DIR)/bin/iccarm
LD      = $(TOOLCHAIN_BASE_DIR)/bin/ilinkarm
#AR      = $(TOOLCHAIN_BASE_DIR)/bin/arm-none-eabi-ar
AS      = $(TOOLCHAIN_BASE_DIR)/bin/iasmarm
CP      = $(TOOLCHAIN_BASE_DIR)/bin/ielftool
#OD	= $(TOOLCHAIN_BASE_DIR)/bin/arm-none-eabi-objdump
#SZ	= $(TOOLCHAIN_BASE_DIR)/bin/arm-none-eabi-size
REMOVE 	= rm -f
AWK	= tools/awk
SCP 	= tools/scp.awk

all: o elf $(TARGET).bin

c: 	$(CSRC)
s: 	$(ASRC)
o: 	$(OBJ)

%.o: %.s
	$(AS) $< $(ASFLAGS) -L$(BUILD)/$(PLATFORM)/ -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -o $@
	
#$(TARGET).bin $(TARGET).hex $(TARGET).list : $(TARGET).elf
$(TARGET).bin $(TARGET).list : $(TARGET).elf
	$(CP) --bin $< $(TARGET).bin
#	$(CP) -Oihex $< $(TARGET).hex
#	$(OD) -D -S $< > $(TARGET).list

elf: o
	@if ! test -d $(BUILD); then mkdir $(BUILD); fi
	@if ! test -d $(BUILD)/$(PLATFORM); then mkdir $(BUILD)/$(PLATFORM); fi
	$(LD) $(OBJ) $(LFLAGS) -o $(TARGET).elf
#	$(SZ) $(TARGET).elf	

clean:
	$(REMOVE) $(OBJ)
	$(REMOVE) $(DEP)


include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# include py core make definitions
include ../py/py.mk


CROSS_COMPILE = arm-none-eabi-

KSDK_INC = \
	-Iplatform/osa/inc \
	-Iplatform/utilities/inc \
	-Iplatform/CMSIS/Include \
	-Iplatform/devices \
	-Iplatform/devices/MK64F12/include \
	-Iplatform/devices/MK64F12/startup \
	-Iplatform/hal/inc \
	-Iplatform/drivers/inc \
	-Iplatform/system/inc \
	-Ifrdmk64f

INC =  -I.
INC += -I..
INC += -I$(BUILD)

INC += $(KSDK_INC)

CPU_CFLAGS = -mcpu=cortex-m4  -mfloat-abi=hard  -mfpu=fpv4-sp-d16  -mthumb -fsingle-precision-constant -Wdouble-promotion -mapcs -fshort-wchar
CC_SYMBOLS = -DDEBUG -DCPU_MK64FN1M0VMD12 -DFRDM_K64F -DFREEDOM
CFLAGS = $(INC) \
	-Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  -std=gnu99\
       	$(CPU_CFLAGS) $(CC_SYMBOLS)

#Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LDSCRIPT = platform/devices/MK64F12/linker/gcc/MK64FN1M0xxx12_flash.ld
LDFLAGS = $(CPU_CFLAGS) -T$(LDSCRIPT) \
	  --specs=nano.specs  -lm  -Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  \
	  -static -Xlinker --gc-sections  -Xlinker -z  -Xlinker muldefs  -Xlinker --defsym=__stack_size__=0x2000 \
	  -Xlinker --defsym=__heap_size__=0x2000 -Wl,-Map,$(BUILD)/firmware.map

LIBS = -L. -lksdk_platform -lm -lc -lgcc -lnosys

SDKBOARD = frdmk64f
SRC_C = \
	platform/utilities/src/fsl_misc_utilities.c \
    	$(SDKBOARD)/gpio_pins.c \
    	$(SDKBOARD)/pin_mux.c \
    	$(SDKBOARD)/board.c \
	platform/utilities/src/fsl_debug_console.c \
	platform/utilities/src/print_scan.c \
	platform/devices/MK64F12/startup/system_MK64F12.c \
	platform/devices/startup.c \
	hardware_init.c \
	main.c \
	lib/mp-readline/readline.c \
	lib/utils/pyexec.c \
	lib/utils/stdout_helpers.c \
	lib/libc/string0.c \
	debug_uart.c
#	printf.c \
	string0.c \
	malloc0.c \
	gccollect.c \

SRC_CC = \

SRC_S = \
	platform/devices/MK64F12/startup/gcc/startup_MK64F12.S

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_CC:.cc=.o) $(SRC_S:.S=.o))

all: $(BUILD)/firmware.elf $(BUILD)/firmware.bin

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(OBJCOPY) -O binary $< $@
	cp $@ /home/paul/hs/kl26/micropython_K64F.bin

$(BUILD)/firmware.elf: $(OBJ) $(SYS_OBJECTS)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(OBJ) $(SYS_OBJECTS) $(LIBS)
	$(Q)$(SIZE) $@

include ../py/mkrules.mk

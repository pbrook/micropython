include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# include py core make definitions
include ../py/py.mk

TT = TARGET_K64F/TOOLCHAIN_GCC_ARM
MBED = mbed/build/mbed
MBED_LIBDIR = $(MBED)/$(TT)
MBED_USB = mbed/build/usb

SYS_OBJECTS = $(MBED_LIBDIR)/mbed_overrides.o $(MBED_LIBDIR)/system_MK64F12.o $(MBED_LIBDIR)/startup_MK64F12.o $(MBED_LIBDIR)/board.o $(MBED_LIBDIR)/cmsis_nvic.o $(MBED_LIBDIR)/retarget.o 

MBED_USB_INC = -I$(MBED_USB)/USBAudio -I$(MBED_USB)/USBDevice -I$(MBED_USB)/USBHID -I$(MBED_USB)/USBMIDI -I$(MBED_USB)/USBSerial -I$(MBED_USB)/USBMSD 
MBED_TARGET_DIR=$(MBED)/TARGET_K64F/TARGET_Freescale/TARGET_KPSDK_MCUS
MBED_INC = -I$(MBED) -I$(MBED)/TARGET_K64F \
	-I$(MBED)/TARGET_K64F/TARGET_Freescale \
	-I$(MBED_TARGET_DIR) \
	-I$(MBED_TARGET_DIR)/TARGET_MCU_K64F \
	-I$(MBED_TARGET_DIR)/TARGET_MCU_K64F/TARGET_FRDM \
	-I$(MBED_TARGET_DIR)/TARGET_MCU_K64F/device \
	-I$(MBED_TARGET_DIR)/TARGET_MCU_K64F/device/device \
	-I$(MBED_TARGET_DIR)/TARGET_MCU_K64F/device/MK64F12 \
	-I$(MBED_TARGET_DIR)/TARGET_KPSDK_CODE/hal/gpio

CROSS_COMPILE = arm-none-eabi-

INC =  -I.
INC += -I..
INC += -I$(BUILD)

INC += $(MBED_INC) $(USB_INC)

CFLAGS_CPU = -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fsingle-precision-constant -Wdouble-promotion
CFLAGS = $(INC) -Wall -Werror -ansi -std=gnu99 -nostdlib $(CFLAGS_CPU) $(COPT)

#Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LDFLAGS = $(CFLAGS_CPU) -T$(MBED_LIBDIR)/K64FN1M0xxx12.ld -Wl,-Map=$@.map,--cref
LIBS = -L$(MBED_LIBDIR) -L$(MBED_USB)/$(TT) -lmbed -lUSBDevice -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

SRC_C = \
	main.c \
#	printf.c \
	string0.c \
	malloc0.c \
	gccollect.c \

SRC_S = \
#	startup_stm32f40xx.s \
	gchelper.s \

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.s=.o))

all: $(BUILD)/firmware.elf

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)
	$(Q)$(SIZE) $@

include ../py/mkrules.mk

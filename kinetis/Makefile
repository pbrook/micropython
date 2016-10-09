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
	-Iplatform/drivers/src/flash/C90TFS/drvsrc/include \
	-Iplatform/system/inc \
	-Ifrdmk64f

USB_INC = \
	-Iusb/usb_core/include \
	-Iusb/adapter/sources \
	-Iusb/adapter/sources/sdk \
	-Iusb/usb_core/device/include/MK64F12 \
	-Iusb/usb_core/device/include \
	-Iusb/usb_core/device/sources/classes/cdc \
	-Iusb/usb_core/device/sources/classes/common \
	-Iusb/usb_core/device/sources/classes/composite \
	-Iusb/usb_core/device/sources/classes/include \
	-Iusb/usb_core/device/sources/classes/include/config \
	-Iusb/usb_core/device/sources/classes/msd \
	-Iusb/usb_core/device/sources/controller/khci \
	-Iusb/usb_core/device/sources/controller \
	-Iusb/usb_core/hal \

INC =  -I.
INC += -I..
INC += -I$(BUILD)
INC += -I../lib/mp-readline
INC += -I../lib/timeutils

INC += $(KSDK_INC) $(USB_INC)

CPU_CFLAGS = -mcpu=cortex-m4  -mfloat-abi=hard  -mfpu=fpv4-sp-d16  -mthumb -fsingle-precision-constant -Wdouble-promotion -mapcs -fshort-wchar
CC_SYMBOLS = -DDEBUG -DCPU_MK64FN1M0VMD12 -DFRDM_K64F -DFREEDOM
CFLAGS = $(INC) \
	-Wall  -fno-common  -ffunction-sections  -fdata-sections --std=gnu99\
       	$(CPU_CFLAGS) $(CC_SYMBOLS)

#Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LDSCRIPT = MK64F12.ld
LDFLAGS = $(CPU_CFLAGS) -T$(LDSCRIPT) \
	  --specs=nano.specs  -Wall  -fno-common  -ffunction-sections  -fdata-sections \
	  -static -Xlinker --gc-sections  -Xlinker -z  -Xlinker muldefs  -Xlinker --defsym=__stack_size__=0x2000 \
	  -Xlinker --defsym=__heap_size__=0x2000 -Wl,-Map,$(BUILD)/firmware.map

LIBS = -L. -lksdk_platform -lusbd_bm -lc -lgcc -lnosys

LIB_C = $(addprefix lib/, \
	libm/math.c \
	libm/asinfacosf.c \
	libm/atanf.c \
	libm/atan2f.c \
	libm/fmodf.c \
	libm/roundf.c \
	libm/log1pf.c \
	libm/acoshf.c \
	libm/asinhf.c \
	libm/atanhf.c \
	libm/kf_rem_pio2.c \
	libm/kf_sin.c \
	libm/kf_cos.c \
	libm/kf_tan.c \
	libm/ef_rem_pio2.c \
	libm/erf_lgamma.c \
	libm/sf_sin.c \
	libm/sf_cos.c \
	libm/sf_tan.c \
	libm/sf_frexp.c \
	libm/sf_modf.c \
	libm/sf_ldexp.c \
	libm/sf_erf.c \
	libm/wf_lgamma.c \
	libm/wf_tgamma.c \
	mp-readline/readline.c \
	utils/pyexec.c \
	utils/stdout_helpers.c \
	libc/string0.c \
	fatfs/ff.c \
	fatfs/option/ccsbcs.c \
	timeutils/timeutils.c \
	)

SDKBOARD = frdmk64f

STM_SRC_C = $(addprefix stmhal/, \
	gccollect.c \
	pybstdio.c \
	moduos.c \
	builtin_open.c \
	import.c \
	lexerfatfs.c \
	)

SRC_C = \
	$(LIB_C) \
	$(STM_SRC_C) \
	platform/utilities/src/fsl_misc_utilities.c \
    	$(SDKBOARD)/gpio_pins.c \
    	$(SDKBOARD)/pin_mux.c \
    	$(SDKBOARD)/board.c \
    	$(SDKBOARD)/flash.c \
	platform/utilities/src/fsl_debug_console.c \
	platform/utilities/src/print_scan.c \
	platform/devices/MK64F12/startup/system_MK64F12.c \
	platform/devices/startup.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/CopyToRam.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashCommandSequence.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashEraseSector.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashGetSecurityState.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashInit.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashProgram.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashProgramCheck.c \
    	platform/drivers/src/flash/C90TFS/drvsrc/source/FlashVerifySection.c \
	hardware_init.c \
	fatfs_port.c \
	main.c \
	storage.c \
	virtual_com.c \
	cube/cube_spi.c \
	cube/moducube.c \
	modutime.c \
	debug_uart.c
#	printf.c \
	string0.c \
	malloc0.c \
	gccollect.c \


SRC_QSTR += main.c $(STM_SRC_C) storage.c cube/moducube.c modutime.c

SRC_S = \
	platform/devices/MK64F12/startup/gcc/startup_MK64F12.S

STM_SRC_S = stmhal/gchelper.s

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_CC:.cc=.o) $(SRC_S:.S=.o) $(STM_SRC_S:.s=.o))

all: $(BUILD)/firmware.elf $(BUILD)/firmware.bin

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(OBJCOPY) -O binary $< $@
	cp $@ /home/paul/hs/kl26/micropython_K64F.bin

$(BUILD)/firmware.elf: $(OBJ) $(SYS_OBJECTS)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(OBJ) $(SYS_OBJECTS) $(LIBS)
	$(Q)$(SIZE) $@

include ../py/mkrules.mk

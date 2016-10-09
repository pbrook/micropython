#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "lib/utils/pyexec.h"
#include "lib/fatfs/ff.h"
#include "extmod/fsusermount.h"
#include "storage.h"
#include "board.h"
#include "readline.h"

fs_user_mount_t fs_user_mount_flash;

const mp_obj_type_t pyb_uart_type;

static const char fresh_boot_py[] =
"# boot.py -- run on boot-up\r\n"
"# can run arbitrary Python, but best to keep it minimal\r\n"
"\r\n"
"#import machine\r\n"
"#import pyb\r\n"
"#pyb.main('main.py') # main script to run after this one\r\n"
"#pyb.usb_mode('VCP+MSC') # act as a serial and a storage device\r\n"
"#pyb.usb_mode('VCP+HID') # act as a serial device and a mouse\r\n"
;

static const char fresh_main_py[] =
"# main.py -- put your code here!\r\n"
;

#if 0
static const char fresh_pybcdc_inf[] =
#include "genhdr/pybcdc_inf.h"
;
#endif

static const char fresh_readme_txt[] =
"This is a MicroPython board\r\n"
"\r\n"
"You can get started right away by writing your Python code in 'main.py'.\r\n"
"\r\n"
"For a serial prompt:\r\n"
" - Windows: you need to go to 'Device manager', right click on the unknown device,\r\n"
"   then update the driver software, using the 'pybcdc.inf' file found on this drive.\r\n"
"   Then use a terminal program like Hyperterminal or putty.\r\n"
" - Mac OS X: use the command: screen /dev/tty.usbmodem*\r\n"
" - Linux: use the command: screen /dev/ttyACM0\r\n"
"\r\n"
"Please visit http://micropython.org/help/ for further help.\r\n"
;

void _exit(int code)
{
    while (1);
}

// avoid inlining to avoid stack usage within main()
MP_NOINLINE STATIC void init_flash_fs() {
    // init the vfs object
    fs_user_mount_t *vfs = &fs_user_mount_flash;
    vfs->str = "/flash";
    vfs->len = 6;
    vfs->flags = 0;
    pyb_flash_init_vfs(vfs);

    // put the flash device in slot 0 (it will be unused at this point)
    MP_STATE_PORT(fs_user_mount)[0] = vfs;

    // try to mount the flash
    FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);

    if (res == FR_NO_FILESYSTEM) {
        // no filesystem so create a fresh one
        res = f_mkfs("/flash", 0, 0);
        if (res == FR_OK) {
            // success creating fresh LFS
        } else {
            printf("PYB: can't create flash filesystem\n");
            MP_STATE_PORT(fs_user_mount)[0] = NULL;
            return;
        }

        // set label
        f_setlabel("/flash/pybflash");

        // create empty main.py
        FIL fp;
        f_open(&fp, "/flash/main.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);

#if 0
        // create .inf driver file
        f_open(&fp, "/flash/pybcdc.inf", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_pybcdc_inf, sizeof(fresh_pybcdc_inf) - 1 /* don't count null terminator */, &n);
        f_close(&fp);
#endif

        // create readme file
        f_open(&fp, "/flash/README.txt", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
        f_close(&fp);

    } else if (res == FR_OK) {
        // mount sucessful
    } else {
        printf("PYB: can't mount flash\n");
        MP_STATE_PORT(fs_user_mount)[0] = NULL;
        return;
    }

    // The current directory is used as the boot up directory.
    // It is set to the internal flash filesystem by default.
    f_chdrive("/flash");

    // Make sure we have a /flash/boot.py.  Create it if needed.
    FILINFO fno;
#if _USE_LFN
    fno.lfname = NULL;
    fno.lfsize = 0;
#endif
    res = f_stat("/flash/boot.py", &fno);
    if (res == FR_OK) {
        if (fno.fattrib & AM_DIR) {
            // exists as a directory
            // TODO handle this case
            // see http://elm-chan.org/fsw/ff/img/app2.c for a "rm -rf" implementation
        } else {
            // exists as a file, good!
        }
    } else {
        // doesn't exist, create fresh file

        FIL fp;
        f_open(&fp, "/flash/boot.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_boot_py, sizeof(fresh_boot_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);
    }
}

void DD(void)
{
    LED1_ON;
}

static char heap[4096];

void init_ticks(void);

int main(int argc, char **argv) {
    hardware_init();
    init_ticks();
    DEBUG_printf("Hello World\n");
    LED1_EN;
    storage_init();
    gc_init(heap, heap + sizeof(heap));
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_)); // current dir (or base dir of the script)
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
    mp_obj_list_init(mp_sys_argv, 0);

    // zero out the pointers to the mounted devices
    memset(MP_STATE_PORT(fs_user_mount), 0, sizeof(MP_STATE_PORT(fs_user_mount)));

    readline_init0();
    init_flash_fs();
#if MICROPY_REPL_EVENT_DRIVEN
    pyexec_event_repl_init();
    for (;;) {
        int c = mp_hal_stdin_rx_chr();
        if (pyexec_event_repl_process_char(c)) {
            break;
        }
    }
#else
    pyexec_friendly_repl();
#endif
    mp_deinit();
    return 0;
}

void NORETURN __fatal_error(const char *msg) {
    DEBUG_printf("FATAL: %s", msg);
    while (1);
}

void nlr_jump_fail(void *val) {
    DEBUG_printf("FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, (mp_obj_t)val);
    __fatal_error("");
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif


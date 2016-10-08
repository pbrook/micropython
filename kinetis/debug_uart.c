#include <unistd.h>
#include "py/mpconfig.h"
#include "fsl_debug_console.h"

#include <stdio.h>
#include <stdarg.h>

int DEBUG_printf(const char *fmt, ...)
{
    va_list ap;
    static char err_buf[1024];
    const char *p;

    va_start(ap, fmt);
    vsprintf(err_buf, fmt, ap);
    for (p = err_buf; *p; p++) {
        PUTCHAR(*p);
    }
    return 0;
}

/*
 * Core UART functions to implement for a port
 */

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    return GETCHAR();
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    while (len) {
        PUTCHAR(*str);
        str++;
        len--;
    }
}

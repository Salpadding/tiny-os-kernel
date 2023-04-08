#include <stdarg.h>
#include <string.h>
#include <printk_helpers.h>
#include <serial.h>

int printk(const char* fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);
    i = _vsprintf(buf, fmt, args);
    va_end(args);
    serial_puts(buf);
    return i;
}

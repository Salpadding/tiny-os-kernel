#include <8086.h>

#include <printk_helpers.h>
#include <serial.h>


int printk_16(const char* fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);
    i = _vsprintf(buf, fmt, args);
    va_end(args);
    puts(buf);
    return i;
}

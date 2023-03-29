#ifndef __SERIAL_H
#define __SERIAL_H

#include <io.h>

#ifndef SERIAL_PORT
#define SERIAL_PORT 0x3f8
#endif

#define INIT_SERIAL \
	outb_p(0x80,SERIAL_PORT+3); \
	outb_p(0x30,SERIAL_PORT); \
	outb_p(0x00,SERIAL_PORT+1); \
	outb_p(0x03,SERIAL_PORT+3); \
	outb_p(0x0b,SERIAL_PORT+4); \
	outb_p(0x0d,SERIAL_PORT+1); 

#ifndef __ASM__

static void serial_putc(char c) {
    unsigned int ax = 0 ; 

    while(!(ax & 0xff)) {
        asm volatile ("inb %%dx,%%al" :"=a"(ax):"d"(SERIAL_PORT+5):);
    }
    outb_p(c, SERIAL_PORT);
}

static void serial_puts(const char* s) {
    while(*s) {
        serial_putc(*(s++));
    }
}
#endif

#endif

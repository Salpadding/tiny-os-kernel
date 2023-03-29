#ifndef __IO_H
#define __IO_H

static void outb_p(unsigned char value, unsigned short port) {
    asm volatile("outb %%al, %%dx" ::"a"(value), "d"(port):);
}

static unsigned char inb(unsigned short port) {
    unsigned long eax;
    asm volatile("inb %%dx, %%al" :"=a"(eax):"d"(port):);
    return (unsigned char)(eax &0xff);
}

#endif

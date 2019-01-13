#include "system.h"

char meow_getchar(void) {
    com32sys_t regs;

    regs.eax.b[1] = 0x10; // or eax.w[0] = 0x10 << 8
    __intcall(0x16, &regs, &regs);

    return (char)(regs.eax.w[0] & 0xFF);
}

int meow_getkeycode(void) {
    com32sys_t regs;
 
    regs.eax.b[1] = 0x10;
    __intcall(0x16, &regs, &regs);

    return regs.eax.l;
}

void meow_system_reboot(void) {
    uint64_t x = 0;
    uint64_t *xp = &x;
    asm volatile("lidt (%%eax)" : : "a" (xp));
    volatile int b = 0;
    volatile int a = 1 / b;
    a;
}

void meow_system_shutdown(void) {
    asm volatile ("outw %1, %0" : : "dN" ((uint16_t)0xB004), "a" ((uint16_t)0x2000));
    // TODO: Make ACPI or APM support and true shutdown
}

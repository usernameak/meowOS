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

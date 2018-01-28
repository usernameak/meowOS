#include "system.h"

char meow_getchar(void) {
    com32sys_t regs;
    regs.eax.w[0] = 0x1000;
    __intcall(0x16, &regs, &regs);
    return (char)(regs.eax.w[0] & 0xFF);
}
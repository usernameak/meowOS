#pragma once

#include "vesa.h"

enum eMEOW_KEYCODES {

    KEYCODE_NULL        = 0x0,

    KEYCODE_UPARROW     = 0X348E0,
    KEYCODE_DOWNARROW   = 0X350E0,
    KEYCODE_LEFTARROW   = 0X34BE0,
    KEYCODE_RIGHTARROR  = 0X34DE0,

    KEYCODE_HOME        = 0x347e0,
    KEYCODE_END         = 0x34fe0,
    KEYCODE_PGUP        = 0x349e0,
    KEYCODE_PGDOWN      = 0x351e0,

    KEYCODE_ENTER       = 0X31C0D,
    KEYCODE_BACKSPACE   = 0X30E08,
    KEYCODE_ESC         = 0X3011B,
    KEYCODE_DELETE      = 0X353E0,

    KEYCODE_F1          = 0X33B00,
    KEYCODE_F2          = 0X33C00,
    KEYCODE_F3          = 0X33D00,
    KEYCODE_F4          = 0X33E00,
    KEYCODE_F5          = 0X33F00,
    KEYCODE_F6          = 0X34000,
    KEYCODE_F7          = 0X34100,
    KEYCODE_F8          = 0X34200,
    KEYCODE_F9          = 0X34300,
    KEYCODE_F10         = 0X34400,
    KEYCODE_F11         = 0X34500,
    KEYCODE_F12         = 0X34600,
};

extern struct vbe_mode_info_structure *meow_mi;
char meow_getchar(void);
int  meow_getkeycode(void);

void meow_system_reboot(void);
void meow_system_shutdown(void);

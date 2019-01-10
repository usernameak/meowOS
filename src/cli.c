#include "cli.h"

#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/io.h>
#include <sys/cpu.h>
#include <sys/pci.h>
#include <string.h>

#define MAX_LINES     48 // Maximal quantity of lines in one screen before autoclean
#define MAX_HISTORY   20 // Maximal history size
#define MAX_CMDLENGTH 80 // Maximal one command length

static int meow_cli_line;
static int meow_cli_col;

static char meow_cli_history_cmds[20][80] = {{0}}; // MAX_HISTORY commands
static uint8_t meow_cli_history_windex = 0; // History write index
static uint8_t meow_cli_history_rindex = 0; // History read index

void meow_cli_next_line(void) {
    meow_cli_line = (meow_cli_line + 1) % 50;
    meow_draw_str(meow_mi, 0, meow_cli_line, "                                                                                ", 0x00FFFFFF, 0);
}

void meow_cli_exec_command(char *command) {
    // Add command to the history
    // > if the command exists and or it is the first command or it isn't repeation
    if(strlen(command) && (!meow_cli_history_windex || strcmp(command, meow_cli_history_cmds[meow_cli_history_windex-1]))) {
        // Check for overflow
        if(meow_cli_history_windex >= MAX_HISTORY) meow_cli_history_windex = 0;
        // Reset the read index
        meow_cli_history_rindex = meow_cli_history_windex;
        // Write the command to the command history
        strlcpy(meow_cli_history_cmds[meow_cli_history_windex], command, MAX_CMDLENGTH);
        meow_cli_history_windex++;
        meow_cli_history_rindex = meow_cli_history_windex;
    }

    if(strncmp(command, "ls", 2) == 0) {
        char *dir = "";
        if(command[2] == '\0') {
            dir = ".";
        } else if(command[2] == ' ') {
            dir = &command[3];
        }
        // meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "LS // TODO", 0x00FFFFFF, 0);
        DIR *d;
        struct dirent *dp;
        if((d = opendir(dir)) == NULL) {
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "ERROR", 0x00FFFFFF, 0);
            return;
        }
        while((dp = readdir(d)) != NULL) {
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, dp->d_name, 0x00FFFFFF, 0);
            meow_cli_next_line();
        }
        closedir(d);
    } else if(strncmp(command, "echo", 4) == 0) {
        char *message = "";

        if(command[4] == ' ' && strlen(command) > 5) {
            message = &command[5];
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, message, 0x00FFFFFF, 0);
        }

        return;
    } else if(strncmp(command, "cat", 3) == 0) {
        char *file = "";
        if(command[3] == '\0') {
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "Usage: cat <file>", 0x00FFFFFF, 0);
            return;
        } else if(command[3] == ' ') {
            file = &command[4];
        }
        // meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "LS // TODO", 0x00FFFFFF, 0);
        FILE *f;
        char s[80];
        if((f = fopen(file, "r")) == NULL) {
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "ERROR", 0x00FFFFFF, 0);
            return;
        }
        while((fgets(s, 80, f)) != NULL) {
            s[strcspn(s, "\n")] = 0;
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, s, 0x00FFFFFF, 0);
            meow_cli_next_line();
        }
        fclose(f);
    } else if(strncmp(command, "view", 4) == 0) {
            char *file = "";
            if(command[4] == '\0') {
                meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "Usage: view <file>", 0x00FFFFFF, 0);
                return;
            } else if(command[4] == ' ') {
                file = &command[5];
            }
            FILE *f;
            if((f = fopen(file, "rb")) == NULL) {
                meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "ERROR", 0x00FFFFFF, 0);
                return;
            }
            fread((void*) meow_mi->framebuffer, 640 * 400, 4, f);
            char *fb = (char *) meow_mi->framebuffer;
            for(int i = 0; i < 640 * 400 * 4; i += 4) {
                int r = fb[i];
                int g = fb[i + 1];
                int b = fb[i + 2];
                int a = fb[i + 3];
                fb[i] = b;
                fb[i + 1] = g;
                fb[i + 2] = r;
                fb[i + 3] = a;
            }
            fclose(f);
            meow_draw_str(meow_mi, 0, 0, "Press any key", 0x00FFFFFF, 0);
            meow_getchar();
    } else if(strcmp(command, "clear") == 0) {
        char *fb = (char *) meow_mi->framebuffer;

        for(int i = 640 * 4 * 8; i < 640 * 480 * 4; i++) { // Ignore first (copyright) line
            fb[i] = 0x0;
        }
        meow_cli_col  = 0;
        meow_cli_line = 0;
    } else if(strcmp(command, "reboot") == 0) {
            uint64_t x = 0;
            uint64_t *xp = &x;
            asm volatile("lidt (%%eax)" : : "a" (xp));
            volatile int b = 0;
            volatile int a = 1 / b;
            a;
    } else if(strcmp(command, "info") == 0) {
        unsigned char idt_out[6];
        __asm__ volatile ("sidt %0": "=m" (idt_out));
        uint32_t idt_address = *((uint32_t *)(idt_out+2));
        uint16_t idt_size = *((uint16_t *)(idt_out));
        char s[80];
        sprintf(s, "IDTR: addr 0x%X size %d bytes", idt_address, idt_size + 1);
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, s, 0x00FFFFFF, 0);
        struct cpuid_t {
            char a[4];
            char b[4];
            char c[4];
            char d[4];
            char nul;
        } id;
        cpuid(0, (uint32_t*)&id.a, (uint32_t*)&id.b, (uint32_t*)&id.c, (uint32_t*)&id.d);
        id.nul = 0;
        meow_cli_next_line();
        sprintf(s, "VENDOR: %.4s%.4s%.4s", id.b, id.d, id.c);
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, s, 0x00FFFFFF, 0);
    } else if(strcmp(command, "help") == 0) {
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "Help:", 0x0000FF00, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " info: shows system info", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " help: shows this", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " clear: clear the screen", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " echo <text>: prints the text", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " ls [dir]: shows directory contents", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " view <image.raw>: shows raw image", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " cat <file>: prints file contents", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " reboot: reboots", 0x00FFFFFF, 0);
        meow_cli_next_line();
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " pci: lists PCI devs", 0x00FFFFFF, 0);
    } else if(strcmp(command, "pci") == 0) {
        struct pci_domain *dom = pci_scan();
        struct pci_device *func;
        pciaddr_t addr;
        int i = 0;
        for_each_pci_func3(func, dom, addr) {
            char s[80];
            sprintf(s, "Device %d.%d.%d: %04X:%04X/SUB:%04X:%04X/REV%04X", pci_bus(addr), pci_dev(addr), pci_func(addr), 
                func->vendor, func->product,
                func->sub_vendor, func->sub_product, func->revision);
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, s, 0x00FFFFFF, 0);
            meow_cli_next_line();
            i++;
        }
    } else {
        if(strlen(command)) meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "Unknown command, use help command for help.", 0x00FF0000, 0);
    }

    if(meow_cli_line > MAX_LINES) meow_cli_exec_command("clear");
}

void meow_cli_process_keyboard(char *command, uint8_t cmdpos) {
        while(1) {
            meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '_', 0x00FFFFFF, 0);
            int key = meow_getkeycode();
            char c = key & 0xFF;

            switch(key) {
                case KEYCODE_ENTER:
                    command[cmdpos] = '\0';
                    return;
                case KEYCODE_BACKSPACE:
                    if(cmdpos > 0) {
                        command[cmdpos--] = '\0';
                        meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '\0', 0x00FFFFFF, 0);
                        meow_cli_col--;
                    }
                    continue;
                case KEYCODE_UPARROW:
                case KEYCODE_DOWNARROW:
                    // Clear current command
                    for(short i = meow_cli_col; i > 1; i--, meow_cli_col--) {
                        meow_draw_char(meow_mi, i, meow_cli_line, '\0', 0x00FFFFFF, 0);
                    }

                    // Check for overflow
                    if(key == KEYCODE_UPARROW) {
                        if(meow_cli_history_rindex == 0) 
                            meow_cli_history_rindex = meow_cli_history_windex;
                        meow_cli_history_rindex--;
                    } else {
                        if(meow_cli_history_rindex == meow_cli_history_windex) 
                            meow_cli_history_rindex = 0;
                        meow_cli_history_rindex++;
                    }

                    // Draw command from the history using its read index
                    meow_draw_str(
                            meow_mi, meow_cli_col, meow_cli_line, 
                            meow_cli_history_cmds[meow_cli_history_rindex],
                            0x00FFFFFF, 0x0
                    );

                    // Copy command from the history to the current input
                    strlcpy(command, meow_cli_history_cmds[meow_cli_history_rindex], MAX_CMDLENGTH);

                    // Move cursor to the end of command
                    meow_cli_col = (strlen(meow_cli_history_cmds[meow_cli_history_rindex]) + 1) % MAX_CMDLENGTH;
                    cmdpos = (strlen(meow_cli_history_cmds[meow_cli_history_rindex])) % MAX_CMDLENGTH;
                    command[cmdpos] = '\0';

                    // Don't draw or execute anything
                    continue;
            }

            if(c < 0x20 || c > 0x7F) continue; // No trash in the console =D

            meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, c, 0x00FFFFFF, 0);
            command[cmdpos++] = c;
            meow_cli_col++;
        }
}

void meow_cli_start_interpreter(void) {
    meow_cli_line = 0;
    meow_cli_col = 0;

    meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "meowOS (c) UsernameAK", 0x00FFFF00, 0);
    meow_cli_next_line();
    
    char command[MAX_CMDLENGTH];
    uint8_t cmdpos = 0;

    while(1) {
        command[0] = '\0';
        cmdpos = 0;
        meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '>', 0x00FFFFFF, 0);
        meow_cli_col++;

        meow_cli_process_keyboard(command, cmdpos);

        meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '\0', 0x00FFFFFF, 0);
        meow_cli_col = 0; // \r
        meow_cli_next_line();
        meow_cli_exec_command(command);
        meow_cli_line++;  // \n
    }
}

#include "cli.h"

#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/io.h>
#include <sys/cpu.h>
#include <sys/pci.h>
#include <string.h>

static int meow_cli_line;
static int meow_cli_col;

void meow_cli_exec_command(char *command) {
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
            meow_cli_line++;
        }
        closedir(d);
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
        char *s = malloc(80);
        if((f = fopen(file, "r")) == NULL) {
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "ERROR", 0x00FFFFFF, 0);
            return;
        }
        while((fgets(s, 80, f)) != NULL) {
            s[strcspn(s, "\n")] = 0;
            meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, s, 0x00FFFFFF, 0);
            meow_cli_line++;
        }
        free(s);
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
        meow_cli_line++;
        sprintf(s, "VENDOR: %.4s%.4s%.4s", id.b, id.d, id.c);
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, s, 0x00FFFFFF, 0);
    } else if(strcmp(command, "help") == 0) {
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "Help:", 0x0000FF00, 0);
        meow_cli_line++;
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " info: shows system info", 0x00FFFFFF, 0);
        meow_cli_line++;
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " help: shows this", 0x00FFFFFF, 0);
        meow_cli_line++;
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " ls [dir]: shows directory contents", 0x00FFFFFF, 0);
        meow_cli_line++;
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " view <image.raw>: shows raw image", 0x00FFFFFF, 0);
        meow_cli_line++;
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " cat <file>: prints file contents", 0x00FFFFFF, 0);
        meow_cli_line++;
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, " reboot: reboots", 0x00FFFFFF, 0);
        meow_cli_line++;
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
            meow_cli_line++;
            i++;
        }
    } else {
        meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "Unknown command, use help command for help.", 0x00FF0000, 0);
    }
}

void meow_cli_start_interpreter(void) {

    meow_cli_line = 0;
    meow_cli_col = 0;
    meow_draw_str(meow_mi, meow_cli_col, meow_cli_line, "meowOS (c) UsernameAK", 0x00FFFF00, 0);
    meow_cli_line++;
    char *s = malloc(80);
    char *ss = s;
    while(1) {
        s = ss;
        meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '>', 0x00FFFFFF, 0);
        meow_cli_col++;

        while(1) {
            meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '_', 0x00FFFFFF, 0);
            char c = meow_getchar();
            if(c == 0xD) {
                *s++ = 0;
                break;
            } else if (c == 0x8) {
                if(s > ss) {
                    *s-- = 0;
                    meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '\0', 0x00FFFFFF, 0);
                    meow_cli_col--;
                }
                continue;
            }
            meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, c, 0x00FFFFFF, 0);
            *s++ = c;
            meow_cli_col++;
        }
        meow_draw_char(meow_mi, meow_cli_col, meow_cli_line, '\0', 0x00FFFFFF, 0);
        meow_cli_col = 0;
        meow_cli_line++;
        meow_cli_exec_command(ss);
        meow_cli_line++;
    }
}
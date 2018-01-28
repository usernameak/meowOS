#include "vesa.h"
#include "util.h"
#include "cli.h"

struct vbe_mode_info_structure *meow_mi;


int main(void) {
    uint16_t width = 640;
    uint16_t height = 400;
    uint16_t bpp = 32;

    printf("Init...\n");

    meow_mi = meow_init_vesa(width, height, bpp);

    meow_cli_start_interpreter();

    meow_stop_system();
    
    return 0;
}
#pragma once

#include <stdint.h>

void meow_stop_system(void);
void meow_memset32( void * dest, uint32_t value, uintptr_t size );
#include "util.h"
#include <sys/cpu.h>
#include <com32.h>
#include <math.h>
#include <limits.h>
#include <float.h>

void meow_stop_system(void) {
    while(1) {
        hlt();
    }
}

void meow_memset32( void * dest, uint32_t value, uint32_t size )
{
  uintptr_t i;
  for( i = 0; i < (size & (~3)); i+=4 )
  {
    memcpy( ((char*)dest) + i, &value, 4 );
}  
for( ; i < size; i++ )
  {
    ((char*)dest)[i] = ((char*)&value)[i&3];
  }
  // asm volatile("")
}

#define pow2(x) pow(2, x)

double
ldexp(double f, register int x)
{
	if (x < 0)
		f /= pow2(-x);
	else if (x < DBL_MAX_EXP)
		f *= pow2(x);
	else
		f = (f * pow2(DBL_MAX_EXP - 1)) * pow2(x - (DBL_MAX_EXP - 1));
	return f;
}
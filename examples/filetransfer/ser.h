#include <stdlib.h>
#include <stdint.h>
#include <string.h>


void ser_writedata64(uint64_t obj, unsigned char* buf);

uint64_t ser_readdata64(unsigned char* buf);


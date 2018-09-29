
#include "ser.h"



void ser_writedata64(uint64_t obj, unsigned char* buf)
{
    obj = htole64(obj);
    memcpy(buf, (char*)&obj, 8);
}

uint64_t ser_readdata64(unsigned char* buf)
{
    uint64_t obj;
    memcpy((char*)&obj, buf, 8);
    return le64toh(obj);
}


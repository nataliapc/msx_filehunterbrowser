#include <stdint.h>
#include <string.h>
#include "conio.h"
#include "utils.h"


void putstrxy(uint8_t x, uint8_t y, char *str)
{
	putlinexy(x, y, strlen(str), str);
}

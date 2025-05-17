#include <stdint.h>
#include "utils.h"


void memncpy(char *dst, char *src, char c, uint16_t size)
{
	while (size && *src != c) {
		*dst++ = *src++;
		size--;
	}
}

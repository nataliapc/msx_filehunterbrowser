#include <stdint.h>
#include "utils.h"
#include "conio.h"


char* formatSize(char *dst, uint16_t size)
{
	char *format;
	uint16_t finalSize;

	if (size < 1024L) {
		format = "%uKB";
		finalSize = size;
	} else {
		format = "%uMB";
		finalSize = size / 1024;
	}
	csprintf(dst, format, finalSize);
	return dst;
}

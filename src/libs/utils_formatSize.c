#include <stdint.h>
#include "utils.h"
#include "conio.h"


char* formatSize(char *dst, uint32_t size)
{
	char *format;
	uint16_t finalSize;

	if (size < 1024L) {
		format = "%uB ";
		finalSize = (uint16_t)size;
	} else if (size < 1024L*1024L) {
		format = "%uKB";
		finalSize = (uint16_t)(size / 1024L);
	} else {
		format = "%uMB";
		finalSize = (uint16_t)(size / (1024L*1024L));
	}
	csprintf(dst, format, finalSize);
	return dst;
}

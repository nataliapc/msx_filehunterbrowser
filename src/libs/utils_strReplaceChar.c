#include "utils.h"


char* strReplaceChar(char *str, char find, char replace)
{
	if (!str) return str;

	char *ptr = str;
	while (*ptr) {
		if (*ptr == find) {
			*ptr = replace;
		}
		ptr++;
	}
	return str;
}
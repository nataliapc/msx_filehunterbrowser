#include <stdint.h>
#include <ctype.h>
#include "utils.h"
#include "conio.h"


// obtain chars from getch() and put into str until
// a carriage return is found or length is reached
uint8_t scanf(char *str, uint16_t maxLen)
{
	char *ptr = str;

	while (true) {
		*ptr = getch();
		if (*ptr == KEY_ESC) {
			ptr = str;
			break;
		}
		if (*ptr == '\r') {
			break;
		}
		if (*ptr == '\b') {
			if (ptr > str) {
				ptr--;
				maxLen++;
				cputs("\b \b");
			}
		} else if (maxLen) {
			if (isalnum(*ptr) || *ptr == ' ') {
				putchar(*ptr);
				maxLen--;
				ptr++;
			} else {
				putchar('\x07');
			}
		}
	}
	*ptr = 0;
	return ptr - str;
}
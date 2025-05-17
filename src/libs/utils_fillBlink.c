#include "conio.h"


void fillBlink(uint8_t x, uint8_t y, uint8_t lines, uint8_t len, bool enabled)
{
	while (lines--) {
		textblink(x, y++, len, enabled);
	}
}
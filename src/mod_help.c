/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdlib.h>
#include "msx_const.h"
#include "conio.h"
#include "heap.h"
#include "utils.h"
#include "fh.h"
#include "mod_help.h"


// ========================================================
extern const unsigned char out_help_bin_zx0[];


// ========================================================
#define HELPWIN_POSY	6
#define HELPWIN_SIZE	(*((uint16_t*)out_help_bin_zx0))
#define HELPWIN_HEIGHT	(HELPWIN_SIZE/80)



// ========================================================
void showHelpWindow()
{
	setSelectedLine(false);
	_fillVRAM(0+(HELPWIN_POSY-1)*80, HELPWIN_SIZE, ' ');
	fillBlink(1,HELPWIN_POSY, HELPWIN_HEIGHT,80, true);

	dzx0_standard(out_help_bin_zx0 + 2, heap_top);
	msx2_copyToVRAM((uint16_t)heap_top, 0+(HELPWIN_POSY-1)*80, HELPWIN_SIZE);

	// Wait for a pressed key
	waitKey();

	_fillVRAM(0+(HELPWIN_POSY-1)*80, HELPWIN_SIZE, ' ');
	fillBlink(1,HELPWIN_POSY, HELPWIN_HEIGHT,80, false);
	printList();
}

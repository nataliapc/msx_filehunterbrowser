/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdlib.h>
#include "msx_const.h"
#include "conio.h"
#include "utils.h"
#include "fh.h"
#include "mod_help.h"


// ========================================================
static const HelpWin_t helpWin[] = {
	{ HELPWIN_POSX, HELPWIN_POSY+1, "Keyboard shortcuts:" },
	{ HELPWIN_POSX, HELPWIN_POSY+3, "M ........... Change MSX target" },
	{ HELPWIN_POSX, HELPWIN_POSY+4, "R/D/C/V ..... Select a panel" },
	{ HELPWIN_POSX, HELPWIN_POSY+5, "TAB ......... Next panel" },
	{ HELPWIN_POSX, HELPWIN_POSY+6, "UP/DOWN ..... Select item" },
	{ HELPWIN_POSX, HELPWIN_POSY+7, "RIGHT/LEFT .. Next/Prev page" },
	{ HELPWIN_POSX, HELPWIN_POSY+8, "ENTER ....... Search by text" },
	{ HELPWIN_POSX, HELPWIN_POSY+9, "F1 .......... Help" },
	{ HELPWIN_POSX, HELPWIN_POSY+10,"F5 .......... Download selected file" },
	{ HELPWIN_POSX, HELPWIN_POSY+11,"ESC ......... Exit" },
	{ 0,0, NULL }
};

// ========================================================
void showHelpWindow()
{
	setSelectedLine(false);
	_fillVRAM(0+(HELPWIN_POSY-1)*80, HELPWIN_HEIGHT*80, ' ');
	fillBlink(1,HELPWIN_POSY, HELPWIN_HEIGHT,80, true);

	HelpWin_t *help = helpWin;
	while (help->str) {
		putstrxy(help->x, help->y, help->str);
		help++;
	}

	// Wait for a pressed key
	waitKey();

	_fillVRAM(0+(HELPWIN_POSY-1)*80, HELPWIN_HEIGHT*80, ' ');
	fillBlink(1,HELPWIN_POSY, HELPWIN_HEIGHT,80, false);
	printList();
}

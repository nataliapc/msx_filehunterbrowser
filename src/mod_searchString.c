/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <string.h>
#include <stdbool.h>
#include "msx_const.h"
#include "conio.h"
#include "fh.h"
#include "utils.h"
#include "mod_searchString.h"


// ========================================================
inline void printSearchString()
{
	_fillVRAM(0+(SEARCH_POSY-1)*80, SEARCH_HEIGHT*80, ' ');
	fillBlink(1,SEARCH_POSY, SEARCH_HEIGHT,80, true);

	// Print search message
	putstrxy(4, SEARCH_POSY+1, "Search:");
}

inline void clearSearchString()
{
	fillBlink(1,SEARCH_POSY, SEARCH_HEIGHT,80, false);
}

void changeSearchString()
{
	ASM_EI; ASM_HALT;
	setSelectedLine(false);
	printSearchString();

	*buff = '\0';
	gotoxy(12, SEARCH_POSY+1);
	scanf(buff, SEARCH_MAX_SIZE);
	bool needUpdate = *buff;

	ASM_EI; ASM_HALT;
	clearSearchString();

	if (needUpdate) {
		strcpy(request.search.value, buff);
		resetSelectedLine();
	}
	printRequestData();

	if (needUpdate) {
		updateList();
	} else {
		printList();
		setSelectedLine(true);
	}
}

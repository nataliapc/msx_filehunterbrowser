/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <string.h>
#include "msx_const.h"
#include "conio.h"
#include "heap.h"
#include "utils.h"
#include "fh.h"
#include "structs.h"
#include "mod_downloadFiles.h"


// ========================================================
inline void printDownloadMessage(ListItem_t *item)
{
	_fillVRAM(0+(DOWNLOAD_POSY-1)*80, DOWNLOAD_HEIGHT*80, ' ');
	fillBlink(1,DOWNLOAD_POSY, DOWNLOAD_HEIGHT,80, true);

	// Prepare filename with elipsis if too long
	strcpy(heap_top, item->name);
	if (strlen(item->name) >= 63) {
		strcpy(heap_top+63, "...");
	}

	// Print download message
	csprintf(buff, "File: \"%s\"", heap_top);
	putstrxy(4, DOWNLOAD_POSY+1, buff);
	csprintf(buff, "Size: %lu bytes", item->size);
	putstrxy(4, DOWNLOAD_POSY+2, buff);
	putstrxy(4, DOWNLOAD_POSY+4, "Filename to save: ________.___");
	gotoxy(22, DOWNLOAD_POSY+4);
}

inline void clearDownloadMessage()
{
	fillBlink(1,DOWNLOAD_POSY, DOWNLOAD_HEIGHT,80, false);
}

// ========================================================
void downloadFile()
{
	ListItem_t *item = getCurrentItem();

	ASM_EI; ASM_HALT;
	setSelectedLine(false);
	printDownloadMessage(item);

	getch();	// Wait for a key press

	ASM_EI; ASM_HALT;
	clearDownloadMessage();
	printList();
	setSelectedLine(true);
}

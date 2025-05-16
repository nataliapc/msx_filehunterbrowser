/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma opt_code_size
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "msx_const.h"
#include "heap.h"
#include "utils.h"
#include "dos.h"
#include "conio.h"
#include "asm.h"
#include "hgetlib.h"
#include "fh.h"
#include "patterns.h"
#ifdef _DEBUG_
	#include "test.h"
#endif

#define PANEL_FIRSTY	5
#define PANEL_LASTY		22
#define PANEL_HEIGHT	(PANEL_LASTY-PANEL_FIRSTY)

// ========================================================
static uint8_t msxVersionROM;
static uint8_t kanjiMode;
static uint8_t originalLINL40;
static uint8_t originalSCRMOD;
static uint8_t originalFORCLR;
static uint8_t originalBAKCLR;
static uint8_t originalBDRCLR;

char *emptyArea;
char *buff;
ListItem_t *list_start;
Panel_t *currentPanel;
uint8_t itemsCount;
uint8_t topLine = 0, currentLine = 0;
uint8_t newTopLine = 0, newCurrentLine = 0;

#define UNAPI_BUFFER_SIZE	1600
extern char unapiBuffer[UNAPI_BUFFER_SIZE];
char *list_raw = NULL;
uint32_t downloadSize;


// ========================================================
// Function declarations

void restoreScreen();
void setByteVRAM(uint16_t vram, uint8_t value) __sdcccall(0);
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);
void _copyRAMtoVRAM(uint16_t memory, uint16_t vram, uint16_t size) __sdcccall(0);


// ========================================================
void abortRoutine()
{
	restoreScreen();
	dos2_exit(1);
}

void initializeBuffers()
{
	// A way to avoid using low memory when using BIOS calls from DOS
	if (heap_top < (void*)0x8000)
		heap_top = (void*)0x8000;

	// Initialize buffers
	list_start = (ListItem_t*)malloc(sizeof(ListItem_t) * 255);
	buff = malloc(80*20);
}

void checkPlatformSystem()
{
	// Check TCP/IP UNAPI
	char ret = hgetinit((uint16_t)unapiBuffer);
	if (ret != ERR_TCPIPUNAPI_OK) {
#ifndef _DEBUG_
		if (ret == ERR_TCPIPUNAPI_NOT_TCPIP_CAPABLE) {
			die("TCP/IP UNAPI not fully capable!\x07\r\n");
		} else if (ret == ERR_HGET_INVALID_BUFFER) {
			die("Invalid buffer for TCP/IP UNAPI!\x07\r\n");
		}
		die("TCP/IP UNAPI not found!\x07\r\n");
#endif
	}

	// Check MSX2 ROM or higher
	msxVersionROM = getRomByte(MSXVER);
	if (!msxVersionROM) {
		die("MSX1 not supported!");
	}

	// Check MSX-DOS 2 or higher
	if (dosVersion() < VER_MSXDOS2x) {
		die("MSX-DOS 2.x or higher required!");
	}

	// Set abort exit routine
	dos2_setAbortRoutine((void*)abortRoutine);

	// Backup original values
	originalLINL40 = varLINL40;
	originalSCRMOD = varSCRMOD;
	originalFORCLR = varFORCLR;
	originalBAKCLR = varBAKCLR;
	originalBDRCLR = varBDRCLR;
	kanjiMode = (detectKanjiDriver() ? getKanjiMode() : 0);
}

inline void redefineFunctionKeys()
{
	char *fk = (char*)FNKSTR;
	memset(fk, 0, 160);
	for (uint8_t i='1'; i<='5'; i++,fk+=16) {
		*fk = i;
	}
}

inline void redefineCharPatterns()
{
	_copyRAMtoVRAM((uint16_t)charPatters, 0x1000+24*8, 2*8);
}

inline bool isShiftKeyPressed()
{
	return varNEWKEY_row6.shift == 0;
}

void putstrxy(uint8_t x, uint8_t y, char *str)
{
	putlinexy(x, y, strlen(str), str);
}


// ========================================================
uint32_t fileSize = 0;
char progressChar[] = {'\x84', '\x85'};
uint8_t progress = 0;
void HTTPStatusUpdate (bool isChunked)
{
	gotoxy(79,1);
	putch(progressChar[progress]);
	progress = (progress + 1) % sizeof(progressChar);
}

void DataWriteCallback(char *rcv_buffer, int bytes_read)
{
	if (bytes_read) {
		memcpy(heap_top, rcv_buffer, bytes_read);
		heap_top += bytes_read;
	}
}

void HGETFileSizeUpdate (long contentSize)
{
	downloadSize = contentSize;
}

void getRemoteList()
{
	heapPop();
	heapPush();
	list_raw = heap_top;

#ifdef _DEBUG_
	uint16_t i = TEST_SIZE, size = 1024, pos = 0, cnt;
	HGETFileSizeUpdate(TEST_SIZE);
	while (i) {
		if (i < size) {
			size = i;
		}
		memcpy(unapiBuffer, &test_txt[pos], size);
		DataWriteCallback(unapiBuffer, size);
		HTTPStatusUpdate(true);
		pos += size;
		i -= size;

		cnt = varJIFFY;
		while (varJIFFY-cnt < 25) {
			ASM_EI; ASM_HALT;
		}
	}
#else
	csprintf(buff, BASEURL, request.type->value, request.msx->value, request.search.value, "");
	if (hget(
		buff,							// URL
		NULL,							// filename
		NULL,							// credent
		(int)HTTPStatusUpdate,			// progress_callback
		0,								// rcvbuffer
		NULL,//(char*)(varTPALIMIT-4096),		// rcvbuffersize
		(int)DataWriteCallback,			// data_write_callback
		(int)HGETFileSizeUpdate,		// content_size_callback
		false							// enableKeepAlive
	) != ERR_TCPIPUNAPI_OK)
	{
		die("Failure!!!");
	}
#endif
//cprintf("Download complete! %lu    \n", downloadSize);
//getchar();
	*((char*)heap_top) = '\0';
	heap_top++;
	initializeBuffers();

	gotoxy(79,1);
	putch(progressChar[sizeof(progressChar)-1]);
	progress = 0;
}

char *findStringEnd(char *str)
{
	char *end = str;
	while (*end && *end != '\t' && *end != '\n') {
		end++;
	}
	return end;
}

void processList()
{
	char *data = list_raw;
	char *end;
	bool isCas = false;
	ListItem_t *item = list_start;

	while (*data) {
		// Name
		end = findStringEnd(data);
		item->name = data;
		*end++ = 0;
		data = end;

		// Size
		end = findStringEnd(data);
		item->size = strtol(data, NULL, 10);
		isCas = (*end == '\t');
		*end++ = 0;
		data = end;

		// Load method
		if (isCas) {
			end = findStringEnd(data);
			*end++ = 0;
			item->loadMethod = *data;
			data = end;
		} else {
			item->loadMethod = 0;
		}

//gotoxy(1,1); cprintf(" %x   ", item);
//getchar();
		item++;
	}
//cputs("DONE!!");
//getchar();
	item->name = NULL;
	itemsCount = item - list_start;
}

void updateList()
{
	// Clear list area
	_fillVRAM(0+4*80, 18*80, ' ');

	// Get remote list
	getRemoteList();
	processList();
}


// ========================================================
void printDefaultFooter()
{
	putstrxy(48,24, "F1:Help  F5:Download  RET:Search");
}

void printLineCounter()
{
	csprintf(buff, "\x13 %u/%u \x14\x17\x17\x17\x17", topLine+currentLine+1, itemsCount);
	putstrxy(35,23, buff);
}

void printHeader()
{
	textblink(1,1, 80, true);

	putstrxy(2,1, "File-Hunter Browser "VERSIONAPP);

	for (uint8_t i=0; i<80; i++) {
		setByteVRAM(3*80+i, 0x17);
		setByteVRAM(22*80+i, 0x17);
	}

	printDefaultFooter();
}

void printTabs()
{
	Panel_t *panel = panels;
	while (panel->name[0]) {
		if (panel == currentPanel) {
			putstrxy(panel->posx, 2, "\x18\x17\x17\x17\x17\x17\x19");
			putstrxy(panel->posx, 3, "\x16     \x16");
			putstrxy(panel->posx, 4, "\x1b     \x1a");
		} else {
			putstrxy(panel->posx, 2, "       ");
			putstrxy(panel->posx, 3, "       ");
			putstrxy(panel->posx, 4, "\x17\x17\x17\x17\x17\x17\x17");
		}
		putstrxy(panel->posx+1, 3, panel->name);
		panel++;
	}
}

void printRequestData()
{
	csprintf(buff, "[M]SX:%s", request.msx->name);
	putstrxy(62, 3, buff);
	csprintf(buff, "[C]char:%c", dos2_toupper(request.search.value[0]));
	putstrxy(72, 3, buff);
	csprintf(buff, "Search:\"%s\"", request.search.value);
	putstrxy(2, 24, buff);
}

void setSelectedLine(uint8_t y, bool selected)
{
	textblink(1, PANEL_FIRSTY+y, 80, selected);
}

void printItem(uint8_t y, ListItem_t *item)
{
	strncpy(buff, item->name, 64);
	putstrxy(2, y, buff);
	if (item->loadMethod) {
		csprintf(buff, " (%c)", item->loadMethod);
		putstrxy(67, y, buff);
	}
	csprintf(buff, "%lub", item->size);
	putstrxy(74, y, buff);
}

void printList()
{
	ListItem_t *item = list_start + topLine;
	uint8_t y = 5;

	while (item->name) {
		printItem(y, item);
		y++;
		item++;
		if (y > 22) break;
	}
}

void panelScrollUp()
{
	gettext(1,PANEL_FIRSTY+1, 80,PANEL_LASTY, buff);
	puttext(1,PANEL_FIRSTY, 80,PANEL_LASTY-1, buff);
	_fillVRAM(0+(PANEL_LASTY-1)*80, 80, ' ');
}

void panelScrollDown()
{
	gettext(1,PANEL_FIRSTY, 80,PANEL_LASTY-1, buff);
	puttext(1,PANEL_FIRSTY+1, 80,PANEL_LASTY, buff);
	_fillVRAM(0+(PANEL_FIRSTY-1)*80, 80, ' ');
}

void selectPanel(Panel_t *panel)
{
	currentPanel = panel;
	request.type = panel->type;

	ASM_EI; ASM_HALT;
	setSelectedLine(currentLine, false);
	currentLine = topLine = 0;
	printTabs();
	printRequestData();

	updateList();

	ASM_EI; ASM_HALT;
	printList();
	printLineCounter();
	setSelectedLine(currentLine, true);
}

// ========================================================
bool menu_loop()
{
	// Disable kanji mode if needed
	if (kanjiMode) {
		setKanjiMode(0);
	}

	// Initialize screen 0[80]
	textmode(BW80);
	textattr(0x71f4);
	setcursortype(NOCURSOR);
	redefineFunctionKeys();
	redefineCharPatterns();

	// Initialize header & panel
	printHeader();
	selectPanel(&panels[PANEL_FIRST]);

	// Menu loop
	bool end = false;
	uint8_t key;

	while (!end) {
		// Wait for a pressed key
		if (kbhit()) {
			key = dos2_toupper(getchar());
			switch (key) {
				case KEY_ESC:
				case 'X':
					end = true;
					break;
				case KEY_UP:
					if (currentLine > 0) {
						ASM_EI; ASM_HALT;
						setSelectedLine(currentLine, false);
						currentLine--;
						setSelectedLine(currentLine, true);
						printLineCounter();
					} else {
						if (topLine > 0) {
							topLine--;
							ASM_EI; ASM_HALT;
							panelScrollDown();
							printItem(PANEL_FIRSTY, list_start + topLine);
							printLineCounter();
						}
					}
					break;
				case KEY_DOWN:
					if (currentLine + topLine + 1 >= itemsCount)
						break;
					if (currentLine < PANEL_HEIGHT) {
						ASM_EI; ASM_HALT;
						setSelectedLine(currentLine, false);
						currentLine++;
						setSelectedLine(currentLine, true);
						printLineCounter();
					} else {
						if (topLine + currentLine < itemsCount - 1) {
							topLine++;
							ASM_EI; ASM_HALT;
							panelScrollUp();
							printItem(PANEL_LASTY, list_start + topLine + currentLine);
							printLineCounter();
						}
					}
					break;
				case KEY_TAB:
					if (isShiftKeyPressed()) {
						if (currentPanel == &panels[0]) {
							currentPanel = &panels[PANEL_LAST];
						} else {
							currentPanel--;
						}
					} else {
						currentPanel++;
						if (currentPanel->name[0] == 0) {
							currentPanel = &panels[PANEL_FIRST];
						}
					}
					selectPanel(currentPanel);
					break;
				case 'R':
					currentPanel = &panels[PANEL_ROM];
					selectPanel(currentPanel);
					break;
				case 'D':
					currentPanel = &panels[PANEL_DSK];
					selectPanel(currentPanel);
					break;
				case 'C':
					currentPanel = &panels[PANEL_CAS];
					selectPanel(currentPanel);
					break;
				case 'V':
					currentPanel = &panels[PANEL_VGM];
					selectPanel(currentPanel);
					break;
				case 'M':
					break;
				case KEY_RETURN:
					break;
				case '1':
					break;
				case '5':
				case KEY_SELECT:
					break;
			}
		}
	}
	return true;
}


// ========================================================
void restoreOriginalScreenMode() __naked
{
	// Restore original screen mode
	__asm
		ld   a, (_originalSCRMOD)
		or   a
		jr   nz, .screen1
		ld   ix, #INITXT				; Restore SCREEN 0
		jr   .bioscall
	.screen1:
		ld   ix, #INIT32				; Restore SCREEN 1
	.bioscall:
		JP_BIOSCALL
	__endasm;
}

void restoreScreen()
{
	// Clear & restore original screen parameters & colors
	__asm
		ld   ix, #DISSCR				; Disable screen
		BIOSCALL
		ld   ix, #INIFNK				; Restore function keys
		BIOSCALL
	__endasm;

	textattr(0x00f4);				// Clear blink
	_fillVRAM(0x0800, 240, 0);

	varLINL40 = originalLINL40;
	varFORCLR = originalFORCLR;
	varBAKCLR = originalBAKCLR;
	varBDRCLR = originalBDRCLR;

	if (kanjiMode) {
		// Restore kanji mode if needed
		setKanjiMode(kanjiMode);
	} else {
		// Restore original screen mode
		restoreOriginalScreenMode();
	}

	__asm
		ld   ix, #ENASCR
		BIOSCALL
	__endasm;

	// Restore abort routine
	dos2_setAbortRoutine((void*)0x0000);

	// Finish HGET library
	hgetfinish();
}

void printHelp()
{
	// Print help message
	cputs("## FileHunter Browser v"VERSIONAPP"\n"
		  "## by NataliaPC'2025\n\n"
		  "Usage:\n"
		  "  fh\n\n"
		  "See FH.HLP file for more information.\n");
	dos2_exit(1);
}

void checkArguments(char **argv, int argc)
{
	// Check if argument is a INI file
	if (argc && argv[0]) {
		printHelp();
	}
}

// ========================================================
int main(char **argv, int argc) __sdcccall(0)
{
	argv, argc;

	// Check arguments
	checkArguments(argv, argc);

	//Platform system checks
	checkPlatformSystem();

	initializeBuffers();
	heapPush();

	// Menu loop
	menu_loop();

	restoreScreen();
	return 0;

}

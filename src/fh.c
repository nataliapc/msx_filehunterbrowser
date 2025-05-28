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
#include "fh.h"
#include "hgetlib.h"
#include "patterns.h"
#include "structs.h"
#include "mod_downloadFiles.h"
#include "mod_searchString.h"
#include "mod_commandLine.h"
#include "mod_help.h"
#ifdef _DEBUG_
	#include "test.h"
#endif


// ========================================================
const char *BASEURL = "http://api.file-hunter.com/index4.php?base=1BA0&type=%s&msx=%s&char=%s&download=";

const ReqType_t reqType[] = {
	{""}, {"rom"}, {"dsk"}, {"cas"}, {"vgm"}
};

const ReqMSX_t reqMSX[] = {
	{"All",""}, {"1  ","1"}, {"2  ","2"}, {"2+ ","2+"}, {"TR ","turbo-r"}, {"",""}
};

const Panel_t panels[] = {
	{"[R]OM", &reqType[REQTYPE_ROM], 'r', 1},
	{"[D]SK", &reqType[REQTYPE_DSK], 'd', 8},
	{"[C]AS", &reqType[REQTYPE_CAS], 'c', 15},
	{"[V]GM", &reqType[REQTYPE_VGM], 'v', 22},
	{"", NULL, 0, 0}
};

Request_t request = {
	&reqType[REQTYPE_ROM],
	&reqMSX[REQMSX_ALL],
	{"a"},
	{""}
};

const char *downloadMessage[] = {
	"",
	"Error retrieving list! Try again",
	"Connection cancelled by user, try again",
	"No items found for your request",
	"List too long, press ENTER and refine your search",
	"Error downloading file",
	"File already exists",
	"Root directory full",
	"Disk full"
};


// ========================================================
static uint8_t msxVersionROM;
static uint8_t kanjiMode;
static uint8_t originalLINL40;
static uint8_t originalSCRMOD;
static uint8_t originalFORCLR;
static uint8_t originalBAKCLR;
static uint8_t originalBDRCLR;

char *buff;
Panel_t *currentPanel = &panels[PANEL_FIRST];
int16_t itemsCount;
int16_t topLine, currentLine;

#define MARQUEE_FIRST			200
#define MARQUEE_STEP			6
#define MARQUEE_LEN_OFFSET		20
uint8_t countDownMarquee;
uint8_t marqueePos = 0;
uint8_t marqueeLen = 0;

#define UNAPI_BUFFER_SIZE		1600
#define BUFF_SIZE				200
#define STACKPILE_SIZE			1024
#define DOWNLOAD_LIMIT_ADDR		(varTPALIMIT - BUFF_SIZE - STACKPILE_SIZE)
#define VRAM_LIMIT_ADDR			(131072L)
extern char *unapiBuffer;
ListItem_t *list_start;
ListItem_t *list_item;
char *list_raw;
bool structList;
uint32_t vramAddress;
bool isDownloading = false;
uint8_t downloadStatus;


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

	// Assign buffers
	buff = malloc(BUFF_SIZE);
}

void checkPlatformSystem()
{
	// Check TCP/IP UNAPI
	char ret = hgetinit((uint16_t)unapiBuffer);
	if (ret != ERR_TCPIPUNAPI_OK) {
#ifndef _DEBUG_
		if (ret == ERR_TCPIPUNAPI_NOT_TCPIP_CAPABLE) {
			die("TCP/IP UNAPI not fully capable!\x07\r\n");
		} else
		if (ret == ERR_HGET_INVALID_BUFFER) {
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
	_copyRAMtoVRAM((uint16_t)charPatters, 0x1000+24*8, 6*8);
}

inline bool isShiftKeyPressed()
{
	return varNEWKEY_row6.shift == 0;
}


// ========================================================
const char progressChar[] = {'\x1c', '\x1d'};
uint8_t progress = 0;
#define STATUS_PROGRESS_POS		1
void printActivityLed(bool reset)
{
	if (reset) progress = sizeof(progressChar) - 1;
	setByteVRAM(STATUS_PROGRESS_POS, progressChar[progress]);
	progress = (progress + 1) % sizeof(progressChar);
}

void HTTPStatusUpdate(bool isChunked)
{
	isChunked;
	printActivityLed(false);
}

void DataWriteCallback(char *rcv_buffer, int bytes_read)
{
	if (!bytes_read || !isDownloading) return;

	if (structList) {
		if (list_raw + bytes_read > (char*)DOWNLOAD_LIMIT_ADDR) {
			downloadStatus = DOWNLOAD_LIST_TOO_LONG;
			isDownloading = false;
			hgetcancel();
			return;
		}
		memcpy(list_raw, rcv_buffer, bytes_read);
		list_raw += bytes_read;

		// Recorre el buffer recibido de la lista de ListItem_t
		while ((char*)(list_item + 1) < list_raw) {
			if (!*((uint32_t *)list_item)) {	// Si el ListItem_t no tiene nombre, es el final de la lista
				char *ptr = ((char *)list_item) + sizeof(uint32_t);	// ajusta el puntero a la lista al principio de lista de strings
				int16_t size = list_raw - ptr;	// Calcula si hay algo del inicio de la lista de strings que copiar a VRAM
				if (size > 0) {					// Copia el inicio de la lista de strings a VRAM
					msx2_copyToVRAM((uint16_t)ptr, vramAddress, size);
					vramAddress += size;		// Ajusta la direccion de VRAM para la siguiente copia
				}
				list_raw -= size;				// Ajusta el buffer al final de la lista de ListItem_t para los chunks siguientes
				structList = false;				// Define que ya solo quedan datos de la lista de strings
				break;
			}
			++list_item;
		}
	} else {
		if (vramAddress + bytes_read >= VRAM_LIMIT_ADDR) {
			downloadStatus = DOWNLOAD_LIST_TOO_LONG;
			isDownloading = false;
			hgetcancel();
		} else {								// copia el buffer recibido a VRAM
			msx2_copyToVRAM((uint16_t)rcv_buffer, vramAddress, bytes_read);
			vramAddress += bytes_read;
		}
	}
}

void formatURL(char *buff, uint16_t fileNum)
{
	char *buffSearch = (char*)malloc(SEARCH_MAX_SIZE + 1);
	strcpy(buffSearch, request.search.value);
	strReplaceChar(buffSearch, ' ', '+');
	csprintf(buff, BASEURL, request.type->value, request.msx->value, buffSearch);
	if (fileNum != -1) {
		csprintf(buffSearch, "%u", fileNum);
		strcat(buff, buffSearch);
	}
	free(SEARCH_MAX_SIZE + 1);
}

void getRemoteList()
{
	vramAddress = VRAM_START;
	downloadStatus = DOWNLOAD_OK;
	isDownloading = true;
	structList = true;

	formatURL(buff, -1);
	resetList();			// popHeap() + pushHeap()

#ifdef _DEBUG_
	uint16_t i = TEST_SIZE, size = 1024, pos = 0, cnt;
	while (i) {
		size += (rand() % 10) - 5;
		if (i < size) {
			size = i;
		}
		memcpy(unapiBuffer, &test_txt[pos], size);
		DataWriteCallback(unapiBuffer, size);
		HTTPStatusUpdate(true);
		pos += size;
		i -= size;

		cnt = varJIFFY;
		while (varJIFFY-cnt < 3) {
			ASM_EI; ASM_HALT;
		}
	}
#else
	HgetReturnCode_t ret = hget(
		buff,						// URL
		(int)HTTPStatusUpdate,		// progress_callback
		(int)DataWriteCallback,		// data_write_callback
		0,							// content_size_callback
		false						// enableKeepAlive
	);
	if (ret != ERR_TCPIPUNAPI_OK)
	{
		resetList();
		if (downloadStatus == DOWNLOAD_OK) {
			if (ret == ERR_HGET_ESC_CANCELLED)
				downloadStatus = DOWNLOAD_CANCELLED;
			else
				downloadStatus = DOWNLOAD_LIST_ERROR;
		}
	}
#endif

	isDownloading = false;
	itemsCount = list_item - list_start;
	if (!itemsCount && downloadStatus == DOWNLOAD_OK) {
		downloadStatus = DOWNLOAD_EMPTY;
	}

	heap_top = list_raw;
	initializeBuffers();
	printActivityLed(true);
}


// ========================================================
void clearBlinkList()
{
	_fillVRAM(0x0800 + (PANEL_FIRSTY-1)*10, PANEL_HEIGHT*10, 0);
}

#define UPDATING_POSY	11
void printUpdatingListMessage()
{
	fillBlink(1,UPDATING_POSY, 3,80, true);
	putstrxy(31, UPDATING_POSY+1, "Retrieving list...");
}

void removeUpdateMessage()
{
	_fillVRAM(0+UPDATING_POSY*80, 80, ' ');
	fillBlink(1,UPDATING_POSY, 3,80, false);
}

void printDefaultFooter()
{
	putstrxy(48,24, "F1:Help  F5:Download  RET:Search");
}

void printLineCounter()
{
	csprintf(buff, "\x13 %u/%u \x14\x17\x17\x17\x17",
		itemsCount ? topLine+currentLine+1 : 0,
		itemsCount);
	putstrxy(35,23, buff);
}

void printHeader()
{
	textblink(1,1, 80, true);

	putstrxy(2,1, "\x85 File-Hunter Browser v"VERSIONAPP);
	putstrxy(66,1, AUTHORAPP);

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
		++panel;
	}
}

void printRequestData()
{
	csprintf(buff, "[M]SX:%s", request.msx->name);
	putstrxy(71, 3, buff);

	memset(buff, ' ', SEARCH_MAX_SIZE);
	buff[SEARCH_MAX_SIZE] = '\0';
	putstrxy(9,24, buff);

	csprintf(buff, "Search:%s", request.search.value);
	putstrxy(2, 24, buff);
}

void resetSelectedLine()
{
	currentLine = topLine = 0;
}

void printItem(uint8_t y, ListItem_t *item)
{
	#define MAX_NAME_SIZE	70
	#define ITEM_POS_LOAD	67
	#define ITEM_POS_SIZE	78

	if (!item->name) return;

	// Add name
	msx2_copyFromVRAM((uint32_t)item->name, (uint16_t)buff, 80);
	buff[80] = '\0';
	marqueeLen = strlen(buff);
	if (marqueePos) {
		memncpy(buff, &buff[marqueePos], '\0', MAX_NAME_SIZE);
	}

	uint8_t len = marqueeLen - marqueePos;
	if (len > MAX_NAME_SIZE) {
		len = MAX_NAME_SIZE;
	}
	memset(&buff[len], ' ', 80);
	buff[80] = '\0';

	// Add load method
	if (item->loadMethod) {
		csprintf(heap_top, " (%c)     ", item->loadMethod);
		memncpy(&buff[ITEM_POS_LOAD], heap_top, '\0', 5);
	}

	// Add size
	formatSize(heap_top, item->size);
	strcpy(&buff[ITEM_POS_SIZE-strlen(heap_top)], heap_top);

	putlinexy(2,y, 78, buff);
}

void setSelectedLine(bool selected)
{
	countDownMarquee = MARQUEE_FIRST;	// Reset marquee counter
	marqueePos = 0;						// Reset marquee position
	printItem(PANEL_FIRSTY + currentLine, list_start + topLine + currentLine);

	textblink(1, PANEL_FIRSTY+currentLine, 80, selected);
}

void printList()
{
	printLineCounter();

	if (downloadStatus == DOWNLOAD_OK) {
		ListItem_t *item = list_start + topLine;
		uint8_t y = 5;

		while (y <= PANEL_LASTY) {
			if (item->name) {
				printItem(y++, item);
				++item;
			} else {
				_fillVRAM((y-1)*80, (PANEL_LASTY-y+1)*80, ' ');
				break;
			}
		}
		setSelectedLine(true);
	} else {
		// Print error message
		setSelectedLine(false);
		putstrxy(2, PANEL_FIRSTY, downloadMessage[downloadStatus]);
		putch(0x07);
	}
}

void panelScrollUp()
{
	msx2_copyFromVRAM(0+(PANEL_FIRSTY)*80, (uint16_t)heap_top, (PANEL_HEIGHT-1)*80);
	msx2_copyToVRAM((uint16_t)heap_top, 0+(PANEL_FIRSTY-1)*80, (PANEL_HEIGHT-1)*80);
	_fillVRAM(0+(PANEL_LASTY-1)*80, 80, ' ');
}

void panelScrollDown()
{
	msx2_copyFromVRAM(0+(PANEL_FIRSTY-1)*80, (uint16_t)heap_top, (PANEL_HEIGHT-1)*80);
	msx2_copyToVRAM((uint16_t)heap_top, 0+(PANEL_FIRSTY)*80, (PANEL_HEIGHT-1)*80);
	_fillVRAM(0+(PANEL_FIRSTY-1)*80, 80, ' ');
}

void clearListArea()
{
	_fillVRAM(0+4*80, 18*80, ' ');
}


// ========================================================
void resetList()
{
	heapPop();
	heapPush();

	list_start = list_item = (ListItem_t*)heap_top;
	list_raw = heap_top;
	list_start->name = 0L;
}

ListItem_t* getCurrentItem()
{
	return list_start + topLine + currentLine;
}

void updateList()
{
	ASM_EI; ASM_HALT;
	clearBlinkList();
	resetSelectedLine();

	// Clear list area
	clearListArea();

	printUpdatingListMessage();

	// Get remote list
	getRemoteList();

	ASM_EI; ASM_HALT;
	removeUpdateMessage();
	printList();
}

void selectPanel(Panel_t *panel)
{
	currentPanel = panel;
	request.type = panel->type;

	ASM_EI; ASM_HALT;
	printTabs();
	printRequestData();

	updateList();
}

inline void nextTargetMSX()
{
	++request.msx;
	if (request.msx->name[0] == 0) {
		request.msx = &reqMSX[REQMSX_ALL];
	}
	printRequestData();
	updateList();
}


// ========================================================
void menu_loop()
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
	selectPanel(currentPanel);

	// Menu loop
	int8_t  newPanel = PANEL_NONE;
	bool end = false;
	bool resetMarquee = true;

	while (!end) {
		// Wait for a pressed key
		ASM_EI; ASM_HALT;
		if (kbhit()) {
			switch(dos2_toupper(getch())) {
				case KEY_UP:
					if (currentLine > 0) {
						setSelectedLine(false);
						--currentLine;
						setSelectedLine(true);
					} else {
						if (topLine > 0) {
							--topLine;
							panelScrollDown();
							printItem(PANEL_FIRSTY, list_start + topLine);
						}
					}
					printLineCounter();
					break;
				case KEY_DOWN:
					if (currentLine + topLine + 1 < itemsCount) {
						if (currentLine < PANEL_HEIGHT - 1) {
							setSelectedLine(false);
							++currentLine;
							setSelectedLine(true);
						} else {
							if (topLine + currentLine + 1 < itemsCount) {
								++topLine;
								panelScrollUp();
								printItem(PANEL_LASTY, list_start + topLine + currentLine);
							}
						}
						printLineCounter();
					}
					break;
				case KEY_RIGHT:
					if (itemsCount) {
						topLine += PANEL_HEIGHT;
						if (topLine + PANEL_HEIGHT >= itemsCount) {
							setSelectedLine(false);
							if (PANEL_HEIGHT > itemsCount) {
								topLine = 0;
								currentLine = itemsCount - 1;
							} else {
								currentLine = PANEL_HEIGHT - 1;
								topLine = itemsCount - currentLine - 1;
							}	
							setSelectedLine(true);
						}
						printList();
					}
					break;
				case KEY_LEFT:
					if (itemsCount) {
						topLine -= PANEL_HEIGHT;
						if (topLine < 0) {
							setSelectedLine(false);
							resetSelectedLine();
							setSelectedLine(true);
						}
						printList();
					}
					break;
				case KEY_TAB:
					if (isShiftKeyPressed()) {
						if (currentPanel == &panels[0]) {
							currentPanel = &panels[PANEL_LAST];
						} else {
							--currentPanel;
						}
					} else {
						++currentPanel;
						if (currentPanel->name[0] == 0) {
							currentPanel = &panels[PANEL_FIRST];
						}
					}
					selectPanel(currentPanel);
					break;
				case 'R':
					newPanel = PANEL_ROM; break;
				case 'D':
					newPanel = PANEL_DSK; break;
				case 'C':
					newPanel = PANEL_CAS; break;
				case 'V':
					newPanel = PANEL_VGM; break;
				case 'M':
					nextTargetMSX();
					break;
				case KEY_RETURN:
					changeSearchString();
					break;
				case '1':
					showHelpWindow();
					break;
				case '5':
				case KEY_SELECT:
					downloadFile();
					break;
				case KEY_ESC:
					++end;
			}
			if (newPanel != PANEL_NONE) {
				if (currentPanel != &panels[newPanel]) {
					currentPanel = &panels[newPanel];
					selectPanel(currentPanel);
				}
				newPanel = PANEL_NONE;
			}
		}
		if (itemsCount && marqueeLen > MAX_NAME_SIZE) {
			if (!countDownMarquee) {
				countDownMarquee = MARQUEE_STEP;
				if (marqueePos < marqueeLen - MARQUEE_LEN_OFFSET) {
					marqueePos++;
				} else {
					countDownMarquee = MARQUEE_FIRST;
					marqueePos = 0;
				}
				printItem(PANEL_FIRSTY + currentLine, list_start + topLine + currentLine);
			} else {
				countDownMarquee--;
			}
		}
	}
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
	// Finish HGET library
	hgetfinish();

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
}

// ========================================================
int main(char **argv, int argc) __sdcccall(0)
{
	argv, argc;

	// Check arguments
	checkArguments(argv, argc);

	//Platform system checks
	checkPlatformSystem();

	resetList();
	initializeBuffers();

	// Menu loop
	menu_loop();

	restoreScreen();
	return 0;
}

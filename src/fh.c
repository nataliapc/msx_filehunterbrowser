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
#include "structs.h"
#include "mod_downloadFiles.h"
#include "mod_searchString.h"
#include "mod_commandLine.h"
#include "mod_help.h"
#include "mod_disposable.h"
#ifdef _DEBUG_
	#include "test.h"
#endif

// ========================================================
const char *BASEURL = "http://api.file-hunter.com/index4.php?base=1BA0&type=%s&msx=%s&char=%s&download=";

const ReqType_t reqType[] = {
	{"", ""},
	{"rom", ".ROM"},
	{"dsk", ".DSK"},
	{"cas", ".CAS"},
	{"vgm", ".ZIP"}
};

const ReqMSX_t reqMSX[] = {
	{"All",""}, {"1  ","1"}, {"2  ","2"}, {"2+ ","2+"}, {"TR ","turbo-r"}, {"",""}
};

const Panel_t panels[] = {
	{"[R]OM", &reqType[REQTYPE_ROM], 'r', 1,  "[ ROM files ]"},
	{"[D]SK", &reqType[REQTYPE_DSK], 'd', 8,  "[ Disk images ]"},
	{"[C]AS", &reqType[REQTYPE_CAS], 'c', 15, "[ CAS tape dumps ]"},
	{"[V]GM", &reqType[REQTYPE_VGM], 'v', 22, "[ VGM music files ]"},
	{"", NULL, 0, 0, NULL}
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
extern void HEAP_start;

uint8_t msxVersionROM;
uint8_t kanjiMode;
uint8_t originalLINL40;
uint8_t originalSCRMOD;
uint8_t originalFORCLR;
uint8_t originalBAKCLR;
uint8_t originalBDRCLR;

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
char *user_agent;
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
	net_waitConnected(60*10);	// Wait for connection (10 seconds on NTSC, 12 on PAL)

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

void printLineCounter()
{
	csprintf(buff, "\x13 %u/%u \x14\x17\x17\x17\x17",
		itemsCount ? topLine+currentLine+1 : 0,
		itemsCount);
	putstrxy(35,23, buff);
}

void printTabs()
{
	Panel_t *panel = panels;
	while (panel->name[0]) {
		if (panel == currentPanel) {
			putstrxy(panel->posx, 2, "\x18\x17\x17\x17\x17\x17\x19");
			putstrxy(panel->posx, 3, "\x16     \x16");
			putstrxy(panel->posx, 4, "\x1b     \x1a");
			// Print description
			_fillVRAM(34, MAX_PANEL_DESCRIPTION, ' ');
			putstrxy(35, 1, panel->description);
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

void resetMarquee()
{
	countDownMarquee = MARQUEE_FIRST;	// Reset marquee counter
	marqueePos = 0;						// Reset marquee position
}

void printCurrentLine()
{
	printItem(PANEL_FIRSTY + currentLine, list_start + topLine + currentLine);
}

void setSelectedLine(bool selected)
{
	printCurrentLine();
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
	// Initialize panel
	selectPanel(currentPanel);

	// Menu loop
	int8_t  newPanel = PANEL_NONE;
	bool end = false;
	bool shiftPressed;
	char key;

	while (!end) {
		// Wait for a pressed key
		ASM_EI; ASM_HALT;
		if (kbhit()) {
			resetMarquee();
			key = dos2_toupper(getch());
			shiftPressed = isShiftKeyPressed();
			switch(key) {
				case KEY_UP:
					if (!itemsCount) break;
					if (currentLine > 0) {
						setSelectedLine(false);
						--currentLine;
						setSelectedLine(true);
					} else {
						if (topLine > 0) {
							--topLine;
							panelScrollDown();
							printCurrentLine();
						}
					}
					printLineCounter();
					break;
				case KEY_DOWN:
					if (!itemsCount) break;
					if (currentLine + topLine + 1 < itemsCount) {
						if (currentLine < PANEL_HEIGHT - 1) {
							setSelectedLine(false);
							++currentLine;
							setSelectedLine(true);
						} else {
							if (topLine + currentLine + 1 < itemsCount) {
								++topLine;
								panelScrollUp();
								printCurrentLine();
							}
						}
						printLineCounter();
					}
					break;
				case KEY_RIGHT:
					if (!itemsCount) break;
					topLine += PANEL_HEIGHT;
					if (topLine + PANEL_HEIGHT >= itemsCount || shiftPressed) {
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
					break;
				case KEY_LEFT:
					if (!itemsCount) break;
					if (topLine + currentLine > 0 || shiftPressed) {
						setSelectedLine(false);
						topLine -= PANEL_HEIGHT;
						if (topLine < 0 || shiftPressed) {
							resetSelectedLine();
						}
						setSelectedLine(true);
					}
					printList();
					break;
				case KEY_TAB:
					if (shiftPressed) {
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
					if (!itemsCount) break;
					downloadFile();
					break;
				case KEY_ESC:
					++end;
				default:
					printCurrentLine();
					break;
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
					resetMarquee();
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

	// Initialize screen
	initializeScreen();

	// Initialize program
	resetList();
	initializeBuffers();

	// Menu loop
	menu_loop();

	restoreScreen();
	return 0;
}

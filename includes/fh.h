/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "structs.h"
#include "conio_aux.h"


#define VERSIONAPP		"1.0.1"
#define AUTHORAPP		"NataliaPC'2025"
#define FHBROWSER_AGENT	"MSX_FHBrowser (MSX-DOS2)"

#define VRAM_START		0x1ba0

extern const char *BASEURL;

#define PANEL_FIRSTY	5
#define PANEL_LASTY		22
#define PANEL_HEIGHT	(PANEL_LASTY - PANEL_FIRSTY + 1)


#define REQTYPE_ALL		0
#define REQTYPE_ROM		1
#define REQTYPE_DSK		2
#define REQTYPE_CAS		3
#define REQTYPE_VGM		4
extern const ReqType_t reqType[];

#define REQMSX_ALL		0
#define REQMSX_MSX1		1
#define REQMSX_MSX2		2
#define REQMSX_MSX2P	3
#define REQMSX_MSXTR	4
extern const ReqMSX_t reqMSX[];

#define PANEL_NONE		-1
#define PANEL_FIRST		PANEL_ROM
#define PANEL_ROM		0
#define PANEL_DSK		1
#define PANEL_CAS		2
#ifndef DISABLE_VGM
	#define PANEL_VGM		3
	#define PANEL_LAST		PANEL_VGM
#else
	#define PANEL_LAST		PANEL_CAS
#endif
extern const Panel_t panels[];
extern Panel_t *currentPanel;

extern Request_t request;
enum {
	DOWNLOAD_OK,			// No error
	DOWNLOAD_LIST_ERROR,	// List Error (generic)
	DOWNLOAD_CANCELLED,		// Cancelled by user
	DOWNLOAD_EMPTY,			// Empty list
	DOWNLOAD_LIST_TOO_LONG,	// List too long
	DOWNLOAD_FILE_ERROR,	// File error (generic)
	DOWNLOAD_FILE_EXISTS,	// File already exists
	DOWNLOAD_ROOT_FULL,		// Root directory full
	DOWNLOAD_DISK_FULL		// Disk full
};
extern uint8_t downloadStatus;
extern const char *downloadMessage[];

extern char *buff;
extern ListItem_t *list_start;


// ========================================================
// Function declarations

void resetList();
void formatURL(char *buff, uint16_t fileNum);
void HTTPStatusUpdate(bool isChunked);

void restoreScreen();
void resetSelectedLine();
void setSelectedLine(bool selected);
void updateList();
void printActivityLed(bool reset);
void printList();
void printRequestData();
ListItem_t* getCurrentItem();

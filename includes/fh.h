/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "structs.h"


#define VERSIONAPP		"0.2.0"
#define BASEURL			"http://api.file-hunter.com/index3.php?type=%s&msx=%s&char=%s&download=%s"


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
#define PANEL_VGM		3
#define PANEL_LAST		PANEL_VGM
extern const Panel_t panels[];

extern Request_t request;
enum {
	DOWNLOAD_OK,			// No error
	DOWNLOAD_ERROR,			// Error
	DOWNLOAD_EMPTY,			// Empty list
	DOWNLOAD_LIST_TOO_LONG	// List too long
};
extern uint8_t downloadStatus;

extern char *buff;
extern ListItem_t *list_start;


// ========================================================
// Function declarations

void setByteVRAM(uint16_t vram, uint8_t value) __sdcccall(0);
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);
void _copyRAMtoVRAM(uint16_t memory, uint16_t vram, uint16_t size) __sdcccall(0);
void restoreScreen();

void resetSelectedLine();
void setSelectedLine(bool selected);
void updateList();
void printList();
void printRequestData();
ListItem_t* getCurrentItem();

/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "structs.h"


#define VERSIONAPP		"0.1.0"
#define BASEURL			"http://api.file-hunter.com/index3.php?type=%s&msx=%s&char=%s&download=%s"


#define REQTYPE_ALL		0
#define REQTYPE_ROM		1
#define REQTYPE_DSK		2
#define REQTYPE_CAS		3
#define REQTYPE_VGM		4
const ReqType_t reqType[] = {
	{""}, {"rom"}, {"dsk"}, {"cas"}, {"vgm"}
};

#define REQMSX_ALL		0
#define REQMSX_MSX1		1
#define REQMSX_MSX2		2
#define REQMSX_MSX2P	3
#define REQMSX_MSXTR	4
const ReqMSX_t reqMSX[] = {
	{"All",""}, {"1  ","1"}, {"2  ","2"}, {"2+ ","2+"}, {"TR ","turbo-r"}
};

#define PANEL_FIRST		PANEL_ROM
#define PANEL_ROM		0
#define PANEL_DSK		1
#define PANEL_CAS		2
#define PANEL_VGM		3
#define PANEL_LAST		PANEL_VGM
Panel_t panels[] = {
	{"[R]OM", &reqType[REQTYPE_ROM], 'r', 1},
	{"[D]SK", &reqType[REQTYPE_DSK], 'd', 8},
	{"[C]AS", &reqType[REQTYPE_CAS], 'c', 15},
	{"[V]GM", &reqType[REQTYPE_VGM], 'v', 22},
	{"", NULL, 0, 0}
};


Request_t request = {
	&reqType[REQTYPE_ROM],
	&reqMSX[REQMSX_ALL],
	{"maze"},
	{""}
};


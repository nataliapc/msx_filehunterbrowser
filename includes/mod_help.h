/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once


// ========================================================
#define HELPWIN_POSX	20
#define HELPWIN_POSY	6
#define HELPWIN_HEIGHT	16

typedef struct {
	uint8_t x;
	uint8_t y;
	char   *str;
} HelpWin_t;


// ========================================================
void showHelpWindow();

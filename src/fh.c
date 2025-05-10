/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma opt_code_size
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "msx_const.h"
#include "heap.h"
#include "utils.h"
#include "dos.h"
#include "conio.h"


#define VERSIONAPP "0.1.0"


// ========================================================
static uint8_t msxVersionROM;
static uint8_t kanjiMode;
static uint8_t originalLINL40;
static uint8_t originalSCRMOD;
static uint8_t originalFORCLR;
static uint8_t originalBAKCLR;
static uint8_t originalBDRCLR;

char *buff;


// ========================================================
// Function declarations

void restoreScreen();


// ========================================================
static void abortRoutine()
{
	restoreScreen();
	dos2_exit(1);
}

static void checkPlatformSystem()
{
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


// ========================================================
void launch_exec()
{
	// Restore screen
	restoreScreen();
	// Execute command
//	execv(entry->exec);
}

bool menu_loop()
{
	// Menu loop
	bool end = false;
	uint8_t key;

	while (!end) {
		// Wait for a pressed key
		if (kbhit()) {
			key = getchar();
			switch (key) {
				case KEY_ESC:
					end = true;
					break;
				case KEY_UP:
					break;
				case KEY_DOWN:
					break;
				case KEY_RETURN:
				case KEY_SELECT:
				case KEY_SPACE:
					launch_exec();
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

void printHelp()
{
	// Print help message
	cputs("## NMENU v"VERSIONAPP"\n"
		  "## by NataliaPC 2025\n\n"
		  "Usage:\n"
		  "  nmenu <INI file>\n\n"
		  "See NMENU.HLP file for more information.\n");
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

	// A way to avoid using low memory when using BIOS calls from DOS
	if (heap_top < (void*)0x8000)
		heap_top = (void*)0x8000;

	// Initialize generic string buffer
	buff = malloc(200);

	// Check arguments
	checkArguments(argv, argc);

	//Platform system checks
	checkPlatformSystem();

	// Menu loop
	menu_loop();

	restoreScreen();
	return 0;
}

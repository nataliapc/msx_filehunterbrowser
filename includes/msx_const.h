/*
#####################################################################
	MSX Constants & structs 
	by NataliaPC
	[2024-09-12]
#####################################################################
*/
#pragma once
#include <stdint.h>


// ========================================================
// Tools
#define DOSCALL		call 5
#define DOSJP		jp 5
#define BIOSCALL	ld iy,(#EXPTBL-1) \
                	call CALSLT
#define JP_BIOSCALL	ld iy,(#EXPTBL-1) \
                   	jp CALSLT

#define ASM_HALT	__asm halt __endasm;
#define ASM_DI		__asm di __endasm;
#define ASM_EI		__asm ei __endasm;

#define ADDR_POINTER_BYTE(X)	(*((uint8_t*)X))
#define ADDR_POINTER_WORD(X)	(*((uint16_t*)X))


// ========================================================
// MSX Generation (used by ROM address MSXVER @ 0x002d)
#define GEN_MSX1	0	// MSX 1
#define GEN_MSX2	1	// MSX 2
#define GEN_MSX2P	2	// MSX 2+
#define GEN_TURBOR	3	// MSX TurboR

// ========================================================
// MSX I/O Ports
__sfr __at (0x98) IO_VDP1;
__sfr __at (0x99) IO_VDP2;


// ========================================================
// MSX BIOS
// http://map.grauw.nl/resources/msxbios.php
//
#define RDSLT	0x00c		// Reads value of address in another slot
#define CALSLT 	0x01c		// Executes inter-slot call [Input: IY-High byte with slot ID | IX-The address that will be called]
#define INIFNK	0x03e		// Initialises the contents of the function keys [Changes: All]
#define DISSCR	0x041		// Inhibits the screen display [Changes: AF, BC]
#define ENASCR	0x044		// Enables the screen display [Changes: AF, BC]
#define WRTVDP	0x047		// Write data in the VDP-register [Input: B-Data to write | C-Register][Changes: AF,BC]
#define RDVRM	0x04a		// Reads the content of VRAM [Input: HL-Address read][Output: A-Data][Changes: AF]
#define WRTVRM	0x04d		// Writes data in VRAM [Input: HL-Address write|A-Data][Changes:AF]
#define SETRD	0x050		// Sets VRAM address to read [Input: HL-VRAM address (00000h~03FFFh)][Changes: AF]
#define SETWRT	0x053		// Sets VRAM address to write [Input: HL-VRAM address (00000h~03FFFh)][Changes: AF]
#define FILVRM	0x056		// Fill VRAM with value [Input: A-Data byte|BC-Length|HL-VRAM address][Changes: AF,BC]
#define LDIRMV	0x059		// Block transfer to memory from VRAM [Input: BC-Length|DE-Start memory address|HL-VRAM address][Changes: ALL]
#define LDIRVM	0x05c		// Block transfer to VRAM from memory [Input: BC-Length|DE-VRAM address|HL-Start memory address][Changes: ALL]
#define CHGMOD	0x05f		// Switches to given screen mode [Input: A-Screen mode][Changes: ALL]
#define CHGCLR	0x062		// Changes the screen colors [Input: Foreground color in FORCLR | Background color in BAKCLR | Border color in BDRCLR]
#define CLRSPR	0x069		// Initialises all sprites [Changes: All]
#define INITXT	0x06c		// Switches to SCREEN 0 (text screen with 40×24 characters)
#define INIT32	0x06f		// Switches to SCREEN 1 (text screen with 32×24 characters)
#define INIGRP	0x072		// Switches to SCREEN 2 (high resolution screen with 256×192 pixels)
#define SETGRP	0x07e		// Switches VDP to SCREEN 2 mode
#define CLSSCR	0x0c3		// Clears the screen [Changes: AF, BC, DE] Zero flag must be set to be able to run this routine. XOR A will do fine most of the time
#define POSIT	0x0c6		// Moves cursor to the specified position [Input: H-Y pos | L-X pos]
// MSX2
#define EXTROM	0x15f		// Calls a routine in the extended BIOS ROM [Input: IX-Address of routine in SUB-ROM][Output: Depends on the routine
#define BIGFIL	0x16b		// Same function as FILVRM with 16-bit VRAM-address [Input: A-Data|BC-Length|HL-VRAM address][Changes: AF,BC]
#define NSETRD	0x16e		// Same function as SETRD with 16-bit VRAM-address [Input: HL-VRAM address (00000h~0FFFFh)][Changes: AF]
#define NSTWRT	0x171		// Same function as SETWRT with 16-bit VRAM-address [Input: HL-VRAM address (00000h~0FFFFh)][Changes: AF]
#define NRDVRM	0x174		// Reads VRAM like in RDVRM with 16-bit VRAM-address [Input: HL-VRAM address (00000h~0FFFFh)][Output: A-Data][Changes: F]
#define NWRVRM	0x177		// Writes to VRAM like in WRTVRM with 16-bit VRAM-address [Input: A-Data|HL-VRAM address (00000h~0FFFFh)][Changes: AF]


// ========================================================
// MSX-DOS 1.x BIOS
// http://map.grauw.nl/resources/dos2_functioncalls.php
//
#define TERM0   0x00		// Program terminate			CPM MSX1
#define CONIN   0x01		// Console input				CPM MSX1
#define CONOUT  0x02		// Console output				CPM MSX1
#define INNOE   0x08		// Console input w/o echo		    MSX1
#define CONST   0x0B		// Console status				CPM MSX1

#define FOPEN   0x0F		// Open file (FCB)				CPM MSX1
#define FCLOSE  0x10		// Close file (FCB)				CPM MSX1
#define FDELETE 0x13		// Delete file (FCB)			CPM MSX1
#define RDSEQ   0x14		// Sequential read (FCB)		CPM MSX1
#define WRSEQ   0x15		// Sequential write FCB)		CPM MSX1
#define FMAKE   0x16		// Create file (FCB)			CPM MSX1

#define CURDRV  0x19		// Get current drive			CPM MSX1
#define SETDTA  0x1A		// Set disk transfer address	CPM MSX1

#define RDRND   0x21		// Random read (FCB)			CPM MSX1
#define WRRND   0x22		// Random write (FCB)			CPM MSX1
#define FSIZE   0x23		// Get file size (FCB)			CPM MSX1
#define WRBLK   0x26		// Random block read (FCB)		    MSX1
#define RDBLK   0x27		// Random block write (FCB)		    MSX1


// ========================================================
// MSX-DOS Nextor 2.1 BIOS
// https://github.com/Konamiman/Nextor/blob/v2.1/docs/Nextor%202.1%20Programmers%20Reference.md#3-new-function-calls
//
#define FOUT    0x71		// Turn On/Off the fast STROUT mode. When enabled, the _STROUT and _ZSTROUT functions will work faster.
#define ZSTROUT 0x72		// Prints on the screen the string pointed by DE, the string must be terminated with a zero character.
#define RDDRV   0x73		// Reads sectors directly from a drive (no filesystem dependent)
#define WRDRV   0x74		// Writes sectors directly from a drive (no filesystem dependent)
#define RALLOC  0x75
#define DSPACE  0x76		// Get drive space information
#define LOCK    0x77		// Lock/unlock a drive, or get lock state for a drive
#define GDRVR   0x78		// Get information about a device driver
#define GDLI    0x79		// Get information about a drive letter
#ifndef _DOSLIB_
#define GPART   0x7a		// Get information about a device partition
#define CDRVR   0x7b		// Call a routine in a device driver
#define MAPDRV  0x7c		// Map a drive letter to a driver and device
#define Z80MODE 0x7d		// Enable or disable the Z80 access mode for a driver
#define GETCLUS 0x7e		// Get information for a cluster on a FAT drive
#endif//_DOSLIB_


// ========================================================
// MSX/MSX2 system constants
// http://map.grauw.nl/resources/msxsystemvars.php
// https://www.msx.org/wiki/System_variables_and_work_area
//
#define LOCALE		0x002b	// (BYTE) 7 6 5 4 3 2 1 0
//                                    │ │ │ │ └─┴─┴─┴── Character set
//                                    │ │ │ │           0=Japanese, 1=International, 2=Korean
//                                    │ └─┴─┴────────── Date format
//                                    │                 0=Y-M-D, 1=M-D-Y, 2=D-M-Y
//                                    └──────────────── Default interrupt frequency
//                                                      0=60Hz, 1=50Hz
#define KBOARD		0x002c	// (BYTE) 7 6 5 4 3 2 1 0
//                                    │ │ │ │ └─┴─┴─┴── Keyboard type
//                                    │ │ │ │           0=Japanese, 1=International
//                                    │ │ │ │           2=French (AZERTY), 3=UK, 4=German (DIN)
//                                    └─┴─┴─┴────────── Basic version
//                                                      0=Japanese, 1=International
#define MSXVER		0x002d	// (BYTE) MSX version number. 0=MSX1 1=MSX2 2=MSX2+ 3=TurboR
#define TRMIDI		0x002e	// (BYTE) Bit0: if 1 then MSX-MIDI is present internally (MSX TurboR only)
#define LINL40		0xf3ae	// (BYTE) Width for SCREEN 0 (default 37)
#define LINLEN		0xf3b0	// (BYTE) Current screen width per line
#define CRTCNT		0xf3b1	// (BYTE) Number of lines of current screen (default 24)
#define CLIKSW		0xf3db	// (BYTE) Key click switch. (0 = Disabled / 1 = Enabled)
#define CSRY		0xf3dc	// (BYTE) Y-coordinate of text cursor
#define CSRX		0xf3dd	// (BYTE) X-coordinate of text cursor
#define RG0SAV		0xf3df	// (BYTE) Mirror of VDP Register 0 (R#0)
#define FORCLR		0xf3e9	// (BYTE) Foreground colour
#define BAKCLR		0xf3ea	// (BYTE) Background colour
#define BDRCLR		0xf3eb	// (BYTE) Border colour
#define REPCNT		0xf3f7	// (BYTE) Delay until the auto-repeat of the key begins. (50 by default)
#define PUTPNT		0xf3f8	// (WORD) Address in the keyboard buffer where a character will be written
#define GETPNT		0xf3fa	// (WORD) Address in the keyboard buffer where the next character is read
#define FNKSTR		0xf87f	// (10*16 bytes) Value of the function keys
#define MODE		0xfafc	// (BYTE) Flags for VRAM
#define HOKVLD		0xfb20	// (BYTE) Bit 0 indicates presence of extended Bios (EXTBIO) [0:No bios | 1:Exists at least one]
#define VCBA		0xfb41	// (37 bytes) Data for PSG voice A
#define VCBB		0xfb66	// (37 bytes) Data for PSG voice B
#define VCBC		0xfb8b	// (37 bytes) Data for PSG voice C
#define NEWKEY		0xfbe5	// (11 bytes) Status of each keyboard matrix row. The status is updated by the KEYINT interrupt routine.
							//            https://www.msx.org/wiki/Keyboard
#define KEYBUF		0xfbf0	// (40 bytes) Key code buffer [0xfbf0...0xfc17]
#define HIMEM		0xfc4a	// (WORD) Address of the end of the available RAM area
#define JIFFY		0xfc9e	// (WORD) Internal time counter
#define CSRSW		0xfca9	// (BYTE) Cursor display switch (0:hidden | 1:visible)
#define CSTYLE		0xfcaa	// (BYTE) Cursor style (0:full cursor | other:halve cursor)
#define SCRMOD		0xfcaf	// (BYTE) Current BASIC screen mode number
#define EXPTBL		0xfcc1	// (BYTE) BIOS slot
#define H_TIMI		0xfd9a	// (WORD) Interrupt Hook
#define H_NMI		0xfdd6	// (...) At the beginning of non-maskable interrupts routine (Main-ROM at 0066h)
#define EXTBIO		0xffca	// (...) Extended BIOS call
#define RG8SAV		0xffe7	// (BYTE) Mirror Of VDP Register 8 (R#8)

// MSX-DOS system variables

#define TPALIM		0x0006	// (WORD) DOS TPA Limit, to calculate the max size of a program

// MSX mapped system variables

volatile __at (TPALIM) uint16_t varTPALIMIT;
volatile __at (CLIKSW) uint8_t  varCLIKSW;
volatile __at (LINL40) uint8_t  varLINL40;
volatile __at (CRTCNT) uint8_t  varCRTCNT;
volatile __at (SCRMOD) uint8_t  varSCRMOD;
volatile __at (REPCNT) uint8_t  varREPCNT;
volatile __at (PUTPNT) uint16_t varPUTPNT;
volatile __at (GETPNT) uint16_t varGETPNT;
volatile __at (MODE)   uint8_t  varMODE;
volatile __at (JIFFY)  uint16_t varJIFFY;
volatile __at (H_TIMI) uint16_t varHTIMI;
volatile __at (FORCLR) uint16_t varFORCLR;
volatile __at (BAKCLR) uint16_t varBAKCLR;
volatile __at (BDRCLR) uint16_t varBDRCLR;
volatile __at (NEWKEY+0) uint16_t varNEWKEY_row0;
volatile __at (NEWKEY+1) uint16_t varNEWKEY_row1;
volatile __at (NEWKEY+2) uint16_t varNEWKEY_row2;
volatile __at (NEWKEY+3) uint16_t varNEWKEY_row3;
volatile __at (NEWKEY+4) uint16_t varNEWKEY_row4;
volatile __at (NEWKEY+5) uint16_t varNEWKEY_row5;
volatile __at (NEWKEY+6) struct {
	unsigned shift: 1;
	unsigned ctrl:  1;
	unsigned graph: 1;
	unsigned capsl: 1;	// Caps Lock
	unsigned code:  1;	// Code/Kana/Hangul/Rus
	unsigned f1: 1;
	unsigned f2: 1;
	unsigned f3: 1;
} varNEWKEY_row6;
volatile __at (NEWKEY+7) struct {
	unsigned f4: 1;
	unsigned f5: 1;
	unsigned esc:  1;
	unsigned tab:  1;
	unsigned stop: 1;
	unsigned bs:   1;
	unsigned select: 1;
	unsigned enter:  1;
} varNEWKEY_row7;
volatile __at (NEWKEY+8) struct {
	unsigned space: 1;
	unsigned home:  1;
	unsigned ins:   1;
	unsigned del:   1;
	unsigned curLeft:  1;
	unsigned curUp:    1;
	unsigned curDown:  1;
	unsigned curRight: 1;
} varNEWKEY_row8;
volatile __at (NEWKEY+9) struct {
	unsigned numAst: 1;	// Asterisk
	unsigned numAdd: 1;	// Addition
	unsigned numDiv: 1;	// Division
	unsigned num0: 1;
	unsigned num1: 1;
	unsigned num2: 1;
	unsigned num3: 1;
	unsigned num4: 1;
} varNEWKEY_row9;
volatile __at (NEWKEY+10) struct {
	unsigned num5: 1;
	unsigned num6: 1;
	unsigned num7: 1;
	unsigned num8: 1;
	unsigned num9: 1;
	unsigned numSub: 1;	// Substraction
	unsigned numCom: 1;	// Comma
	unsigned numDot: 1;	// Dot
} varNEWKEY_row10;
volatile __at (NEWKEY+11) struct {
	unsigned unused0: 1;
	unsigned execute: 1;	// extra key on a few machines
	unsigned unused2: 1;
	unsigned cancel:  1;	// extra key on a few machines
	unsigned unused4: 1;
	unsigned unused5: 1;
	unsigned unused6: 1;
	unsigned unused7: 1;
} varNEWKEY_row11;


// ========================================================
// MSX character codes and compatible sentences VT-52
// https://www.msx.org/wiki/MSX_Characters_and_Control_Codes
//
#ifndef __VT_KEY_CODES__
#define __VT_KEY_CODES__
#define  VT_BEEP		"\x07"		// A beep sound
#define  VT_UP			"\x1E"		//27,"A"	; Cursor up
#define  VT_DOWN		"\x1F"		//27,"B"	; Cursor down
#define  VT_RIGHT		"\x1C"		//27,"C"	; Cursor right
#define  VT_LEFT		"\x1D"		//27,"D"	; Cursor left
#define  VT_CLRSCR		"\x0C"		//27,"E"	; Clear screen:	Clears the screen and moves the cursor to home
#define  VT_HOME		"\x0B"		//27,"H"	; Cursor home:	Move cursor to the upper left corner.
#define  VT_CLREOS		"\x1B""J"	// Clear to end of screen:	Clear screen from cursor onwards.
#define  VT_CLREOL		"\x1B""K"	// Clear to end of line:	Deletes the line to the right of the cursor.
#define  VT_INSLINE		"\x1B""L"	// Insert line:	Insert a line.
#define  VT_DELLINE		"\x1B""M"	// Delete line:	Remove line.
#define  VT_CLLINE		"\x1B""l"	// Deletes the full line at the cursor
#define  VT_GOTOXY		"\x1B""Y"	// Set cursor position [rc]:	Move cursor to position [r+32]ow,[c+32]olumn encoded as single characters.
#define  VT_CURFULL		"\x1B""x4"	// Cursor to full size
#define  VT_CURHIDE		"\x1B""x5"	// Removes the cursor
#define  VT_CURHALF		"\x1B""y4"	// Cursor to half size
#define  VT_CURSHOW		"\x1B""y5"	// Shows the cursor

#define  KEY_BS			 8		// CTRL+H - BS key (back space). Moves cursos to the left
#define  KEY_TAB		 9		// CTRL+I - TAB key (tabulation)
#define  KEY_HOME		11		// CTRL+K - HOME key. Places the cursor at top left
#define  KEY_RETURN		13		// CTRL+M - RETURN key
#define  KEY_ENTER		13		//    "        "
#define  KEY_INSERT		18		// CTRL+R - INSERT key
#define  KEY_SELECT		24		// CTRL+X - SELECT key
#define  KEY_ESC		27		// ESC key
#define  KEY_RIGHT		28		// RIGHT cursor key
#define  KEY_LEFT		29		// LEFT cursor key
#define  KEY_UP			30		// UP cursor key
#define  KEY_DOWN		31		// DOWN cursor key
#define  KEY_SPACE		32		// SPACE key
#define  KEY_DELETE		127		// DELETE key
#endif	//__VT_KEY_CODES__
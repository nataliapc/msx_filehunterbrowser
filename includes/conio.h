/**
 * ============================================================================
 *  CONIO Library port for MSX2 (SCREEN 0) [2024-08-31]
 *  Borland (mostly) compatibility
 *  by NataliaPC
 *  https://web.archive.org/web/20090225093706/http://c.conclase.net/Borland/funcion.php?fun=cgets
 *  https://conclase.net/c/borland/conio
 * ============================================================================
 */
#pragma once
#ifndef  __CONIO_H__
#define  __CONIO_H__

#include <stdint.h>
#include <stdbool.h>



// =====================================================================
// Constants, Enums & Structs

/**
 * Constants for {@link setcursortype}
 */
#define NOCURSOR		0
#define NORMALCURSOR	1
#define SOLIDCURSOR		2

/**
 * Constants for {@link gettextinfo}
 */
enum TextModes {
	LASTMODE = -1,	// Selects the previous mode
	BW40 = 0,		// Black and white 40 columns
	BW80 = 2,		// Black and white 80 columns
	_ORIGMODE = 65	// Original mode at the beginning of the program
};

/**
 * Color constants
 */
enum Color {
	TRANSPARENT = 0,
	BLACK,
	MEDIUM_GREEN,
	LIGHT_GREEN,
	DARK_BLUE,
	LIGHT_BLUE,
	DARK_RED,
	CYAN, 
	MEDIUM_RED,
	LIGHT_RED,
	DARK_YELLOW,
	LIGHT_YELLOW,
	DARK_GREEN,
	MAGENTA, 
	GRAY,
	WHITE,
};

/**
 * Struct for {@link gettextinfo}
 */
typedef struct {
	uint8_t winleft;		// [0] Left coordinate of the window
	uint8_t wintop;			// [1] Top coordinate of the window
	uint8_t winright;		// [2] Right coordinate of the window
	uint8_t winbottom;		// [3] Bottom coordinate of the window
	uint16_t attribute;		// [4] Current text attributes. See textattr()
	uint16_t normattr;		// [6] Attributes before textmode() was called.
	uint8_t currmode;		// [8] Mode in use: BW40, BW80, C40, C80, or C4350
	uint8_t screenheight;	// [9] Height of the text screen
	uint8_t screenwidth;	// [10] Width of the text screen
	uint8_t curx;			// [11] X coordinate of the current window
	uint8_t cury;			// [12] Y coordinate of the current window
	//Only MSX
	bool     vramBlink;		// [13] True for text blink enabled
	uint16_t vramCharMap;	// [14] Address at VRAM for chars(tiles) map
	uint16_t vramCharPat;	// [16] Address at VRAM for chars(tiles) patterns
} text_info;

/**
 * Characters to create graphical frames (frameChars array)
 */
#define FRAME_UP_LEFT		0
#define FRAME_UP_RIGHT		1
#define FRAME_DOWN_LEFT		2
#define FRAME_DOWN_RIGHT	3
#define FRAME_HORIZ_UP		4
#define FRAME_HORIZ_DOWN	5
#define FRAME_VERT_LEFT		6
#define FRAME_VERT_RIGHT	7
#define FRAME_HORIZONTAL	8
#define FRAME_VERTICAL		9
#define FRAME_FILL			10

/**
 * MSX character codes and compatible sentences VT-52
 * https://www.msx.org/wiki/MSX_Characters_and_Control_Codes
 */
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

#define  KEY_TAB		 9		// CTRL+I - TAB key (tabulation)
#define  KEY_HOME		11		// CTRL+K - HOME key. Places the cursor at top left
#define  KEY_ENTER		13		// CTRL+M - RETURN key
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


// ANCHOR =====================================================================
// Screen functions

/**
 * gettextinfo
 * Gets the video information in text mode. This information is stored
 * in a structure pointed to by the *ti argument. The text_info structure
 * is defined according to {@link text_info}
 */
void gettextinfo(text_info *ti);

/**
 * textmode
 * This function selects a text mode specified by the mode argument.
 * This argument can be a symbolic constant of the text_modes enumeration type
 * (in conio.h).
 * 
 * When the textmode function is called, the current window is reset to
 * full screen, and the text attributes in use are reset to normal,
 * corresponding to a call to normvideo. Specifying LASTMODE to textmode
 * causes the most recently selected text mode to be selected again.
 * 
 * The textmode function should only be used when the window or screen is
 * in text mode (supposedly to change to a different text mode).
 * This is the only context where the textmode function is used. When the
 * screen is in graphics mode, use the restorecrtmode function instead of
 * temporarily exiting to text mode.
 * 
 * There are several symbolic constants to indicate text modes.
 */
void textmode(int8_t mode) __z88dk_fastcall;

/**
 * setcursortype
 * Selects the cursor appearance among three types. The cursor_type argument
 * indicates the type of cursor to select according to these:
 *     NOCURSOR     Disables the cursor
 *     NORMALCURSOR Normal cursor: the underscore character
 *     SOLIDCURSOR  Cursor is a filled square
 */
void setcursortype(uint8_t cursor_type);

/**
 * TODO highvideo
 * Selects characters with higher intensity by activating the
 * high intensity bit of the current foreground color. The highvideo function
 * does not affect any of the characters currently on screen,
 * but it does affect those displayed by functions that use video
 * directly for text output after calling the highvideo function.
 *
 * Character attribute (Blink)     0800-08EF (090D)
 */
//void highvideo();

/**
 * TODO lowvideo
 * Selects characters with lower intensity by activating the
 * low intensity bit of the current foreground color. The lowvideo function
 * does not affect any of the characters currently on screen,
 * but it does affect those displayed by functions that use video
 * directly for text output after calling the lowvideo function.
 */
//void lowvideo();

/**
 * TODO normvideo
 * Selects characters with normal intensity by setting
 * the text attribute (foreground and background) to the value it had
 * previously at the beginning of the program. The normvideo function
 * does not affect any of the characters currently on screen,
 * but it does affect those displayed by functions that use video
 * directly for text output after calling the normvideo function.
 */
//void normvideo();

/**
 * textcolor
 * This function selects the text color specified by the color argument.
 * This function only works with functions that send output data in text mode
 * directly to the screen. The color argument is an integer between 0 and 15;
 * symbolic constants defined in conio.h can also be used instead of integers.
 * The textcolor function does not affect any of the characters currently on screen,
 * but it does affect those displayed by functions that use video directly for
 * text output after calling the textcolor function.
 *
 * There are several symbolic color constants to use.
 */
void textcolor(uint8_t color) __z88dk_fastcall;

/**
 * textbackground
 * This function selects the background color specified by the color argument.
 * This function only works with functions that send output data in text mode
 * directly to the screen. The color argument is an integer between 0 and 15;
 * symbolic constants defined in conio.h can also be used instead of integers.
 * The textattr function does not affect any of the characters currently on screen,
 * but it does affect those displayed by functions that use video directly for
 * text output after calling the textattr function.
 * 
 * There are several symbolic color constants to use.
 */
void textbackground(uint8_t color) __z88dk_fastcall;

/**
 * textblink
 * Enables or disables text blinking in a given region.
 */
void textblink(uint8_t x, uint8_t y, uint16_t length, bool enabled);

/**
 * This function clears fullscreen text blinking.
 */
void clrblink();

/**
 * textattr
 * This function assigns foreground and background colors in a single call.
 * (Normally, these attributes are assigned using the textcolor and
 * textbackground functions). The textattr function does not affect any
 * of the characters currently on screen, but it does affect those
 * displayed by functions that use video directly for text output
 * after calling the textattr function.
 * 
 * The color information is encoded in the attribute argument according to
 * this diagram:
 * 
 * Standard
 *   Bits	
 *     7 6 5 4 3 2 1 0
 *     B f f f p p p p
 *   In the 8-bit attribute argument:
 *     pppp is the 4-bit foreground color (0-15).
 *     fff is the 3-bit background color (0-7).
 *     B is the blink enable bit.
 *
 * MSX
 *   16-bit Attribute:
 *     F E D C B A 9 8 - 7 6 5 4 3 2 1 0
 *     F F F F B B B B - f f f f b b b b
 *   Values:
 *     ffff foreground color (0-15)
 *     bbbb background color (0-15)
 *     FFFF foreground blink color (0-15)
 *     BBBB background blink color (0-15)
 *
 * If FFFF and BBBB are 0 the blink mode is disabled.
 * 
 * PC
 * If using the symbolic constants defined in conio.h to create text
 * attributes using textattr, keep in mind the following limitations
 * for the selected background color:
 * 
 * Only one of the first eight colors can be chosen for the background.
 * You must shift the selected background color 4 bits to the left so that
 * they are placed in the correct bit positions.
 * There are several symbolic color constants to use.
 */
void textattr(uint16_t attribute) __z88dk_fastcall;

// ANCHOR =====================================================================
// Screen positions & areas

/**
 * window
 * Defines a text window on the screen specified by the left and top arguments,
 * which describe the upper left corner, and by the right and bottom arguments,
 * which describe the lower right corner. The minimum size of the text window
 * is one column by one row. The default window is the full screen with the
 * upper left corner being (1,1) and the lower right being (C,R); where C is
 * less than the number of columns and R less than the number of rows according
 * to the text mode in use. The call to the window function will be ignored if
 * any of the arguments are invalid.
 */
void window(int left, int top, int right, int bottom);

/**
 * gotoxy
 * Moves the cursor of the text window to the position specified by the
 * coordinates x and y. If the coordinates are not valid, then the call to
 * the gotoxy function is ignored.
 */
void gotoxy(uint8_t x, uint8_t y);

/**
 * wherex
 * Gets the x coordinate of the current cursor position (within the
 * current text window).
 * 
 * @return	The wherex function returns an integer between 1 and the number
 * 			of columns in the current text mode.
 */
uint8_t wherex() __sdcccall(1);

/**
 * wherey
 * Gets the y coordinate of the current cursor position (within the
 * current text window).
 * 
 * @return	The wherey function returns an integer between 1 and the number
 * 			of rows in the current text mode.
 */
uint8_t wherey() __sdcccall(1);

/**
 * gettext
 * Saves the content in a rectangle of text on the screen defined by the
 * left and top arguments, which describe the upper left corner,
 * and by the right and bottom arguments, which describe the lower right
 * corner, in the memory area pointed to by the destination argument. All
 * coordinates are absolute screen coordinates; they are not relative to
 * the window. The upper left corner is (1,1). The gettext function reads
 * the content in this rectangle into memory sequentially from left to
 * right and from top to bottom. Each screen position requires 2 bytes of
 * memory. The first byte is the character in the cell and the second is
 * the video attribute of the cell. The space required for a rectangle w
 * columns wide and h rows high is defined as:
 *
 * bytes = (h rows) x (w columns) x 2.
 * 
 * @return	Returns 1 if the operation is successful. If there is an error,
 *			such as accessing outside the screen, it returns a value of 0.
 */
uint8_t gettext(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom, void *target);

/**
 * puttext
 * Prints the content in a text rectangle on the screen defined by the
 * arguments left and top, which describe the upper left corner,
 * and by the arguments right and bottom, which describe the lower
 * right corner, in the memory area pointed to by the source argument.
 * All coordinates are absolute screen coordinates; they are not
 * relative to the window. The upper left corner is (1,1). The puttext
 * function places the content of this rectangle in memory sequentially
 * from left to right and top to bottom. The space required for a
 * rectangle b columns wide and h rows high is defined as:
 *
 * bytes = (h rows) x (w width)
 *
 * The puttext function uses direct video output.
 *
 * @return	The puttext function returns a non-zero value if the operation
 * 			is successful. If an error occurs, such as accessing outside
 * 			the screen, it returns a value of 0.
 */
uint8_t puttext(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom, void *source);

/**
 * movetext
 * Copies the content in a text rectangle on the screen defined by the
 * arguments left and top, which describe the upper left corner,
 * and by the arguments right and bottom, which describe the lower
 * right corner, to another rectangle of equal dimensions. The upper left
 * corner of the new rectangle is specified by the arguments
 * dest_left and dest_top. All coordinates are absolute screen
 * coordinates; they are not relative to the window. Rectangles that
 * occupy the same area are moved accordingly. The movetext function
 * uses direct video output.
 * 
 * @return	The movetext function returns a non-zero value if the operation
 * 			is successful. If an error occurs, such as accessing outside
 * 			the screen, it returns a value of 0.
 */
uint8_t movetext(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom, uint8_t dst_left, uint8_t dst_top);

/**
 * putlinexy
 * Prints the content of a buffer directly on the screen at the
 * indicated position.
 */
void putlinexy(uint8_t x, uint8_t y, uint16_t length, void *source);

/**
 * chline
 * The function outputs a horizontal line with the given length starting at the 
 * current cursor position.
 * The character used to draw the horizontal line is system dependent. If available, 
 * a line drawing character is used. Drawing a line that is partially off screen 
 * leads to undefined behaviour.
 */
void chline(uint8_t length);

/**
 * chlinexy
 * The function outputs a horizontal line with the given length starting at a given 
 * position.
 * The character used to draw the horizontal line is system dependent. If available, 
 * a line drawing character is used. Drawing a line that is partially off screen 
 * leads to undefined behaviour.
 */
void chlinexy(unsigned char x, unsigned char y, unsigned char length);

/**
 * cvline
 * The function outputs a vertical line with the given length starting at the 
 * current cursor position.
 * The character used to draw the vertical line is system dependent. If available, 
 * a line drawing character is used. Drawing a line that is partially off screen 
 * leads to undefined behaviour.
 */
void cvline(uint8_t length);

/**
 * chlinexy
 * The function outputs a vertical line with the given length starting at a given 
 * position.
 * The character used to draw the vertical line is system dependent. If available, 
 * a line drawing character is used. Drawing a line that is partially off screen 
 * leads to undefined behaviour.
 */
void cvlinexy(unsigned char x, unsigned char y, unsigned char length);

/**
 * drawFrame
 * The function draw a rectangled frame at a given position.
 * The characters used to draw the frame is system dependent. Drawing a frame that 
 * is partially off screen leads to undefined behaviour.
 */
void drawFrame(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom);


// ANCHOR =====================================================================
// Output to console functions

/**
 * putch
 * Displays a character, specified by the argument c, directly to the
 * text window in use. The putch function uses video directly to
 * display characters. This is done by direct writing to the
 * screen memory or through a BIOS call, depending on the
 * value of the global variable _directvideo. This function does not convert
 * newline characters (\n) to the carriage return/line feed pair (\r\n).
 *
 * @return	Returns the displayed character if successful; if an error occurs,
 * 			it returns EOF.
 */
uint8_t putch(uint8_t c) __sdcccall(1);
int putchar(int c) __sdcccall(1);

/**
 * cprintf
 * Displays text on the screen according to the described format. This function is
 * similar to the printf function, but with the exception that the
 * cprintf function will not convert newline characters (\n) to the
 * carriage return/line feed pair (\r\n). Tab characters (\t)
 * will not be expanded to spaces. The formatted text string will be sent
 * directly to the current text window on the screen.
 *
 * This is done by direct writing to the screen memory.
 *
 * @return	Returns the number of characters written.
 */
int cprintf(const char *format, ...);

/**
 * csprintf
 * Like sprintf but small.
 */
int csprintf(char *str, const char *format, ...);

/**
 * cputs
 * Displays the null-terminated string pointed to by the
 * *string argument in the current text window. This function is similar to
 * the puts function, but with two exceptions: the cputs function will not add
 * the newline character at the end of the text.
 * 
 * This is done by direct writing to the screen memory.
 */
void cputs(const char *string);

/**
 * clrscr
 * This function clears the current text window and places the cursor in the
 * upper left corner: position (1,1).
 *
 * Initial screen position in original conio library is (1,1).
 */
void clrscr();

/**
 * clreol
 * This function clears all characters from the cursor position
 * to the end of the line within the current text window, without moving
 * the cursor position.
 */
void clreol();

/**
 * insline
 * Inserts an empty line in the text window at the cursor position
 * using the current background text color. All lines below the
 * empty one are moved down one line, and the bottom line is moved out
 * of the window.
 */
void insline();

/**
 * delline
 * Deletes the line where the cursor is located and moves all lower lines
 * up one line. The delline function works in the active text window.
 */
void delline();


// ANCHOR =====================================================================
// Input from keyboard functions

/**
 * kbhit
 * Checks if a key press is available. Any available key presses
 * can be retrieved with the getch or getche functions.
 * 
 * @return	The kbhit function returns 0 if no key press has been registered;
 * 			if one is available, then the returned value is non-zero.
 */
bool kbhit();

/**
 * getch
 * Reads a single character directly from the keyboard, without displaying
 * the character on the screen.
 * 
 * @return	Returns the character read from the keyboard.
 */
int getch() __z88dk_fastcall;

/**
 * getche
 * Reads a single character directly from the keyboard, displaying the character
 * on the screen, directly to video.
 *
 * @return	The getche function returns the character read from the keyboard.
 */
int getche() __z88dk_fastcall;

/**
 * TODO ungetch
 * Pushes the character specified by the argument c back to the console,
 * forcing the pushed character, c, to be the next character read. The
 * ungetch function does not work if it is called more than once before
 * the next read.
 * 
 * @return	The ungetch function returns the pushed character if successful;
 * 			if not, it returns EOF.
 */
//int ungetch(int c) __z88dk_fastcall;

/**
 * TODO cgets
 * This function will read a string of characters from the console, storing the
 * string (and its length) in the location pointed to by *string. The cgets
 * function will read characters until it finds a carriage return and line feed
 * (CR/LF) combination, or until the maximum number of allowed characters
 * have been read. If a CR/LF combination is read, it is replaced by a
 * null character '\0' before the string is stored.
 * 
 * Before the cgets function is called, assign to string[0] the maximum
 * length of the string to be read. Upon return, string[1] is assigned the
 * number of characters read. The characters read start from string[2]
 * (inclusive) and end with a null character. For this reason, *string
 * must be at least string[0] plus 2 bytes in length.
 * 
 * @return	Returns the string of characters starting from string[2], if
 * 			successful.
 */
//char *cgets(char *string) __z88dk_fastcall;

/**
 * TODO getpass
 * Reads a password from the system console after displaying a message,
 * which is a string of characters (terminated by a null character) pointed
 * to by the message argument and disabling text output.
 * 
 * @return	Returns a static pointer to the null-terminated string of characters
 * 			containing the password. This string contains up to eight characters,
 * 			not counting the null character. Each time the getpass function is
 * 			called, the character string is overwritten.
 */
//char *getpass(const char *message) __z88dk_fastcall;

/**
 * TODO cscanf
 * Collects text and processes it according to the format given by *format.
 * This function is similar to the scanf function, the difference being that
 * the cscanf function reads data from the console that is automatically
 * displayed.
 * 
 * @return	Returns the number of input elements that have been scanned,
 * 			converted, and successfully stored; the returned value does not
 * 			include elements that have not been stored. If no elements have
 * 			been read, the return value is 0. If cscanf attempts to read at
 * 			end-of-file, the returned value is EOF.
 */
//int cscanf(const char *format, ...);


// ANCHOR =====================================================================
// I/O functions

/**
 * TODO inport
 * Reads 1 byte from the low part of 1 word from the input port indicated
 * by the id_port argument; reads the high byte from id_port+1. The inport
 * function works in the same way as the 80x86 IN instruction.
 * 
 * @return	Returns the value read of a word size from the port pointed to
 *			by the id_port and id_port+1 arguments.
 */
//int inport(int id_port) __z88dk_fastcall;

/**
 * TODO outport
 * Writes the last byte of 1 word to the output port indicated by the id_port
 * argument; writes the first byte to id_port+1. The outport function works
 * in the same way as the 80x86 OUT instruction.
 * 
 * @return	Returns the value written of a word size to the port pointed to
 * 			by the id_port and id_port+1 arguments.
 */
//int outport(int id_port, int value);

/**
 * inportb
 * Reads 1 byte from the port indicated by the id_port argument.
 */
uint8_t inportb(uint8_t port) __sdcccall(1);

/**
 * outportb
 * Writes 1 byte to the output port indicated by the id_port argument.
 */
void outportb(uint8_t port, uint8_t value) __sdcccall(1);



#endif //__CONIO_H__

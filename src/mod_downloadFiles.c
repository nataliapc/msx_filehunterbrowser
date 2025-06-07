/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <string.h>
#include <stdbool.h>
#include "msx_const.h"
#include "structs.h"
#include "conio.h"
#include "dos.h"
#include "heap.h"
#include "utils.h"
#include "fh.h"
#include "hgetlib.h"
#include "mod_downloadFiles.h"


// ========================================================
static FILEH fh;
static uint32_t downloadSize;
static uint8_t downloadFileStatus;
static uint32_t downloadedBytes;
static bool firstChunk;

// ========================================================
inline void printEnterFilename(ListItem_t *item)
{
	ASM_EI; ASM_HALT;
	_fillVRAM(0+(DOWNLOAD_POSY-1)*80, DOWNLOAD_HEIGHT*80, ' ');
	fillBlink(1,DOWNLOAD_POSY, DOWNLOAD_HEIGHT,80, true);

	// Prepare filename with elipsis if too long
	msx2_copyFromVRAM((uint32_t)item->name, (uint16_t)heap_top, 80);
	heap_top[80] = 0;
	if (strlen(heap_top) >= 63) {
		strcpy(heap_top+63, "...");
	}

	// Print download message
	csprintf(buff, "File: \"%s\"", heap_top);
	putstrxy(4, DOWNLOAD_POSY+1, buff);
	csprintf(buff, "Size: %u KB", item->size);
	putstrxy(4, DOWNLOAD_POSY+2, buff);
	csprintf(buff, "Filename to save: [        ]%s  ESC to cancel", currentPanel->type->extension);
	putstrxy(4, DOWNLOAD_POSY+4, buff);
}

inline void clearDownloadMessage()
{
	fillBlink(1,DOWNLOAD_POSY, DOWNLOAD_HEIGHT,80, false);
}

inline void printDownloadMessage()
{
	_fillVRAM(0+(DOWNLOAD_POSY-1)*80, DOWNLOAD_HEIGHT*80, ' ');
}

void clearStatusLine()
{
	_fillVRAM(0+(DOWNLOAD_POSY+3)*80, 80, ' ');
}

inline bool isValidFilename(char *filename)
{
	// Check if filename is not empty and is in n+3 format where n > 0 and n <=8. No spaces allowed

	if (*filename == 0 || *filename == '.' || strchr(filename, ' ')) return false;

	char *dot = strchr(filename, '.');
	uint8_t len = strlen(filename);

	if (!dot && len > 8) return false;
	if (&filename[len] - ++dot > 3 || strchr(&filename[len], '.')) return false;

	return true;
}

// ========================================================
static void FileSizeUpdate(long contentSize)
{
	downloadSize = contentSize;
}

static void FileWriteCallback(char *rcv_buffer, int bytes_read)
{
	if (bytes_read) {
		char *ptr = rcv_buffer;
		if (firstChunk) {
			firstChunk = false;
			ptr = strchr(rcv_buffer, '\n') + 1;
			bytes_read -= (ptr - rcv_buffer);
		}
		dos2_fwrite(ptr, bytes_read, fh);
	}

	downloadedBytes += bytes_read;
	csprintf(heap_top, "%lu%%", downloadedBytes * 100L / downloadSize);
	putstrxy(38,DOWNLOAD_POSY+4, heap_top);
}

void downloadFileToDisk(ListItem_t *item)
{
	formatURL(buff, item-list_start);

	net_waitConnected(60*10);		// Wait for connection (10 seconds on NTSC, 12 on PAL)

	if (hget(
		buff,						// URL
		(int)HTTPStatusUpdate,		// progress_callback
		(int)FileWriteCallback,		// data_write_callback
		(int)FileSizeUpdate,		// content_size_callback
		false						// enableKeepAlive
	) != ERR_TCPIPUNAPI_OK)
	{
		if (downloadFileStatus == DOWNLOAD_OK)
			downloadFileStatus = DOWNLOAD_FILE_ERROR;
	}
}

// ========================================================
void downloadFile()
{
	ListItem_t *item = getCurrentItem();
	bool end;
	char *filename = malloc(8+1+3+1);

	ASM_EI; ASM_HALT;
	setSelectedLine(false);

	do {
		printEnterFilename(item);
		downloadFileStatus = DOWNLOAD_OK;
		downloadedBytes = 0L;

		do {
			putstrxy(23,DOWNLOAD_POSY+4, "        ");
			gotoxy(23, DOWNLOAD_POSY+4);
			scanf(filename, 8);
			if (!filename[0]) break;
			strcat(filename, currentPanel->type->extension);
			if (isValidFilename(filename)) break;
			putchar('\x07');
		} while (true);

		if (filename[0]) {
			// Print download message
			clearStatusLine();
			dos2_strupr(filename);
			csprintf(buff, "Downloading file \"%s\":", filename);
			putstrxy(4, DOWNLOAD_POSY+4, buff);

			// Create file on disk
			fh = dos2_fcreate(filename, O_WRONLY, ATTR_ARCHIVE);
			if (fh < ERR_FIRST) {
				firstChunk = true;
				downloadFileToDisk(item);
				printActivityLed(true);
				dos2_fclose(fh);
			} else {
				switch (fh) {
					case ERR_FILEX:	// File already exists
					case ERR_DIRX:	// Directory already exists
					case ERR_SYSX:	// System file exists
					case ERR_FILRO:	// Read-only file exists
					case ERR_FOPEN:	// File already in use
						downloadFileStatus = DOWNLOAD_FILE_EXISTS;
						break;
					case ERR_DRFUL:	// Root directory full
						downloadFileStatus = DOWNLOAD_ROOT_FULL;
						break;
					case ERR_DKFUL:	// Disk full
						downloadFileStatus = DOWNLOAD_DISK_FULL;
						break;
					default:		// Generic error
						downloadFileStatus = DOWNLOAD_FILE_ERROR;
						break;
				}
			}
		}
		end = true;

		if (downloadFileStatus != DOWNLOAD_OK) {
			clearStatusLine();
			csprintf(buff, "Error downloading file \"%s\": %s", filename, downloadMessage[downloadFileStatus]);
			putstrxy(4, DOWNLOAD_POSY+4, buff);
			// Wait for a pressed key
			waitKey();
			end = false;
		}
	} while (!end);

	ASM_EI; ASM_HALT;
	clearDownloadMessage();
	printList();
	setSelectedLine(true);

	free(8+1+3+1);
}

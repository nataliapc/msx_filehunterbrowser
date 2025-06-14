/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma codeseg DISPOSABLE

#include <stdint.h>
#include <string.h>
#include "msx_const.h"
#include "structs.h"
#include "heap.h"
#include "conio.h"
#include "dos.h"
#include "utils.h"
#include "fh.h"
#include "mod_commandLine.h"


// ========================================================
extern const unsigned char out_help_cmd_bin_zx0[];


// ========================================================
inline void printHelp()
{
	// Print help message
	cputs("## File-Hunter Browser v"VERSIONAPP"\n"
		  "## by "AUTHORAPP"\n");
	dzx0_standard(out_help_cmd_bin_zx0, heap_top);
	cputs(heap_top);
	dos2_exit(1);
}

void checkArguments(char **argv, int argc)
{
	uint8_t i, j;
	char cmd;

	// Si no hay argumentos o el primer argumento no empieza por '/', regresa
	if (!argc) return;

	// Procesar argumentos
	for (i = 0; i < argc; ++i) {
		if (argv[i][0] != '/') goto end;
		
		cmd = dos2_toupper(argv[i][1]);

		if (++i >= argc) goto end;

		// MSX generation
		if (cmd == 'M') {
			for (j=1; j<=REQMSX_MSXTR && strcmp(argv[i],reqMSX[j].value); ++j);
			if (j > REQMSX_MSXTR) goto end;
			request.msx = &reqMSX[j];
		} else
		// Panel selection
		if (cmd == 'P') {
			for (j=0; j<=PANEL_LAST && argv[i][0]!=panels[j].key; ++j);
			if (j > PANEL_LAST) goto end;
			currentPanel = &panels[j];
		} else
		// Search parameter
		if (cmd == 'S') {
			if (strlen(argv[i]) > SEARCH_MAX_SIZE) {
				argv[i][SEARCH_MAX_SIZE] = '\0';
			}
			strcpy(request.search.value, argv[i]);
		} else {
			goto end;
		}
	}
	return;

end:
	printHelp();
}


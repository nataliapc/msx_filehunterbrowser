#include "msx_const.h"
#include "utils.h"
#include "conio.h"


void waitKey()
{
	// Wait for a pressed key
	while (!kbhit()) { ASM_EI; ASM_HALT; }
	while (kbhit()) getch();
}
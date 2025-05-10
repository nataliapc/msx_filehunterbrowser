#include <stdint.h>
#include "utils.h"
#include "msx_const.h"


uint8_t getRomByte(uint16_t address) __naked __sdcccall(1)
{
	address;
	__asm
		push ix				; HL = param address
		ld   a,(EXPTBL)		; ROM BIOS Primary and Secondary Slot (expanded or not)
		call RDSLT
		pop  ix
		reti				; Returns A = byte content
	__endasm;
}
/*
	Function: net_getDriverInfo
	Author: NataliaPC

	Description: Get UNAPI driver information.
	Parameters: info - Pointer to a struct to store the driver information.
	Returns: bool - Returns true if success, false otherwise.

	Based on: https://github.com/ducasp/MSX-Development/blob/master/UNAPI/WAITWIFI/waitwifi.c
*/
#include <stdint.h>
#include <stdbool.h>
#include "asm.h"
#include "enums.h"
#include "hgetlib.h"


extern Z80_registers reg;


bool net_getDriverInfo(void *codeBlock, UnapiDriverInfo_t *info)
{
	info->driverName[0] = '\0';		// Initialize name address to empty

	UnapiBuildCodeBlock(NULL, 1, codeBlock);
	UnapiCall(codeBlock, UNAPI_GET_INFO, &reg, REGS_NONE, REGS_MAIN);
	info->specVersionMain = reg.Bytes.D;
	info->specVersionSec = reg.Bytes.E;
	info->versionMain = reg.Bytes.B;
	info->versionSec = reg.Bytes.C;

	uint16_t nameAddress = reg.UWords.HL;
	if (!nameAddress || nameAddress < 0x4000 || nameAddress > 0x7FFF) {
		return false; // Invalid address
	}

	char *readChar = info->driverName;
	while(true) {
		*readChar = UnapiRead(codeBlock, nameAddress);
		if (!*readChar++) {
			break;
		}
		nameAddress++;
	}

	return true;
}
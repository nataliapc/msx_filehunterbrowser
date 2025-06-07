/*
	Function: net_isConnected
	Author: NataliaPC

	Description: Checks if the TCP/IP connection is still alive.
	Parameters: timeout - The timeout value for the connection check (not used in this implementation).
	Returns: bool - Returns true if the connection is alive, false otherwise.

	Based on: https://github.com/ducasp/MSX-Development/blob/master/UNAPI/WAITWIFI/waitwifi.c
*/
#include <stdint.h>
#include <stdbool.h>
#include "asm.h"
#include "enums.h"


extern unapi_code_block *codeBlock;
extern Z80_registers reg;

volatile __at (0xFC9E) uint16_t varJIFFY;


bool net_waitConnected(uint16_t timeout_ticks)
{
	// Check if the TCP/IP connection is still alive
	bool result;

	do {
		__asm
			ei
			halt
		__endasm;
		if (!timeout_ticks--) break;

		// Check Connection
		UnapiCall(codeBlock, TCPIP_NET_STATE, &reg, REGS_NONE, REGS_MAIN);
		result = (reg.Bytes.B == TCPIP_NET_STATE_OPEN);

	} while (!result);

	return result;
}
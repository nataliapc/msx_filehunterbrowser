/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma codeseg DISPOSABLE
#pragma constseg DISPOSABLE

#include <stdint.h>
#include <string.h>
#include "msx_const.h"
#include "heap.h"
#include "dos.h"
#include "utils.h"
#include "fh.h"
#include "hgetlib.h"


// ========================================================
// HGET/1.3 (MSX-DOS 2.31; MSX2; TCP/IP UNAPI; ESP8266 Wi-Fi UNAPI v1.2)

static const char *msxName[] = {
	"MSX/1",
	"MSX/2",
	"MSX/2+",
	"MSX/Turbo-R"
};
static const char *dosName[] = {
	"UnknownDOS",
	"MSX-DOS/1.x",
	"MSX-DOS/2.x",
	"Nextor"
};

// ========================================================
extern char *unapiBuffer;
extern const char *user_agent;
extern uint8_t HEAP_disposable;

extern uint8_t msxVersionROM;
extern uint8_t kanjiMode;
extern uint8_t originalLINL40;
extern uint8_t originalSCRMOD;
extern uint8_t originalFORCLR;
extern uint8_t originalBAKCLR;
extern uint8_t originalBDRCLR;


// ========================================================
void redefineFunctionKeys()
{
	char *fk = (char*)FNKSTR;
	memset(fk, 0, 160);
	for (uint8_t i='1'; i<='5'; i++,fk+=16) {
		*fk = i;
	}
}

static void formatUserAgent(RETB msxdosVersion)
{
	UnapiDriverInfo_t *info = (UnapiDriverInfo_t*)&HEAP_disposable;
	char *uagentTmp = ((char*)info) + sizeof(UnapiDriverInfo_t);

	// Get UNAPI driver info
	net_getDriverInfo((void*)unapiBuffer, info);

	// Format user agent
	csprintf(uagentTmp, "FHBrowser/"VERSIONAPP" (%s; %s; TCPIP UNAPI/%u.%u; %s/%u.%u)", 
		msxName[msxVersionROM],
		dosName[msxdosVersion],
		info->specVersionMain, info->specVersionSec,
		info->driverName, info->versionMain, info->versionSec);

	user_agent = (char*)malloc(strlen(uagentTmp) + 1);
	strcpy((char*)user_agent, uagentTmp);
}

// ========================================================
void checkPlatformSystem()
{
	// Check MSX2 ROM or higher
	msxVersionROM = getRomByte(MSXVER);
	if (!msxVersionROM) {
		die("MSX1 not supported!");
	}

	// Check MSX-DOS 2 or higher
	RETB msxdosVersion = dosVersion();
	if (msxdosVersion < VER_MSXDOS2x) {
		die("MSX-DOS 2.x or higher required!");
	}

	// Format the user agent
	formatUserAgent(msxdosVersion);

	// Check TCP/IP UNAPI
	char ret = hgetinit((uint16_t)unapiBuffer, user_agent);
	if (ret != ERR_TCPIPUNAPI_OK) {
#ifndef _DEBUG_
		if (ret == ERR_TCPIPUNAPI_NOT_TCPIP_CAPABLE) {
			die("TCP/IP UNAPI not fully capable!\x07\r\n");
		} else
		if (ret == ERR_HGET_INVALID_BUFFER) {
			die("Invalid buffer for TCP/IP UNAPI!\x07\r\n");
		}
		die("TCP/IP UNAPI not found!\x07\r\n");
#endif
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

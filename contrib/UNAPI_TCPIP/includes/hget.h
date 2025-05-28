/*
--
-- HGET.h
--   Header for HGET.c
--   Revision 0.4
--
--        Oduvaldo Pavan Junior 09/2020 v0.1 - 0.4
--
--   Based on HGET Unapi Utility that is a work from:
--        Konamiman 1/2011 v1.1
--        Oduvaldo Pavan Junior 07/2019 v1.3
--
*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hgetlib.h"
//These are available at www.konamiman.com
#include "asm.h"
#include "base64.h"


#ifndef HGET_H
#define HGET_H

#define _TERM0 0
#define _CONIN 1
#define _INNOE 8
#define _BUFIN 0x0A
#define _CONST 0x0B
#define _GDATE 0x2A
#define _GTIME 0x2C
#define _FFIRST 0x40
#define _OPEN 0x43
#define _CREATE 0x44
#define _CLOSE 0x45
#define _ENSURE 0x46
#define _SEEK 0x4A
#define _READ 0x48
#define _WRITE 0x49
#define _IOCTL 0x4B
#define _PARSE 0x5B
#define _TERM 0x62
#define _DEFAB 0x63
#define _DEFER 0x64
#define _EXPLAIN 0x66
#define _GENV 0x6B
#define _DOSVER 0x6F
#define _REDIR 0x70

#define _CTRLC 0x9E
#define _STOP 0x9F
#define _NOFIL 0x0D7
#define _EOF 0x0C7

#define TICKS_TO_WAIT (20*60)
#define SYSTIMER ((uint*)0xFC9E)

#define TCP_BUFFER_SIZE (1024)
#define TCPOUT_STEP_SIZE (512)

#define HTTP_DEFAULT_PORT (80)
#define HTTPS_DEFAULT_PORT (443)

#define TCPIP_CAPAB_VERIFY_CERTIFICATE 16
#define TCPFLAGS_USE_TLS 4
#define TCPFLAGS_VERIFY_CERTIFICATE 8

#define MAX_REDIRECTIONS 10

typedef void (*funcptr)(bool);
typedef void (*funcdataptr)(char *, int);
typedef void (*funcsizeptr)(long);

enum TcpipUnapiFunctions {
    UNAPI_GET_INFO = 0,
    TCPIP_GET_CAPAB = 1,
    TCPIP_NET_STATE = 3,
    TCPIP_DNS_Q = 6,
    TCPIP_DNS_S = 7,
    TCPIP_TCP_OPEN = 13,
    TCPIP_TCP_CLOSE = 14,
    TCPIP_TCP_ABORT = 15,
    TCPIP_TCP_STATE = 16,
    TCPIP_TCP_SEND = 17,
    TCPIP_TCP_RCV = 18,
    TCPIP_WAIT = 29
};

enum TcpipErrorCodes {
    ERR_OK = 0,
    ERR_NOT_IMP,
    ERR_NO_NETWORK,
    ERR_NO_DATA,
    ERR_INV_PARAM,
    ERR_QUERY_EXISTS,
    ERR_INV_IP,
    ERR_NO_DNS,
    ERR_DNS,
    ERR_NO_FREE_CONN,
    ERR_CONN_EXISTS,
    ERR_NO_CONN,
    ERR_CONN_STATE,
    ERR_BUFFER,
    ERR_LARGE_DGRAM,
    ERR_INV_OPER
};

/* Strings */
#define strDefaultFilename "index.htm"

/* Global Variables */
static const char *default_user_agent = "MSX_HGETLIB (MSX)";
static const char *user_agent;
static bool continue_using_keep_alive;
static byte conn;
static char* domainName;
static bool continueReceived;
static bool redirectionRequested;
static int remainingInputData;
static byte* inputDataPointer;
static byte emptyLineReaded;
static long contentLength,blockSize,currentBlock;
static bool isChunkedTransfer;
static long currentChunkSize;
static bool newLocationReceived;
static long receivedLength;
static byte* TcpInputData;
#define TcpOutputData TcpInputData
static byte remoteFilePath[256];
static byte headerLine[256];
static char statusLine[256];
static char redirectionFullLocation[256];
static int responseStatusCode;
static byte responseStatusCodeFirstDigit;
static char* headerTitle;
static char* headerContents;
static Z80_registers reg;
static unapi_code_block* codeBlock;
static int ticksWaited;
static int sysTimerHold;
static bool redirectionUrlIsNewDomainName;
static bool zeroContentLengthAnnounced;
static bool cancelled_by_handler;
typedef struct {
    byte remoteIP[4];
    uint remotePort;
    uint localPort;
    int userTimeout;
    byte flags;
	int hostName;
} t_TcpConnectionParameters;

static t_TcpConnectionParameters* TcpConnectionParameters;
#ifdef USE_TLS
static bool useHttps = false;
static bool mustCheckCertificate = true;
static bool mustCheckHostName = true;
static bool safeTlsIsSupported = true;
static bool TlsIsSupported = false;
static byte tcpIpSpecificationVersionMain;
static byte tcpIpSpecificationVersionSecondary;
#endif//USE_TLS
static bool tryKeepAlive = false;
static bool keepingConnectionAlive = false;
static byte redirectionRequests = 0;
static funcptr UpdateReceivedStatus;
static funcdataptr SaveReceivedData;
static funcsizeptr SendContentSize;
static bool thereisacallback = false;
static bool thereisasavecallback = false;
static bool thereisasizecallback = false;
static bool hasinitialized = false;
static bool indicateblockprogress = false;

/* Some handy defines */

#define LetTcpipBreathe() UnapiCall(codeBlock, TCPIP_WAIT, &reg, REGS_NONE, REGS_NONE)
#define SkipCharsWhile(pointer, ch) {while(*pointer == ch) pointer++;}
#define SkipCharsUntil(pointer, ch) {while(*pointer != ch) pointer++;}
#define SkipLF() GetInputByte(NULL)
#define ToLowerCase(ch) {ch |= 32;}
#define ResetTcpBuffer() {remainingInputData = 0; inputDataPointer = TcpInputData;}
#define AbortIfEscIsPressed() ((*((byte*)0xFBEC) & 4) == 0 || cancelled_by_handler)

/* Internal Function prototypes */
/* Functions Related to HTTP Handling */
static void TerminateConnection();
static HgetReturnCode_t ProcessUrl(char* url, bool isRedirection);
inline void ExtractPortNumberFromDomainName();
inline HgetReturnCode_t DoHttpWork();
inline HgetReturnCode_t SendHttpRequest();
static HgetReturnCode_t ReadResponseHeaders();
static HgetReturnCode_t SendLineToTcp(char* string);
inline HgetReturnCode_t CheckHeaderErrors();
inline HgetReturnCode_t DownloadHttpContents();
static HgetReturnCode_t SendCredentialsIfNecessary();
inline HgetReturnCode_t ReadResponseStatus();
inline HgetReturnCode_t ProcessResponseStatus();
static HgetReturnCode_t ReadNextHeader();
inline HgetReturnCode_t ProcessNextHeader();
inline void ExtractHeaderTitleAndContents();
static bool HeaderTitleIs(char* string);
static bool HeaderContentsIs(char* string);
inline HgetReturnCode_t DiscardBogusHttpContent();
inline HgetReturnCode_t DoDirectDatatransfer();
inline HgetReturnCode_t DoChunkedDataTransfer();
static long GetNextChunkSize();
/* Functions Related to Callbacks Handling  */
static void UpdateReceivingMessage();
static bool WriteContents(byte* dataPointer, int size);
/* Functions Related to Network I/O  */
inline bool InitializeTcpipUnapi();
inline bool CheckTcpipCapabilities();
static HgetReturnCode_t EnsureThereIsTcpDataAvailable();
inline bool EnsureTcpConnectionIsStillOpen();
inline HgetReturnCode_t ReadAsMuchTcpDataAsPossible();
static HgetReturnCode_t GetInputByte(byte *data);
inline bool CheckNetworkConnection();
static HgetReturnCode_t OpenTcpConnection();
static HgetReturnCode_t ResolveServerName();
static void CloseTcpConnection();
inline HgetReturnCode_t SendTcpData(byte* data, int dataSize);
/* Functions Related to Strings  */
static bool StringStartsWith(const char* stringToCheck, const char* startingToken);
static uint8_t strcmpi(const char *a1, const char *a2);
static uint8_t strncmpi(const char *a1, const char *a2, unsigned size);

#endif//HGET_H

/* HTTP getter Library 0.4
        Oduvaldo Pavan Junior 09/2020 v0.1 - 0.4

   Based on HGET Unapi Utility that is a work from:
        Konamiman 1/2011 v1.1
        Oduvaldo Pavan Junior 07/2019 v1.3

   HGET Library history:
   Version 0.4 -
   - Some Internet Service Providers have a heavy hand on open connections, so
     a keep-alive connection might be terminated by the ISP (not the server) if
     it is idle for a while after the last GET request was serviced. ISP's
     shouldn't be doing that, but hey, this is life, and we need to live with
     it, so if a new request comes and it fails and the connection was being
     kept alive, will just ignore keep alive and treat as a regular connection
   Version 0.3 -
   - re-organized code and changed the way code was calling Terminate a lot
   - changed a few loops for tick and sequence for TcpIpBreath, this boosts a
     little performance for OBSONET
   - as this lib do not support resuming failed transfers, added pre-allocation
     of the entire file prior to writing when file size is known. This gives
     quite a boost on performance with a few disk adapters

   Version 0.2 - making code cleaner and trying to keep Konamiman style, also
   adding basic support to keep-alive connections, also, agent is defined in
   hgetlib.h so each application can define it own by changing it.

   Version 0.1 - it is a simplification of HGET so it can be used as a library
   inside a project. It doesn't intend to have the exact same features, but has
   different objectives:

   - Allow to be called multiple times without searching UNAPI for every access
   - It can be compiled with SSL/TLS support if you define USE_TLS
   - It allows several means of handling the received data:
    * If no buffer, no data write callback and no filename is provided, try to
      use the same name from the access URL
    * If no buffer, no data write callback is provided, will use the filename /
      path to store the file
    * If data write callback is provided, will call it every time there is data
      received.
   - It allows registering an progress update callback:
    * If registered it is called every 4% of data is received, if size is known,
      parameter is false
    * If registered and size is unknown, it is called every time data is received,
      parameter is true
   - It allows registering a content size update callback that will receive the
     content length if available, or 0 if not available.

   History from HGET:
   Version 1.3 should be TCP-IP v1.1 compliant, that means, TLS support, so you
   can download files from https sites if your device is compliant.
   It also removes an extra tick wait after calling TCPIP_WAIT, as there seems
   to have no reason for it and it can lower the performance. Any needed WAIT
   should be already done by adapter UNAPI when calling TCPIP_WAIT.
   Also I've changed the download progress to a bar, it changes every 4%
   increment of file size of known file size or there is a moving character if
   file size is unknown. This is way easier on VDP / CALLs and allow better
   performance on fast adapters that can use the extra CPU time.
*/
#include "hget.h"

#define USE_CONIO_H
#ifdef USE_CONIO_H
    int csprintf(char *str, const char *format, ...);
    #define sprintf csprintf
#else
    #warning "Conio.h is not available, using stdio.h instead for csprintf"
    #include <stdio.h>
#endif


HgetReturnCode_t hgetinit(unsigned int addressforbuffer, const char* userAgent)
{
    if (!hasinitialized)
    {
        if (userAgent != NULL)
            user_agent = userAgent;
        else
            user_agent = default_user_agent;

        continue_using_keep_alive = true;
        conn = 0;
        hasinitialized = false;
        thereisacallback = false;
        thereisasavecallback = false;
        unsigned int bufferSize = sizeof(t_TcpConnectionParameters) + sizeof(unapi_code_block) + 0x200 + TCP_BUFFER_SIZE;
#ifdef USE_TLS
        TlsIsSupported = false;
        useHttps = false;
#endif
        tryKeepAlive = false;

        //InitializeBufferPointers - We need room for our data from the beginning of addressforbuffer
        //And that data MUST NOT be in page 1, that means, 0x4000 to 0x7FFF, as this is where UNAPI
        //device code resides and calls to UNAPI will switch that page
        if ((addressforbuffer>=0x8000)||((addressforbuffer+bufferSize)<0x4000)) {
            TcpConnectionParameters = (t_TcpConnectionParameters*)addressforbuffer;
            domainName = (char*)((unsigned int)TcpConnectionParameters + sizeof(t_TcpConnectionParameters));
            codeBlock = (unapi_code_block*)((unsigned int)domainName + 0x200);
            TcpInputData = (byte*)((unsigned int)codeBlock + sizeof(unapi_code_block));

            if (!InitializeTcpipUnapi())
                return ERR_TCPIPUNAPI_NOTFOUND;
            if (!CheckTcpipCapabilities())
                return ERR_TCPIPUNAPI_NOT_TCPIP_CAPABLE;

            hasinitialized = true;
            return ERR_TCPIPUNAPI_OK;
        } else
            return ERR_HGET_INVALID_BUFFER;
    }
    else
        return ERR_HGET_ALREADY_INITIALIZED;
}


void hgetfinish(void)
{
    if (hasinitialized) {
        thereisacallback = false;
        keepingConnectionAlive = false;
        tryKeepAlive = false;
        CloseTcpConnection();
    }
}


#ifdef USE_TLS
HgetReturnCode_t hget(char* url, int progress_callback, bool checkcertificateifssl, bool checkhostnameifssl, int data_write_callback, int content_size_callback, bool enableKeepAlive)
#else
HgetReturnCode_t hget(char* url, int progress_callback, int data_write_callback, int content_size_callback, bool enableKeepAlive)
#endif
{
    HgetReturnCode_t funcret;
    char* pointer;
#ifdef USE_TLS
	mustCheckCertificate = checkcertificateifssl;
	mustCheckHostName = checkhostnameifssl;
#endif
    cancelled_by_handler = false;
    receivedLength = 0;

    if (!hasinitialized)
        return ERR_HGET_NOT_INITIALIZED;

    if(!url)
        return ERR_HGET_INVALID_PARAMETERS;

    if (progress_callback) {
        UpdateReceivedStatus = (funcptr)progress_callback;
        thereisacallback = true;
    } else
        thereisacallback = false;

    if (data_write_callback) {
        SaveReceivedData = (funcdataptr)data_write_callback;
        thereisasavecallback = true;
    } else
        thereisasavecallback = false;

    if (content_size_callback) {
        SendContentSize = (funcsizeptr)content_size_callback;
        thereisasizecallback = true;
    } else
        thereisasizecallback = false;

    tryKeepAlive = enableKeepAlive & continue_using_keep_alive;

    //did the previous operation requested to keep connection alive?
    if (!keepingConnectionAlive) {
        *domainName = '\0';
        funcret = ProcessUrl(url, false);
    } else //connection is alive, so treat as redirection so it checks the previous domain name
        funcret = ProcessUrl(url, true);

    if (funcret != ERR_TCPIPUNAPI_OK)
        return funcret;

    if (!CheckNetworkConnection()) {
        if (keepingConnectionAlive)
        {
            tryKeepAlive = false;
            TerminateConnection();
        }
        return ERR_TCPIPUNAPI_NO_CONNECTION;
    }

    funcret = DoHttpWork();

    if (funcret != ERR_TCPIPUNAPI_OK)
        tryKeepAlive = false;

    TerminateConnection();

    return funcret;
}

void hgetcancel()
{
    cancelled_by_handler = true;
}


/****************************
 ***  FUNCTIONS are here  ***
 ****************************/
/* Functions Related to HTTP Handling */
void TerminateConnection()
{
    if (!tryKeepAlive) {
        CloseTcpConnection();
        keepingConnectionAlive = false;
    }
    else
        keepingConnectionAlive = true;
}


HgetReturnCode_t ProcessUrl(char* url, bool isRedirection)
{
    char* pointer;

    if(url[0] == '/') {
        if(isRedirection) {
            redirectionUrlIsNewDomainName = false;
        } else {
            return ERR_HGET_INVALID_PARAMETERS;
        }
    } else if(StringStartsWith(url, "http://")) {
		TcpConnectionParameters->remotePort = HTTP_DEFAULT_PORT;
		TcpConnectionParameters->flags = 0 ;
        if(isRedirection) {
#ifdef USE_TLS
			if (useHttps)
				redirectionUrlIsNewDomainName = true;
			else
#endif
			{
				pointer = strstr(url+7, "/");
				redirectionUrlIsNewDomainName = pointer && strncmpi(url+7, domainName, (pointer-url-7));
			}
        }
        strcpy(domainName, url + 7);
#ifdef USE_TLS
		useHttps = false;
#endif
    } else
#ifdef USE_TLS
    if((TlsIsSupported)&&(StringStartsWith(url, "https://"))) {
        if(isRedirection) {
			if (!useHttps)
				redirectionUrlIsNewDomainName = true;
			else
			{
				pointer = strstr(url+8, "/");
				redirectionUrlIsNewDomainName = pointer && strncmpi(url+8, domainName, (pointer-url-8));
			}
        }
        strcpy(domainName, url + 8);
		useHttps = true;
		TcpConnectionParameters->remotePort = HTTPS_DEFAULT_PORT;
		TcpConnectionParameters->flags = TcpConnectionParameters->flags | TCPFLAGS_USE_TLS ;
    } else
#endif
    if(strstr(url, "://") != NULL) {
        if(isRedirection) {
            return ERR_TCPIPUNAPI_REDIRECT_TO_HTTPS_WHICH_IS_UNSUPPORTED;
        } else {
            return ERR_TCPIPUNAPI_HTTPS_UNSUPPORTED;
        }
    } else {
        if(isRedirection) {
            return ERR_TCPIPUNAPI_NON_ABSOLUTE_URL_ON_REDIRECT_REQUEST;
        }
        strcpy(domainName, url);
    }

    if(url[0] == '/') {
        strcpy(remoteFilePath, url);
    } else {
        remoteFilePath[0] = '/';
        remoteFilePath[1] = '\0';
        pointer = strstr(domainName, "/");
        if(pointer != NULL) {
            *pointer = '\0';
            strcpy(remoteFilePath+1, pointer+1);
        }

        ExtractPortNumberFromDomainName();
    }

    return ERR_TCPIPUNAPI_OK;
}


inline void ExtractPortNumberFromDomainName()
{
    char* pointer;

    pointer = strstr(domainName, ":");
    if(pointer == NULL) {
#ifdef USE_TLS
		if (!useHttps)
#endif
			TcpConnectionParameters->remotePort = HTTP_DEFAULT_PORT;
#ifdef USE_TLS
		else
			TcpConnectionParameters->remotePort = HTTPS_DEFAULT_PORT;
#endif
        return;
    }

    *pointer = '\0';
    pointer++;
    TcpConnectionParameters->remotePort = atoi(pointer);
}


inline HgetReturnCode_t DoHttpWork()
{
    HgetReturnCode_t funcret;
    bool must_continue;

    do {
        must_continue = false;
        redirectionRequests = 0;
        keepingConnectionAlive &= continue_using_keep_alive;

        ResetTcpBuffer();

        if ((!keepingConnectionAlive)||((keepingConnectionAlive)&&(redirectionUrlIsNewDomainName))) {
            funcret = ResolveServerName();
            if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;

            funcret = OpenTcpConnection();
            if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;
        }

        do {
            // Initialize HTTP Variables
            contentLength = 0;
            redirectionRequested = false;
            continueReceived = false;
            isChunkedTransfer = false;
            newLocationReceived = false;
            indicateblockprogress = false;

            funcret = SendHttpRequest();
            if (funcret != ERR_TCPIPUNAPI_OK) {
                if ((keepingConnectionAlive)&&(continue_using_keep_alive)) {
                    keepingConnectionAlive = false;
                    continue_using_keep_alive = false;
                    must_continue = true;
                    break;
                }
                else
                    return funcret;
            }

            funcret = ReadResponseHeaders();
            if (funcret != ERR_TCPIPUNAPI_OK) {
                if ((keepingConnectionAlive)&&(continue_using_keep_alive)) {
                    keepingConnectionAlive = false;
                    continue_using_keep_alive = false;
                    must_continue = true;
                    break;
                }
                else
                    return funcret;
            }

            funcret = CheckHeaderErrors();
            if (funcret != ERR_TCPIPUNAPI_OK) {
                if ((keepingConnectionAlive)&&(continue_using_keep_alive)) {
                    keepingConnectionAlive = false;
                    continue_using_keep_alive = false;
                    must_continue = true;
                    break;
                }
                else
                    return funcret;
            }

            if(redirectionRequested) {
                if(redirectionUrlIsNewDomainName) {
                    CloseTcpConnection();
                    funcret = ResolveServerName();
                    if (funcret != ERR_TCPIPUNAPI_OK)
                        return funcret;
                    funcret = OpenTcpConnection();
                    if (funcret != ERR_TCPIPUNAPI_OK)
                        return funcret;
                }
                ResetTcpBuffer();
            } else if(continueReceived) {
                funcret = DiscardBogusHttpContent();
                if (funcret != ERR_TCPIPUNAPI_OK) {
                    if ((keepingConnectionAlive)&&(continue_using_keep_alive)) {
                        keepingConnectionAlive = false;
                        continue_using_keep_alive = false;
                        must_continue = true;
                        break;
                    }
                    else
                        return funcret;
                }
            }
        } while(continueReceived || redirectionRequested);

        if (must_continue)
            continue;
        funcret = DownloadHttpContents();
        break;
    } while (1);

    return funcret;
}


inline HgetReturnCode_t DiscardBogusHttpContent()
{
    HgetReturnCode_t funcret = ERR_TCPIPUNAPI_OK;
    while(remainingInputData > 0) {
        funcret = GetInputByte(NULL);
        if (funcret!=ERR_TCPIPUNAPI_OK)
            return funcret;
    }
    return funcret;
}


inline HgetReturnCode_t SendHttpRequest()
{
    HgetReturnCode_t funcret;
    sprintf(TcpOutputData, "GET %s HTTP/1.1\r\n", remoteFilePath);
    funcret = SendLineToTcp(TcpOutputData);
    if (funcret!=ERR_TCPIPUNAPI_OK)
        return funcret;
    sprintf(TcpOutputData, "Host: %s\r\n", domainName);
    funcret = SendLineToTcp(TcpOutputData);
    if (funcret!=ERR_TCPIPUNAPI_OK)
        return funcret;
    sprintf(TcpOutputData, "User-Agent: %s\r\n", user_agent);
    funcret = SendLineToTcp(TcpOutputData);
    if (funcret!=ERR_TCPIPUNAPI_OK)
        return funcret;
    if (tryKeepAlive) {
        sprintf(TcpOutputData, "Connection: Keep-Alive\r\n");
        funcret = SendLineToTcp(TcpOutputData);
        if (funcret!=ERR_TCPIPUNAPI_OK)
            return funcret;
    }
    sprintf(TcpOutputData,"\r\n");
    funcret = SendLineToTcp(TcpOutputData);
    return funcret;
}


HgetReturnCode_t ReadResponseHeaders()
{
    HgetReturnCode_t funcret;

    emptyLineReaded = 0;
    zeroContentLengthAnnounced = false;

    funcret = ReadResponseStatus();
    if (funcret == ERR_TCPIPUNAPI_OK)
    {
        while(!emptyLineReaded) {
            funcret = ReadNextHeader();
            if (funcret == ERR_TCPIPUNAPI_OK)
                funcret = ProcessNextHeader();
        }
        if (funcret == ERR_TCPIPUNAPI_OK)
            funcret = ProcessResponseStatus();
    }

    return funcret;
}


inline HgetReturnCode_t ReadResponseStatus()
{
    HgetReturnCode_t funcret;
    char* pointer;
    funcret = ReadNextHeader();
    if (funcret != ERR_TCPIPUNAPI_OK)
        return funcret;
    strcpy(statusLine, headerLine);
    pointer = statusLine;
    SkipCharsUntil(pointer, ' ');
    SkipCharsWhile(pointer, ' ');
    responseStatusCode = atoi(pointer);
    responseStatusCodeFirstDigit = (byte)*pointer - (byte)'0';
    return ERR_TCPIPUNAPI_OK;
}


inline HgetReturnCode_t ProcessResponseStatus()
{
    if(responseStatusCode == 401) {
        return ERR_HGET_AUTH_FAILED;
    }

    if(responseStatusCodeFirstDigit == 1) {
        continueReceived = true;
    } else if(responseStatusCodeFirstDigit == 3) {
        redirectionRequested = true;
		++redirectionRequests;
		if (redirectionRequests>MAX_REDIRECTIONS)
			return ERR_HGET_TOO_MANY_REDIRECTS;
    } else if(responseStatusCodeFirstDigit != 2) {
        return ERR_HGET_HTTP_ERROR;
    }

    return ERR_TCPIPUNAPI_OK;
}


HgetReturnCode_t ReadNextHeader()
{
    char* pointer;
    byte data;
    HgetReturnCode_t funcret;
    pointer = headerLine;

    funcret = GetInputByte(&data);
    if (funcret != ERR_TCPIPUNAPI_OK)
        return funcret;
    if(data == '\r') {
        funcret = SkipLF();
        if (funcret != ERR_TCPIPUNAPI_OK)
            return funcret;
        emptyLineReaded = 1;
        return ERR_TCPIPUNAPI_OK;
    }
    *pointer = data;
    pointer++;

    do {
        funcret = GetInputByte(&data);
        if (funcret != ERR_TCPIPUNAPI_OK)
            return funcret;
        if(data == '\r') {
            *pointer = '\0';
            funcret = SkipLF();
            if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;
        } else {
            *pointer = data;
            pointer++;
        }
    } while(data != '\r');

    return ERR_TCPIPUNAPI_OK;
}


inline HgetReturnCode_t ProcessNextHeader()
{
    char* pointer;

    if(emptyLineReaded) {
        return ERR_TCPIPUNAPI_OK;
    }

    if(continueReceived) {
        return ERR_TCPIPUNAPI_OK;
    }

    ExtractHeaderTitleAndContents();

    if(HeaderTitleIs("Content-Length")) {
        contentLength = atol(headerContents);
        if (thereisasizecallback)
            SendContentSize(contentLength);
        if(contentLength == 0)
            zeroContentLengthAnnounced = true;
    }

    if(HeaderTitleIs("Transfer-Encoding")) {
        if(HeaderContentsIs("Chunked")) {
            isChunkedTransfer = true;
        }
    }

    if(HeaderTitleIs("WWW-Authenticate")) {
        return ERR_HGET_UNK_AUTH_METHOD_REQUEST;
    }

    if(HeaderTitleIs("Location")) {
        strcpy(redirectionFullLocation, headerContents);
        newLocationReceived = true;
        ProcessUrl(headerContents, true);
    }

    if(HeaderTitleIs("Connection") && HeaderContentsIs("close") && (tryKeepAlive)) {
        tryKeepAlive = false;
    }

    return ERR_TCPIPUNAPI_OK;
}


inline void ExtractHeaderTitleAndContents()
{
    char* pointer;
    pointer = headerLine;

    SkipCharsWhile(pointer, ' ');
    headerTitle = headerLine;
    SkipCharsUntil(pointer, ':');

    *pointer = '\0';
    pointer++;
    SkipCharsWhile(pointer, ' ');

    headerContents = pointer;
}

inline HgetReturnCode_t CheckHeaderErrors()
{
    if(contentLength == 0 && !zeroContentLengthAnnounced && !isChunkedTransfer)
        return ERR_TCPIPUNAPI_OK;
    else if(redirectionRequested && !newLocationReceived)
        return ERR_HGET_REDIRECT_BUT_NO_NEW_LOCATION_PROVIDED;
    else
        return ERR_TCPIPUNAPI_OK;
}


bool HeaderTitleIs(char* string)
{
    return strcmpi(headerTitle, string) == 0;
}


bool HeaderContentsIs(char* string)
{
    return strcmpi(headerContents, string) == 0;
}


HgetReturnCode_t SendLineToTcp(char* string)
{
    int len;

    len = strlen(string);
    return SendTcpData(string, len);
}


inline HgetReturnCode_t DownloadHttpContents()
{
    HgetReturnCode_t funcret;

    if(isChunkedTransfer) {
        funcret = DoChunkedDataTransfer();
    } else
        funcret = DoDirectDatatransfer();

    return funcret;
}

inline HgetReturnCode_t DoDirectDatatransfer()
{
    HgetReturnCode_t funcret = ERR_TCPIPUNAPI_OK;

    if(zeroContentLengthAnnounced) {
        return funcret;
    }

	if (contentLength) {
		blockSize = contentLength/25;
		currentBlock = 0;
		if (blockSize)
            indicateblockprogress = true;
	}

    while(contentLength == 0 || receivedLength < contentLength) {
        funcret = EnsureThereIsTcpDataAvailable();
        if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;
        receivedLength += remainingInputData;
        currentBlock += remainingInputData;
        UpdateReceivingMessage();
        if (!WriteContents(inputDataPointer, remainingInputData))
            return ERR_HGET_DISK_WRITE_ERROR;
        ResetTcpBuffer();
    }
    return funcret;
}


inline HgetReturnCode_t DoChunkedDataTransfer()
{
    int chunkSizeInBuffer;
    HgetReturnCode_t funcret = ERR_TCPIPUNAPI_OK;

    currentChunkSize = GetNextChunkSize();

	if (contentLength) {
		blockSize = contentLength/25;
		currentBlock = 0;
	}

    while(1) {
        if(currentChunkSize == 0) {
            funcret = GetInputByte(NULL); //Chunk data is followed by an extra CRLF
            if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;
            funcret = GetInputByte(NULL);
            if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;
            currentChunkSize = GetNextChunkSize();
            if(currentChunkSize == 0) {
                break;
            }
        }

        if(remainingInputData == 0) {
            funcret = EnsureThereIsTcpDataAvailable();
            if (funcret != ERR_TCPIPUNAPI_OK)
                return funcret;
        }

        chunkSizeInBuffer = currentChunkSize > remainingInputData ? remainingInputData : currentChunkSize;
        receivedLength += chunkSizeInBuffer;
        UpdateReceivingMessage();
        if (!WriteContents(inputDataPointer, chunkSizeInBuffer))
            return ERR_HGET_DISK_WRITE_ERROR;
        inputDataPointer += chunkSizeInBuffer;
        currentChunkSize -= chunkSizeInBuffer;
        remainingInputData -= chunkSizeInBuffer;
    }
    return funcret;
}


long GetNextChunkSize()
{
    byte validHexDigit = 1;
    char data;
    long value = 0;

    while(validHexDigit) {
        if (GetInputByte(&data)!=ERR_TCPIPUNAPI_OK)
            return 0; //bad hack, should not happen a lot anyway, and most servers do not used chunked transfers anyway
        ToLowerCase(data);
        if(data >= '0' && data <= '9') {
            value = value*16 + data - '0';
        } else if(data >= 'a' && data <= 'f') {
            value = value*16 + data - 'a' + 10;
        } else {
            validHexDigit = 0;
        }
    }

    do {
        if (GetInputByte(&data)!=ERR_TCPIPUNAPI_OK)
            return 0; //bad hack, should not happen a lot anyway, and most servers do not used chunked transfers anyway
    } while(data != 10);

    return value;
}


/* Functions Related to Callbacks Handling */


void UpdateReceivingMessage()
{
    if(thereisacallback) {
		if (indicateblockprogress) {
			while (currentBlock>=blockSize)
			{
				currentBlock-=blockSize;
				UpdateReceivedStatus(false);
			}
		} else
			UpdateReceivedStatus(true);
    }
}

bool WriteContents(byte* dataPointer, int size)
{
    if (thereisasavecallback) {
        SaveReceivedData(dataPointer, size);
        return true;
    }
}


/* Functions Related to Console I/O */


inline bool InitializeTcpipUnapi()
{
    int i;
    i = UnapiGetCount("TCP/IP");
    if(i==0)
        return false;
    UnapiBuildCodeBlock(NULL, 1, codeBlock);
    reg.Bytes.B = 0;
    UnapiCall(codeBlock, TCPIP_TCP_ABORT, &reg, REGS_MAIN, REGS_MAIN);
    TcpConnectionParameters->remotePort = HTTP_DEFAULT_PORT;
    TcpConnectionParameters->localPort = 0xFFFF;
    TcpConnectionParameters->userTimeout = 0;
    TcpConnectionParameters->flags = 0;
    return true;
}


inline bool CheckTcpipCapabilities()
{
    reg.Bytes.B = 1;
    UnapiCall(codeBlock, TCPIP_GET_CAPAB, &reg, REGS_MAIN, REGS_MAIN);
    if((reg.Bytes.L & (1 << 3)) == 0)
        return false;

#ifdef USE_TLS
    TlsIsSupported = false;
    safeTlsIsSupported = false;

    if(tcpIpSpecificationVersionMain == 0 || (tcpIpSpecificationVersionMain == 1 && tcpIpSpecificationVersionSecondary == 0))
        return true; //TCP/IP UNAPI <1.1 has no TLS support at all

    if(reg.Bytes.D & TCPIP_CAPAB_VERIFY_CERTIFICATE)
		safeTlsIsSupported = true;

    reg.Bytes.B = 4;
    UnapiCall(codeBlock, TCPIP_GET_CAPAB, &reg, REGS_MAIN, REGS_MAIN);
    if(reg.Bytes.H & 1)
        TlsIsSupported = true;
#endif
    return true;
}


/* Functions Related to Network I/O */


HgetReturnCode_t EnsureThereIsTcpDataAvailable()
{
    HgetReturnCode_t funcret;
    ticksWaited = 0;

	sysTimerHold = *SYSTIMER;
	while(remainingInputData == 0) {
		if (sysTimerHold != *SYSTIMER) {
			++ticksWaited;
			if(ticksWaited >= TICKS_TO_WAIT)
                return ERR_HGET_TRANSFER_TIMEOUT;
			sysTimerHold = *SYSTIMER;
		}

		funcret = ReadAsMuchTcpDataAsPossible();
		if (funcret!=ERR_TCPIPUNAPI_OK)
            return funcret;
		if(remainingInputData == 0) {
			if (!EnsureTcpConnectionIsStillOpen())
                return ERR_HGET_CONN_LOST;
            LetTcpipBreathe();
		}
	}
	return ERR_TCPIPUNAPI_OK;
}


inline bool EnsureTcpConnectionIsStillOpen()
{
    reg.Bytes.B = conn;
    reg.Words.HL = 0;
    UnapiCall(codeBlock, TCPIP_TCP_STATE, &reg, REGS_MAIN, REGS_MAIN);
    if(reg.Bytes.A != 0 || reg.Bytes.B != 4)
        return false;
    else
        return true;
}


inline HgetReturnCode_t ReadAsMuchTcpDataAsPossible()
{
    if(AbortIfEscIsPressed())
        return ERR_HGET_ESC_CANCELLED;
    reg.Bytes.B = conn;
    reg.Words.DE = (int)(TcpInputData);
    reg.Words.HL = TCP_BUFFER_SIZE;
    UnapiCall(codeBlock, TCPIP_TCP_RCV, &reg, REGS_MAIN, REGS_MAIN);
    if(reg.Bytes.A != 0)
        return ERR_TCPIPUNAPI_RECEIVE_ERROR;

    remainingInputData = reg.UWords.BC;
    inputDataPointer = TcpInputData;

    return ERR_TCPIPUNAPI_OK;
}


HgetReturnCode_t GetInputByte(byte *data)
{
    HgetReturnCode_t funcret = EnsureThereIsTcpDataAvailable();

    if (funcret == ERR_TCPIPUNAPI_OK) {
        if (data)
            *data = *inputDataPointer;
        inputDataPointer++;
        remainingInputData--;
    }
    return funcret;
}


inline bool CheckNetworkConnection()
{
    UnapiCall(codeBlock, TCPIP_NET_STATE, &reg, REGS_NONE, REGS_MAIN);
    if(reg.Bytes.B == TCPIP_NET_STATE_CLOSED || reg.Bytes.B == TCPIP_NET_STATE_CLOSING) {
        return false;
    }
    return true;
}


HgetReturnCode_t ResolveServerName()
{
    reg.Words.HL = (int)domainName;
    reg.Bytes.B = 0;
    UnapiCall(codeBlock, TCPIP_DNS_Q, &reg, REGS_MAIN, REGS_MAIN);
    if(reg.Bytes.A == ERR_NO_NETWORK)
        return ERR_TCPIPUNAPI_NO_CONNECTION;
    else if(reg.Bytes.A == ERR_NO_DNS)
        return ERR_TCPIPUNAPI_NO_DNS_CONFIGURED;
    else if(reg.Bytes.A == ERR_NOT_IMP)
        return ERR_TCPIPUNAPI_NOT_DNS_CAPABLE;
    else if(reg.Bytes.A != (byte)ERR_OK)
        return ERR_TCPIPUNAPI_UNKNOWN_ERROR;

    do {
        if (AbortIfEscIsPressed())
            return ERR_HGET_ESC_CANCELLED;
        LetTcpipBreathe();
        reg.Bytes.B = 0;
        UnapiCall(codeBlock, TCPIP_DNS_S, &reg, REGS_MAIN, REGS_MAIN);
    } while (reg.Bytes.A == 0 && reg.Bytes.B == 1);

    if(reg.Bytes.A != 0) {
        if(reg.Bytes.B == 2)
            return ERR_TCPIPUNAPI_DNS_FAILURE;
        else if(reg.Bytes.B == 3)
            return ERR_TCPIPUNAPI_DNS_UNKNWON_HOSTNAME;
        else if(reg.Bytes.B == 5)
            return ERR_TCPIPUNAPI_DNS_REFUSED;
        else if(reg.Bytes.B == 16 || reg.Bytes.B == 17)
            return ERR_TCPIPUNAPI_DNS_NO_RESPONSE;
        else if(reg.Bytes.B == 19)
            return ERR_TCPIPUNAPI_NO_CONNECTION;
        else if(reg.Bytes.B == 0)
            return ERR_TCPIPUNAPI_DNS_QUERY_FAILED;
        else
            return ERR_TCPIPUNAPI_DNS_UNKNWON_ERROR;
    }

    TcpConnectionParameters->remoteIP[0] = reg.Bytes.L;
    TcpConnectionParameters->remoteIP[1] = reg.Bytes.H;
    TcpConnectionParameters->remoteIP[2] = reg.Bytes.E;
    TcpConnectionParameters->remoteIP[3] = reg.Bytes.D;
#ifdef USE_TLS
	if (useHttps) {
		if (mustCheckCertificate)
			TcpConnectionParameters->flags = TcpConnectionParameters->flags | TCPFLAGS_VERIFY_CERTIFICATE ;
		if (mustCheckHostName)
			TcpConnectionParameters->hostName =  (int)domainName;
		else
			TcpConnectionParameters->hostName =  0;
	} else
#endif
	{
		TcpConnectionParameters->flags = 0 ;
		TcpConnectionParameters->hostName =  0;
	}

    return ERR_TCPIPUNAPI_OK;
}


HgetReturnCode_t OpenTcpConnection()
{
    reg.Words.HL = (int)TcpConnectionParameters;
    UnapiCall(codeBlock, TCPIP_TCP_OPEN, &reg, REGS_MAIN, REGS_MAIN);
    if(reg.Bytes.A == (byte)ERR_NO_FREE_CONN) {
        reg.Bytes.B = 0;
        UnapiCall(codeBlock, TCPIP_TCP_ABORT, &reg, REGS_MAIN, REGS_NONE);
        reg.Words.HL = (int)TcpConnectionParameters;
        UnapiCall(codeBlock, TCPIP_TCP_OPEN, &reg, REGS_MAIN, REGS_MAIN);
    }

    if(reg.Bytes.A == (byte)ERR_NO_NETWORK)
        return ERR_TCPIPUNAPI_NO_CONNECTION;
    else if(reg.Bytes.A != 0)
        return ERR_TCPIPUNAPI_CONNECTION_FAILED;

    conn = reg.Bytes.B;

    ticksWaited = 0;
    sysTimerHold = *SYSTIMER;
    do {
        if (AbortIfEscIsPressed())
            return ERR_HGET_ESC_CANCELLED;

        if (*SYSTIMER != sysTimerHold)
        {
            ticksWaited++;
            if(ticksWaited >= TICKS_TO_WAIT)
                return ERR_TCPIPUNAPI_CONNECTION_TIMEOUT;
            sysTimerHold = *SYSTIMER;
        }

        reg.Bytes.B = conn;
        reg.Words.HL = 0;
        UnapiCall(codeBlock, TCPIP_TCP_STATE, &reg, REGS_MAIN, REGS_MAIN);
        if ((reg.Bytes.A) == 0 && (reg.Bytes.B != 4))
            LetTcpipBreathe();
        else
            break;
    } while(1);

    if(reg.Bytes.A != 0)
		return ERR_TCPIPUNAPI_CONNECTION_FAILED;

    return ERR_TCPIPUNAPI_OK;
}


void CloseTcpConnection()
{
    if(conn != 0) {
        reg.Bytes.B = conn;
        UnapiCall(codeBlock, TCPIP_TCP_CLOSE, &reg, REGS_MAIN, REGS_NONE);
        conn = 0;
    }
}


inline HgetReturnCode_t SendTcpData(byte* data, int dataSize)
{
    do {
        do {
            reg.Bytes.B = conn;
            reg.Words.DE = (int)data;
            reg.Words.HL = dataSize > TCPOUT_STEP_SIZE ? TCPOUT_STEP_SIZE : dataSize;
            reg.Bytes.C = 1;
            UnapiCall(codeBlock, TCPIP_TCP_SEND, &reg, REGS_MAIN, REGS_AF);
            if(reg.Bytes.A == ERR_BUFFER) {
                LetTcpipBreathe();
                reg.Bytes.A = ERR_BUFFER;
            }
        } while(reg.Bytes.A == ERR_BUFFER);
        dataSize -= TCPOUT_STEP_SIZE;
        data += reg.Words.HL;   //Unmodified since REGS_AF was used for output
    } while(dataSize > 0 && reg.Bytes.A == 0);

    if(reg.Bytes.A == ERR_NO_CONN)
        return ERR_TCPIPUNAPI_NO_CONNECTION;
    else if(reg.Bytes.A != 0)
        return ERR_TCPIPUNAPI_SEND_ERROR;
    else
        return ERR_TCPIPUNAPI_OK;
}


/* Functions Related to Strings */


bool StringStartsWith(const char* stringToCheck, const char* startingToken)
{
    int len;
    len = strlen(startingToken);
    return strncmpi(stringToCheck, startingToken, len) == 0;
}


bool charcmpi(char c1, char c2) {
    return (islower(c1) ? toupper(c1) : c1) != (islower(c2) ? toupper(c2) : c2);
}

uint8_t strcmpi(const char *a1, const char *a2) {
	char c1, c2;
	/* Want both assignments to happen but a 0 in both to quit, so it's | not || */
	while((c1=*a1) | (c2=*a2)) {
		if (!c1 || !c2 || /* Unneccesary? */ charcmpi(c1, c2))
			return 1;
		a1++;
		a2++;
	}
	return 0;
}


uint8_t strncmpi(const char *a1, const char *a2, unsigned size) {
	char c1, c2;
	/* Want both assignments to happen but a 0 in both to quit, so it's | not || */
	while((size > 0) && (c1=*a1) | (c2=*a2)) {
		if (!c1 || !c2 || /* Unneccesary? */ charcmpi(c1, c2))
			return 1;
		a1++;
		a2++;
		size--;
	}
	return 0;
}

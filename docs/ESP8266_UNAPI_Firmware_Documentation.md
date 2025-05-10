# ESP8266 UNAPI Firmware Documentation

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/R6R2BRGX6)

Copyright (c) 2019 - 2025 Oduvaldo Pavan Junior ( ducasp@ gmail.com ) All rights reserved.

## Table of Contents

1. [Introduction / Design Choices - Goals](#goals)
   1. [Introduction](#intro)
   2. [Design Choices - Goals](#dcgoals)    
2. [Protocol](#protocol)
3. [Configurations / Behavior Modifiers](#config)
4. [Commands](#commands)
   1. [Custom Commands](#ccommands)
      1. [Reset](#creset)
      2. [Warm Reset](#cwreset)
      3. [Query](#cquery)
      4. [Get Version](#cgetver)
      5. [Retry Transmission](#cretry)
      6. [Scan Available Access Points](#cscans)
      7. [Scan Available Access Points Results](#cscanr)
      8. [Connect to Access Point](#cconnectap)
      9. [Get Access Point Status](#cgetapsts)
      10. [Update Firmware Over the Air](#cotafwupdt)
      11. [Update Certificates Over the Air](#cotacertupdt)
      12. [Start Local Firmware Update](#clocalfwupdt)
      13. [Start Local Certificate Update](#clocalcertupdt)
      14. [Write Block](#clocalwriteblock)
      15. [Finish Local Update](#clocalfinish)
      16. [Initialize Certificates](#cinitcerts)
      17. [Disable Nagle Algorithm](#cnagledis)
      18. [Enable Nagle Algorithm](#cnagleen)
      19. [Set Radio Off Time-Out](#cradiotimeout)
      20. [Disable Radio](#cradiodisable)
      21. [Get Settings](#cgetset)
      22. [Get Auto Clock Settings](#cgetaclkset)
      23. [Set Auto Clock Settings](#csetaclkset)
      24. [Get Date and Time](#cgetdate)
      25. [Hold Connection](#choldconn)
      26. [Release Connection](#creleaseconn)
      27. [Turn Wi-Fi Off](#cturnwifioff)
      28. [Turn RS232 Off](#cturnrs232off)
      29. [Clear AP from memory](#cclearap)
   2. [UNAPI Commands](#ucommands)
      1. [TCPIP_GET_CAPAB](#cugetcapab)
      2. [TCPIP_GET_IPINFO](#cugetipinfo)
      3. [TCPIP_NET_STATE](#cunetstate)
      4. [TCPIP_SEND_ECHO](#cusendecho)
      5. [TCPIP_RCV_ECHO](#curcvecho)
      6. [TCPIP_DNS_Q](#cudnsq)
      7. [TCPIP_DNS_Q_NEW](#cudnsqnew)
      8. [TCPIP_DNS_S](#cudnss)
      9. [TCPIP_UDP_OPEN](#cuudpopen)
      10. [TCPIP_UDP_CLOSE](#cuudpclose)
      11. [TCPIP_UDP_STATE](#cuudpstate)
      12. [TCPIP_UDP_SEND](#cuudpsend)
      13. [TCPIP_UDP_RCV](#cuudprcv)
      14. [TCPIP_TCP_OPEN](#cutcpopen)
      15. [TCPIP_TCP_CLOSE](#cutcpclose)
      16. [TCPIP_TCP_ABORT](#cutcpabort)
      17. [TCPIP_TCP_STATE](#cutcpstate)
      18. [TCPIP_TCP_SEND](#cutcpsend)
      19. [TCPIP_TCP_RCV](#cutcprcv)
      20. [TCPIP_TCP_DISCARD or TCPIP_TCP_FLUSH](#cutcpdiscard)
      21. [TCPIP_RAW_OPEN](#curawopen)
      22. [TCPIP_RAW_CLOSE](#curawclose)
      23. [TCPIP_RAW_STATE](#curawstate)
      24. [TCPIP_RAW_SEND](#curawsend)
      25. [TCPIP_RAW_RCV](#curawrcv)
      26. [TCPIP_CONFIG_AUTOIP](#cucfgautoip)
      27. [TCPIP_CONFIG_IP](#cucfgip)
      28. [TCPIP_CONFIG_TTL](#cucfgttl)
      29. [TCPIP_CONFIG_PING](#cucfgping)
      30. [TCPIP_WAIT](#cuwait)

## <a name="goals"></a> Introduction / Design Choices - Goals

### <a name="intro"></a> Introduction

ESP8266 and modules using it, like ESP-01, are a quite affordable way to connect to Wi-Fi. Usually they have quite a few GPIO pins that can be used in many different interfaces, but the most common ESP-01 module do not expose a lot of those pins. That ESP-01 module is what provides network capabilities to the SM-X MSX system designed by Victor Trucco ([Get in touch with the new SM-X | MSX Resource Center](https://www.msx.org/news/hardware/en/get-in-touch-with-the-new-smx)). Victor designed the hardware of SM-X and I was one of a few fortunate fellows that were able to get an early prototype version of it. As he has plenty of projects on his table, and still was pretty much busy debugging possible hardware / interface issues with SM-X, he did not have time to create an interface to use the ESP8266 / ESP-01 module. For some reason, after helping him to understand an issue with the prototype firmware where I/O only interfaces were not working fine, I just had the idea of getting the Network interface working. 

As a concept, Victor told me about Luis Fernando Luca Zanoto Wi-Fi cartridge that also used an ESP-01 ([wifi card | MSX Resource Center ](https://www.msx.org/forum/msx-talk/hardware/wifi-card?page=1)) module, and I've first designed the VHDL module that would bridge the ESP8266 serial TX/RX lines to the MSX, it would be kind of similar to the way that Wi-Fi cartridge worked, but I've made a few changes to it as I've progressed through the development. If you are curious, that VHDL interface code can be found at [src/SM-X · master · Victor Trucco / SM-X · GitLab](https://gitlab.com/victor.trucco/sm-x/-/tree/master/src/SM-X), wifi.vhd describes the "cartridge" that will connect to the MSX bus and react to I/O commands, it also uses a UART and a FIFO, customized for this usage ( UART.vhd and fifo.vhd ), those were provided to me by Victor and I've changed the original fifo.vhd to work like a real fifo (it's original behavior was a little strange, it was not really a circular buffer).

The original idea was to, basically, use ESP8266 AT Firmware and perhaps software made for that Wi-Fi Cartridge... Can't say how bad that idea was... The AT firmware has a protocol that is not tokenized / machine efficient, so, parsing it's responses and assembling commands is a nightmare for a z80, let's say it is not really efficient for those tasks, more specifically given that the AT Firmware that is officially available for ESP-01 due to its small size of flash memory doesn't allow you to say it to stop receiving data until you clear your buffer, it is all unsolicited, non-stop, which in turn required an "artificial" way of slowing down it through the serial port speed. This is not optimal, at all, as even though an MSX using a z80 at 3.58MHz can sometimes get data really fast, some other time it would take some time on an proccess like writing to disk, which would cause data to be overrun due to the unsolicited nature of the AT firmware... Making the matter worse, there was not much software available for the Zanoto Wi-Fi cartridge, so, after spending quite some time on a telnet client using the AT firmware directly and on an UNAPI driver that was talking with the ESP using AT commands and getting less than optimal results (had to limit speed to 33600bps to avoid losing characters, resulting in up to 3.3KB/s transfer, the current design will allow you to achieve 15KB/s at 3.58MHz and much faster speeds at turbo speeds).

So, that was a turning point... One of many... Just to enumerate a few:

1. First I've said that having UNAPI on that thing would not be done by me... I knew from the very start that Konamiman (Nestor Soriano) [MSX-UNAPI-specification/TCP-IP UNAPI specification](https://github.com/Konamiman/MSX-UNAPI-specification/blob/master/docs/TCP-IP%20UNAPI%20specification.md) was the best way to do it... There is plenty of software available for UNAPI compatible adapters... But that would require z80 assembly knowledge (which I didn't have much by that time, most stuff I did was using SDCC / C Language) and to be honest, the first time I've seen the source code of the UNAPI driver for Denyonet it was totally out of the world for me, working through Hooks, slots, etc... Didn't thought I would be able to get something working, even using the Denyonet code as reference...

2. Then I've just tried and after a few months I had a working UNAPI driver (not perfect, but working to a good extent) using ESPRESSIF AT Firmware. At that point I've said that going beyond 3KBps wouldn't be doable, AT firmware and z80 having to parse the unsolicited responses wouldn't allow any better (and probably that is true, unless you use the ESP directly instead of using an API like UNAPI, then you probably can hit something like 5 to 6KBps)... Again, ESPRESSIF documentation at that time on doing ESP8266 custom firmwares was not the best, but I've figured out that the excellent [esp8266/Arduino: ESP8266 core for Arduino](https://github.com/esp8266/Arduino) was perfect for what I've wanted... 

3. And then I've just done a quite stable solution with a custom ESP8266 firmware, and Nestor Soriano (Konamiman) extended the UNAPI specification to allow for TLS/SSL (https uses it). You can guess that it is impossible to do TLS/SSL on a puny 8 bit processor running at 3.58MHz, but ESP8266 has a powerful cpu core that can handle that with ease (just limited to a single connection due to RAM constraints). At first, working a little with BearSSL I thought that it would be troublesome to have it working, but could get it working fine and even added https capability to Konamiman HGET utility, which he merged back into his original version!

4. And then it came to have a ROM version of the driver... Making a RAM version was a lot easier, it is sitting on a 16KB segment of RAM memory that is owned by the driver alone, so I can use RAM to make a lot of stuff... When your driver is running in a ROM, it will boot automatically and you can do cool stuff like setting the MSX clock automatically on boot and not needing to load anything using MSX-DOS. But you are left with 16 bytes of RAM memory that is allocated for your slot (as long as it is not expanded or if it is, the other slots do not have devices using their 4 bytes of RAM) which surely was not enough... But hey, that is not firmware related... 

So, this is basically the story behind this development. And for sure I would like to express my gratitude towards really nice people that helped make this true:

- First, Victor Trucco, who helped me whenever I had questions about VHDL, as well gave me a great insight on how to proceed quite a few times, and also for finally trusting in what I've done to use as the official solution for all SM-X devices he produced!

- Now, Nestor Soriano (Konamiman), this guy is really, really busy and did a lot of relevant stuff for the MSX scene... Not only the UNAPI specification and quite a few of the implementations for it, but also NEXTOR that is the evolution of MSX-DOS among many other relevant stuff. You could imagine that someone that has a vast knowledge would not like to help answering basic questions (as I've never done anything either Network related or that used MSX low level stuff), but he was always really kind and helped me whenever I had a question about UNAPI or Network stuff.

- Last, but not least (so, list is in order of when each one jumped in to assist) KdL, that is best know for the great OCM / Zemmix Neo firmware that expanded those machines to an extent the original project never dreamt of... There is no good adjective to his attention to detail and testing capabilities... His input (some times reporting problems, suggesting features, other times helping with code as well) is really valuable and without him, it wouldn't be nearly as good as it is today (i.e.: automatic clock setting was something he suggested me quite a few times, he has helped with some parts of the UNAPI rom Driver as well)

### <a name="dcgoals"></a> Design Choices - Goals

Well, this custom ESP8266 firmware was designed upon a few baselines:

- Implement most features that are part of TCP-IP Unapi specification. SEND ECHO and RCV ECHO was left out as ESP8266 API's do not implement ping in a way that would work with those commands and I personally don't think it is worth the trouble of implementing the code to have it... The only software that uses those calls, as far as I know, is ping, and that is something of limited use. All RAW connection functionality also was left out, as Espressif and Arduino IDE API's are geared towards UDP and TCP protocols, not for raw use of the network. Currently, the only TCP-IP UNAPI application that doesn't work with this implementation, to my knowledge, is PING.
- Move all TCP-IP UNAPI functionality, including parameters parsing and checking, to the firmware. So, basically the firmware is mostly a TCP-IP UNAPI implementation over a different interface, causing the driver to use extremely low CPU resources just to move data to the ESP and receive the response that is alligned so it can be copied directly to the registers as response.
- No unsolicited events/messages: firmware is slave to command requests and only send data in response to those. This makes a lot, lot easier to work with it on the driver.
- As simple as possible protocol: again, z80 at 3.58MHz can limit quite a few possibilities... I.e.: CRC calculation, even using tables, is not very performatic. Checksum is not really reliable even though it is a lot faster... But in the end, when you are trying to squeeze out as much performance as possible, it still adds some overhead. Since the serial communication between ESP-01 and the FPGA is a short trace route, even at 859372bps communications are really, really reliable, occasionally a byte or two might get lost, but the driver is able to detect this discrepancy and request data to be re-sent.
- A little bit over 2KB of serial input buffer. This is probably overkill as I doubt a MSX can cause a buffer overrun on an ESP, but hey, better be safe than sorry.
- Up to 4 simultaneous connections, TCP or UDP.
- Allow 1 active TCP connection only when using SSL, as it nearly sucks all RAM available for a program running on an ESP8266. Two connections could be possible depending upon the other end configuring less usage of buffers, but this is not reliable.
- It should be possible to update the custom firmware or SSL certificates either through the serial connection (computer reads file contents and send it to the device to update it) or OTA (over the air, connects to a server hosting the update file and update directly from the server).
- As some SM-X devices are prone to get audio noise as a result of interference from the ESP-01 lines (most likely the power line, as interference occurs even when there is no data being sent to/from the ESP), have means to disable radio fully (so power spikes don't occur and thus noise is not generated) or to have it disabled automatically after a pre-set period of time occurs without any connection being active.
- Certificates are installed on a SPIFFS partition to work with Bear SSL. Currently a 256KB partition is enough for this.
- Due to the limited size of flash memory available on ESP-01 modules, customized update routines allow the firmware size to be greater than half the size of (flash memory size - spiff partition). If not for those routines, it wouldn't be possible to implement SSL, as the firmware goes over 400KB in size when BearSSL is included. As a result, you can update the SPIFFS / update certificates at any time, but, if you update the firmware itself, you will need to update certificates as well, as the SPIFFS is corrupted during the firmware update. This minor inconvenience still allows for safe firmware updates that will not kill the device if failing, being possible to resume the update again using the computer or the OTA server.
- Firmware exposed commands are always SYNCHRONOUS, that means, for every command you send you need to wait for the command response before sending a new command. This is not an issue as all TCP-IP UNAPI commands are synchronous, and the ones that are not have a second command to check status and get results, this simplifies the design of both the driver on the computer as well as of the firmware.
- Extra commands that are not part of the TCP-IP UNAPI specification should be implemented to allow for firmware updates, easy detection of device being plugged in, easy time and date retrieval from network, configuration of the device behavior, scanning and connecting to Access Points.

## <a name="protocol"></a> Protocol

The idea is mostly to not have a protocol... Basically, the first byte is an indication of which command to be executed, where TCP-IP UNAPI commands have the values ranging from 1 to 29 matching the values defined by TCP-IP UNAPI specification for each command. Custom commands can use any character starting from '?' that has a value of 63. This is the only thing that is standardized for the protocol. Each command then will treat the remaining bytes as needed, to avoid overhead of unnecessary data being sent for the command execution or the overhead of unnecessary data being replied as result of the command execution. This leads to having a larger driver code, but that will execute a lot faster for simples commands, and having the hability to better suit the command format and response format so it can align better with what the driver needs to put in each z80 register as result.  Commands sent to ESP will mostly use one of those formats:

- Quick Commands: Those commands do not need any parameters, so they are composed of just one byte the command byte. Examples are RESET, RETRY_TX, GET VER.

- Regular Commands: Those commands need to send parameters. So they are composed of a three bytes header and the parameter data:

| CMD_BYTE | DATA_SIZE_MSB | DATA_SIZE_LSB | DATA BYTES |
| -------- | ------------- | ------------- | ---------- |

A command reception is expected that no more than 250ms occur between bytes. After 250ms occur while receiving a command since the last byte was received, firmware will discard the data received so far and get back to idle state waiting a new command.

Command responses format vary wildly, an as such, it is better to refer to each command documentation. Most commands will have a response like this if response data is expected for that command:

| CMD_BYTE | ERROR_CODE | RESP_SIZE_MSB | RESP_SIZE_LSB | RESPONSE BYTES |
| -------- | ---------- | ------------- | ------------- | -------------- |

If no response data is expected for that command:

| CMD_BYTE | ERROR_CODE |
| -------- | ---------- |

## <a name="config"></a> Configurations / Behavior Modifiers

This firmware uses ESP8266 Arduino IDE EEPROM functionality, that uses part of the flash memory that is not affected during firmware or file system updates to store a few bytes of data. 32 bytes of flash memory are reserved for this firmware, that are used as follow:

| EEPROM Address(es) | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | Default Value                    |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------- |
| 0 to 7             | Configuration File Name: this is mostly to indicate whether the EEPROM has already been initialized or not, if not containing the expected string (that is not zero terminated), firmware will consider it will need to initialize EEPROM and device configuration to default values.                                                                                                                                                                                                      | "ESPUNAPI"                       |
| 8                  | Structure Version: this identifies which structure of data has been saved to the EEPROM, if an older version if found, a newer one with the extra configurations is generated, using the default values.                                                                                                                                                                                                                                                                                   | 2                                |
| 9                  | Nagle Algorithm: tells whether Nagle Algorithm is off (0) or on (1).                                                                                                                                                                                                                                                                                                                                                                                                                       | 0                                |
| 10                 | Always On: tells whether Wi-Fi radio is always on (1) or if it will be automatically disabled after a given period (0).                                                                                                                                                                                                                                                                                                                                                                    | 0                                |
| 11-14              | Radio Off Timer: 32 bits unsigned integer containing the time in seconds that need to elapse with the device idle and without open connections before turning off the Wi-Fi radio automatically. Only taken into consideration if Always On is set to 0.                                                                                                                                                                                                                                   | 120                              |
| 15                 | Auto Clock: this is a byte that the ROM version of the driver uses to determine a few of its behavior. Current possible values are: (0) Normal Operation but do not set clock automatically, (1) Will wait up to 10s for SNTP server response during computer boot and if a response is received will to update the system time and date with SNTP information, (2) Same as 1 but will turn off the Wi-Fi radio as soon as clock is set and (3) Will disable Wi-Fi radio and UNAPI Driver. | 0                                |
| 16-19              | GMT Setting: 32 bits integer that allows to add a GMT offset from -12 to +12 hours to the time and date response from the SNTP server. Only full hour adjustment is supported.                                                                                                                                                                                                                                                                                                             | -3 (Brazil - Brasilia time zone) |
| 20-31              | Unused                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     | ?                                |

**Note:** IP Configuration (whether it is set to use DHCP or manual IP assignment) and DNS configuration (whether it is automatic from DHCP or force a different DNS server to be used) are saved within ESPRESSIF nonos SDK automatically, not needing to be saved in the configuration structure.

## <a name="commands"></a> Commands

### <a name="ccommands"></a> Custom Commands

#### <a name="creset"></a> Reset

This command will cause a full reset to the device, as the device was being powered off.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'R'   |

*Response Structure:*

| Position | Function    | Value |
|:--------:| ----------- | ----- |
| 0        | CMD_BYTE    | 'R'   |
| 1        | Fixed Value | '0'   |

#### <a name="cwreset"></a> Warm Reset

This command will cause a quick reset of the device, closing any pending connections and restoring settings.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'W'   |

*Response Structure:*

| Position | Function | Value   |
|:--------:| -------- | ------- |
| 0-4      | Response | "Ready" |

#### <a name="cquery"></a> Query

This command is a simple way to check if the firmware is operational or not. It can serve for purpouses like checking if the device is connected or if the firmware flashing after a reboot has already finished, as the device will only reply once the firmware is operational.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | '?'   |

*Response Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0-2      | Response | "Ok"  |

#### <a name="cgetver"></a> Get Version

This command will return the firmware version.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'V'   |

*Response Structure:*

| Position | Function      | Value |
|:--------:| ------------- | ----- |
| 0        | CMD_BYTE      | 'V'   |
| 1        | Version Major | 1     |
| 2        | Version Minor | 2     |

#### <a name="cretry"></a> Retry Transmission

This command will cause the latest command response to be re-sent.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
| -------- | -------- | ----- |
| 0        | CMD_BYTE | 'r'   |

*Response Structure:* depends on the latest command executed

#### <a name="cscans"></a> Scan Available Access Points

This command will cause the device to start a scan for available Access Points in the range of the device radio. This is a function that takes quite some time to finish, so you need to query for Scan Results after starting the Scan with this command.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'S'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'S'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cscanr"></a> Scan Available Access Points Results

This command will return the results of the latest scan for available Access Points, or, the progress of the current scan if it did not finish.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 's'   |

*Response Structure when not finished or no Access Point has been found:*

| Position | Function   | Value                                                                                       |
|:--------:| ---------- | ------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 's'                                                                                         |
| 1        | Error Code | 2 - Scan finished and no Network in range<br/>3 - Scan still in progress, check again later |

*Response Structure when finished:*

| Position | Function                      | Value                 |
|:--------:| ----------------------------- | --------------------- |
| 0        | CMD_BYTE                      | 's'                   |
| 1        | Error Code                    | 0 - Ok                |
| 2        | Number of Access Points Found | 1 to 255              |
| 3 - XXX  | Access Point Information List | See description below |

*Access Point Information List*:

The list has a variable size and it is always comprised of:

| Position | Function   | Value                                                                                             |
|:--------:| ---------- | ------------------------------------------------------------------------------------------------- |
| 0 - X    | AP Name    | A variable number of characters describing the Access Point Name, NULL character (0) is not valid |
| X+1      | Separator  | 0 - Null character, indicating the end of the AP Name                                             |
| X+2      | Encryption | 'O' - Means open, no password needed<br/>'E' - Means encrypted, password needed                   |
| X+3 - Y  | AP Name    | ....                                                                                              |

#### <a name="cconnectap"></a> Connect to Access Point

This command will connect to a given Access Point using a password if provided. **NOTE:** this command may take up to 10 seconds to send a response!

*Input Parameters:* 

The connection parameters, as follow:

| Position | Function  | Value                                                                                             |
|:--------:| --------- | ------------------------------------------------------------------------------------------------- |
| 0 - X    | AP Name   | A variable number of characters describing the Access Point Name, NULL character (0) is not valid |
| X+1      | Separator | 0 - Null character, indicating the end of the AP Name                                             |
| X+2 - Y  | Password  | Optional parameter - if provided, will be used as the AP password.                                |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'A'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                               |
|:--------:| ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'A'                                                                                                                                                                                                 |
| 1        | Error Code | 0 - Ok, connected to the AP<br/>2 - Could not connect to the AP<br/>4 - Invalid parameters: most likely too short AP Name (less than 2 characters) or AP Name doesn't have the separator at the end |

#### <a name="cgetapsts"></a> Get Access Point Status

This command will retrieve the current connection status and the Access Point name if connected or trying to connect to one.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'g'   |

*Response Structure:*

| Position | Function      | Value                                                                                                                           |
|:--------:| ------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 'g'                                                                                                                             |
| 1        | Error Code    | 0 - Ok                                                                                                                          |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response                                                                     |
| 4        | Status        | 0 - Idle<br/>1 - Connecting<br/>2 - Wrong Password<br/>3 - AP Not Found<br/>4 - Connection Failure<br/>5 - Connected and got IP |
| 5 - X    | AP Name       | Name of the Access point, a NULL (0) terminated string                                                                          |

#### <a name="cotafwupdt"></a> Update Firmware Over the Air

This command will connect to a given server (local or remote) to retrieve the firmware file and update it Over the Air. **NOTE:** this command may take considerable time to send the response depending upon connection speed and network traffic! A Reset command should be issued after a succesful update, and that reset will also take considerable time while it flashes the new firmware to the memory. I recommend no less than 20 seconds time-out for the command response and no less than 50 seconds time-out for the module to respond to commands after the Reset command.

*Input Parameters:* 

The OTA connection parameters, as follow:

| Position | Function           | Value                                                                                            |
|:--------:| ------------------ | ------------------------------------------------------------------------------------------------ |
| 0 - 1    | Port               | Port to be used to connect to the OTA server, LSB in position 0 and MSB in position 1            |
| 2 - X    | Server IP or URL   | Up to 255 bytes long names are accepted (IP addresses won't reach that limit) for DNS resolution |
| X+1      | Separator          | 0 - Null character, indicating the end of Server IP / URL                                        |
| X+2 - Y  | File Path and Name | Path and name of the file to get from the OTA server for this firmware update                    |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'U'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                                                                                                                          |
|:--------:| ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'U'                                                                                                                                                                                                                                                                                            |
| 1        | Error Code | 0 - Ok, file was saved and update will occur on next reset<br/>2 - Could not connect to the AP<br/>3 - Could not retrieve the file using the given input parameters<br/>4 - Invalid parameters: too large URL/IP or no terminator on the URL/IP, not enough input data, missing File Path/Name |

#### <a name="cotacertupdt"></a> Update Certificates Over the Air

This command will connect to a given server (local or remote) to retrieve the certificates file and update it Over the Air. **NOTE:** this command may take considerable time to send the response depending upon connection speed and network traffic! A Reset command should be issued after a succesful update, and that reset will also take considerable time while it flashes the new file system with the new certificates to the memory. I recommend no less than 20 seconds time-out for the command response and no less than 50 seconds time-out for the module to respond to commands after the Reset command. Also, notice that if Initialize Certificates command is not sent after the module finish flashing the data, this initialization procedure will be done during the first SSL connection attempt, which most likely will cause that connection to time-out or take really long time to connect, I strongly recommend using the Initialize Certificates command after a succesful certificate update/module is responding to commands after the reset.

*Input Parameters:* 

The OTA connection parameters, as follow:

| Position | Function           | Value                                                                                            |
|:--------:| ------------------ | ------------------------------------------------------------------------------------------------ |
| 0 - 1    | Port               | Port to be used to connect to the OTA server, LSB in position 0 and MSB in position 1            |
| 2 - X    | Server IP or URL   | Up to 255 bytes long names are accepted (IP addresses won't reach that limit) for DNS resolution |
| X+1      | Separator          | 0 - Null character, indicating the end of Server IP / URL                                        |
| X+2 - Y  | File Path and Name | Path and name of the file to get from the OTA server for this certificate update                 |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'U'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                                                                                                                          |
|:--------:| ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'U'                                                                                                                                                                                                                                                                                            |
| 1        | Error Code | 0 - Ok, file was saved and update will occur on next reset<br/>2 - Could not connect to the AP<br/>3 - Could not retrieve the file using the given input parameters<br/>4 - Invalid parameters: too large URL/IP or no terminator on the URL/IP, not enough input data, missing File Path/Name |

#### <a name="clocalfwupdt"></a> Start Local Firmware Update

This command will request the firmware to start the procedures for local firmware update, whose firmware file will be transmitted through commands. This command will evaluate if the firmware file appears correct and allow the procedure to start or not.

*Input Parameters:* 

The firmware file parameters, as follow:

| Position | Function    | Value                                                                                              |
|:--------:| ----------- | -------------------------------------------------------------------------------------------------- |
| 0 - 7    | File Size   | Size of the file, expressed in a 32 bits unsigned integer, LSB at position 0 and MSB at position 7 |
| 8 - 11   | File Header | The first 4 bytes read from the file                                                               |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'Z'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                             |
|:--------:| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'Z'                                                                                                                                                                                               |
| 1        | Error Code | 0 - Ok, file is accepted and can continue transmission of firmware file using Write Block command<br/>4 - Invalid parameters: file is too large or its header is not valid, not enough input data |

#### <a name="clocalcertupdt"></a> Start Local Certificates Update

This command will request the firmware to start the procedures for local certificates update, whose certificates file will be transmitted through commands. This command will evaluate if the certificate file appears correct and allow the procedure to start or not.

*Input Parameters:* 

The certificates file parameters, as follow:

| Position | Function    | Value                                                                                             |
|:--------:| ----------- | ------------------------------------------------------------------------------------------------- |
| 0 - 7    | File Size   | Size of the file, expressed in a 32 bits unsigned integer, LSB at position 0 and MSB at postion 7 |
| 8 - 11   | File Header | The first 4 bytes read from the file                                                              |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'Y'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                                        |
|:--------:| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0        | CMD_BYTE   | 'Y'                                                                                                                                                                                                          |
| 1        | Error Code | 0 - Ok, file is accepted and can continue transmission of certificates file using Write Block command<br/>3 - File is too large or its header is not valid<br/>4 - Invalid parameters: not enough input data |

#### <a name="clocalwriteblock"></a> Write Block

This command will send a block of data from the firmware or certificates update file to be written to the flash memory. **NOTE:** recommendation is to use 256 bytes blocks, starting from the position 0 of the file (so, the file header is sent as well in the first block sent). As this is a flash memory operation, once firmware gathers enough data to write on a block of flash memory, recommended command response time-out is of at least 10 seconds.

*Input Parameters:* 

The current file block to be written, as follow:

| Position | Function        | Value                            |
|:--------:| --------------- | -------------------------------- |
| 0 - FF   | File Block Data | Data from the current file block |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'z'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                                                                           |
|:--------:| ---------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'z'                                                                                                                                                                                                                                             |
| 1        | Error Code | 0 - Ok, file is accepted and can continue transmission of firmware file using Write Block command<br/>3 - Failure writing block to flash<br/>4 - Invalid parameters: not enough input data or a firmware or certificates update was not started |

#### <a name="clocalfinish"></a> Finish Local Update

This command tells that all firmware or certificate file blocks have been sent. Firmware will then check consistency of the file, and if it considers the file ok, will reboot automatically to move the received data to the firmware block or  to the file system block. **NOTE:** As this is a flash memory operation, and involves file hash checking of the whole received data, recommended command response time-out is of at least 30 seconds. The automatic reset will take considerable time while it flashes the new file system with the new certificates or the new firmware to the memory. I recommend no less than 50 seconds time-out for the module to respond to commands after Finish Local Update command returns Ok.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'E'   |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                                                                                                |
|:--------:| ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'E'                                                                                                                                                                                                                                                  |
| 1        | Error Code | 0 - Ok, transfer was succesful and will reboot to make effective the new firmware or file system<br/>3 - Failure: file consistency check failed, file might be corrupted<br/>4 - Invalid parameters: firmware or certificates update was not started |

#### <a name="cinitcerts"></a> Initialize Certificates

This command will make an index of all certificates included on the new file system. If this command is not sent after a file system / certificates update, that index creation will take place automatically during the request of the first SSL connection, which will cause that command to take too much time to return. Thus, it is recommended that this command is always executed after Certificates update, be it local or OTA. **NOTE:** the indexation procedure takes some time, recommended command response time-out is of at least 60 seconds.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'I'   |

*Response Structure:*

| Position | Function   | Value                                                                                                                                                                          |
|:--------:| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0        | CMD_BYTE   | 'I'                                                                                                                                                                            |
| 1        | Error Code | 0 - Ok, either index creation was already done or it was succesful<br/>3 - Failure: index creation failed, perhaps the certificates and index did not fit the file system size |

#### <a name="cnagledis"></a> Disable Nagle Algorithm

Nagle Algorithm might cause performance issues on some applications. This algorithm will delay sending data over a connection until the transmission window size has been hit or a time-out (usually a couple hundred milliseconds) occurred. This can cause poor performance on block driven protocols that require an ACKnowledge to be sent back, as this ACKnowledge is small, it is usually delayed, thus, restraining the number of packets that can be send over the time. This is specifically detrimental to performance if the other side has "Delayed ACK", where your protocol ACK will wait couple hundred milliseconds to be sent, and the other end ACK to your TCP packet containing your ACK also will be delayed, hurting performance really bad. On the other hand, it might be useful to avoid network congestion when many small packets would be sent consecutivelly, as an example, a Telnet client sending key strokes, each key stroke will use a single TCP packet, so there is a 40 bytes overhead (TCP Header) to send a single byte, while Nagle can just pack a lot of keystrokes on one packet (causes lag, but lowers the network traffic). **NOTE:** The setting won't be applied to any connection that is currently opened, it will apply to any connection made after it. This setting is saved and respected even after a Reset or Power Cycle.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'N'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'N'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cnagleen"></a> Enable Nagle Algorithm

Nagle Algorithm might cause performance issues on some applications. This algorithm will delay sending data over a connection until the transmission window size has been hit or a time-out (usually a couple hundred milliseconds) occurred. This can cause poor performance on block driven protocols that require an ACKnowledge to be sent back, as this ACKnowledge is small, it is usually delayed, thus, restraining the number of packets that can be send over the time. This is specifically detrimental to performance if the other side has "Delayed ACK", where your protocol ACK will wait couple hundred milliseconds to be sent, and the other end ACK to your TCP packet containing your ACK also will be delayed, hurting performance really bad. On the other hand, it might be useful to avoid network congestion when many small packets would be sent consecutivelly, as an example, a Telnet client sending key strokes, each key stroke will use a single TCP packet, so there is a 40 bytes overhead (TCP Header) to send a single byte, while Nagle can just pack a lot of keystrokes on one packet (causes lag, but lowers the network traffic). **NOTE:** The setting won't be applied to any connection that is currently opened, it will apply to any connection made after it. This setting is saved and respected even after a Reset or Power Cycle.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'D'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'D'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cradiotimeout"></a> Set Radio Off Time-Out

This command will set a new Time-Out for the automatic Radio Off feature.

*Input Parameters:* 

The firmware file parameters, as follow:

| Position | Function     | Value                                                                                                                                                                                                    |
|:--------:| ------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0 - 1    | New Time-Out | Time-out in second, expressed in a 16 bits unsigned integer, LSB at position 0 and MSB at position 1. If lower than 30, firmware will use 30 seconds. If higher than 600, firmware will use 600 seconds. |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'T'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                        |
|:--------:| ---------- | ---------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'T'                                                                          |
| 1        | Error Code | 0 - Ok, new time-out saved<br/>4 - Invalid parameters: not enough input data |

#### <a name="cradiodisable"></a> Disable Radio

This command requests immediate action to disable the radio. **NOTE:** radio won't be disabled if a connection is active or if a command is being received. Most commands, except for Release Connection/Reset/Query, will cause the radio to be re-enabled again.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'D'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'D'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cgetset"></a> Get Settings

This command will retrieve the current Nagle and Radio Time-Out settings.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'Q'   |

*Response Structure:*

| Position | Function       | Value                                                       |
|:--------:| -------------- | ----------------------------------------------------------- |
| 0        | CMD_BYTE       | 'Q'                                                         |
| 1 - 2    | Response Size  | 16 bits value (MSB LSB) indicating the size of the response |
| 3-X      | Nagle Setting  | "ON" or "OFF"                                               |
| X+1      | Separator      | ':'                                                         |
| X+2 - Y  | Radio Time-Out | Radio Time-Out as a string, i.e.: "30" or "150"             |

#### <a name="cgetaclkset"></a> Get Auto Clock Settings

This command will retrieve the current Auto Clock setting and the GMT Offset Adjust. **NOTE:** Automatic Clock adjusting is a feature of a device driver, this setting is just to help the device driver to store and retrieve that setting, ESP is not able to do this operation.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'c'   |

*Response Structure:*

| Position | Function           | Value                                                                                                                                                                        |
|:--------:| ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE           | 'c'                                                                                                                                                                          |
| 1        | Error Code         | 0 - Ok                                                                                                                                                                       |
| 2 - 3    | Response Size      | 16 bits value (MSB LSB) indicating the size of the response                                                                                                                  |
| 4        | Auto Clock Setting | 0 - Normal Operation, No Auto Clock<br/>1 - Normal Operation, Try to Auto Set the Clock<br/>2 - Same as 1, but turn off radio after doing it<br/>3 - Disable Radio and UNAPI |
| 5        | GMT Offset Adjust  | Signed byte indicating GMT Offset in hours                                                                                                                                   |

#### <a name="csetaclkset"></a> Set Auto Clock Settings

This command will save the current Auto Clock setting and the GMT Offset Adjust. **NOTE:** Automatic Clock adjusting is a feature of a device driver, this setting is just to help the device driver to store and retrieve that setting, ESP is not able to do this operation.

*Input Parameters:* 

The Auto Clock settings, as follow:

| Position | Function           | Value                                                                                                                                                                        |
|:--------:| ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | Auto Clock Setting | 0 - Normal Operation, No Auto Clock<br/>1 - Normal Operation, Try to Auto Set the Clock<br/>2 - Same as 1, but turn off radio after doing it<br/>3 - Disable Radio and UNAPI |
| 1        | GMT Offset Adjust  | Signed byte indicating GMT Offset in hours, only 0 to 12 and -1 to -12 are accepted                                                                                          |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 'C'                                                                 |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function   | Value                                                                                               |
|:--------:| ---------- | --------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE   | 'C'                                                                                                 |
| 1        | Error Code | 0 - Ok, new settings saved<br/>4 - Invalid parameters: not enough input data or values out of range |

#### <a name="cgetdate"></a> Get Date and Time

This command will retrieve the current date and time through SNTP. **NOTE:** Values are adjusted with Auto Clock GMT Offset Adjust setting.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:|:-------- |:----- |
| 0        | CMD_BYTE | 'G'   |

*Response Structure when SNTP connection was possible:*

| Position | Function      | Value                                                                                |
|:--------:| ------------- | ------------------------------------------------------------------------------------ |
| 0        | CMD_BYTE      | 'G'                                                                                  |
| 1        | Error Code    | 0 - Ok                                                                               |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response                          |
| 4        | Seconds       | Current seconds                                                                      |
| 5        | Minutes       | Current minutes                                                                      |
| 6        | Hours         | Current hours                                                                        |
| 7        | Day           | Current Month day                                                                    |
| 8        | Month         | Current Month                                                                        |
| 9-10     | Year          | Year in an unsigned 16 bits integer format, LSB at position 9 and MSB at position 10 |

*Response Structure when SNTP connection was not possible:*

| Position | Function   | Value                                                  |
|:--------:| ---------- | ------------------------------------------------------ |
| 0        | CMD_BYTE   | 'G'                                                    |
| 1        | Error Code | 2 - No network connection or SNTP server not available |

#### <a name="choldconn"></a> Hold Connection

This command requests that the radio connection is held on regardless of time-out periods.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'H'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'H'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="creleaseconn"></a> Release Connection

This command requests that the radio connection hold is released, re-enabling time-out periods to automatically disable the radio.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'h'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'h'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cturnwifioff"></a> Turn Wi-Fi Off

This command requests that Wi-Fi is fully disabled.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'O'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'O'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cturnrs232off"></a> Turn RS232 Off

This command requests that RS232 is fully disabled. Since no commands will be received anymore, the only way to revert from this is power-cycling the device or a physical reset.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'o'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'o'           |
| 1        | Error Code | 0 - always Ok |

#### <a name="cclearap"></a> Clear AP from memory

This command requests that the current AP is forgotten. Wi-Fi connection, if any, will be lost.

*Input Parameters:* none

*Command Structure:*

| Position | Function | Value |
|:--------:| -------- | ----- |
| 0        | CMD_BYTE | 'a'   |

*Response Structure:*

| Position | Function   | Value         |
|:--------:| ---------- | ------------- |
| 0        | CMD_BYTE   | 'a'           |
| 1        | Error Code | 0 - always Ok |

### <a name="ucommands"></a> UNAPI Commands

The intent of this document is not to explain each command functionality. It is only to tell how the input parameters from UNAPI Commands should be ordered and how the output result will be ordered on the command response. For more details on how each UNAPI command work read the [MSX-UNAPI-specification/TCP-IP UNAPI specification](https://github.com/Konamiman/MSX-UNAPI-specification/blob/master/docs/TCP-IP%20UNAPI%20specification.md).

#### <a name="cugetcapab"></a> TCPIP_GET_CAPAB

This command will retrieve information about the TCP/IP capabilities and features offered by the device.

*Input Parameters:* 

Information to be retrieved:

| Position | Function                               | Value                                                                                                                                                                                       |
|:--------:| -------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | Index of information block to retrieve | 1 - Capabilities and features flags, link level protocol<br/>2 - Connection pool size and status<br/>3 - Maximum datagram size allowed<br/>4 - Second set of capabilities and feature flags |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 1                                                                   |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3-X      | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure for Index 1:*

| Position | Function                 | Value                                                                           |
|:--------:| ------------------------ | ------------------------------------------------------------------------------- |
| 0        | CMD_BYTE                 | 1                                                                               |
| 1        | Error Code               | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range |
| 2 - 3    | Response Size            | 16 bits value (MSB LSB) indicating the size of the response                     |
|          |                          | This part of response only exists if error code is 0                            |
| 4 - 5    | Capabilities Flags       | 16 bits value (LSB MSB)                                                         |
| 6 - 7    | Feature Flags            | 16 bits value (LSB MSB)                                                         |
| 8        | Link Level Protocol used | 1 byte value                                                                    |

*Response Structure for Index 2:*

| Position | Function                                | Value                                                                           |
|:--------:| --------------------------------------- | ------------------------------------------------------------------------------- |
| 0        | CMD_BYTE                                | 1                                                                               |
| 1        | Error Code                              | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range |
| 2 - 3    | Response Size                           | 16 bits value (MSB LSB) indicating the size of the response                     |
|          |                                         | This part of response only exists if error code is 0                            |
| 4        | Maximum Simultaneous TCP Connections    | 1 byte value                                                                    |
| 5        | Maximum Simultaneous UDP Connections    | 1 byte value                                                                    |
| 6        | Free TCP Connections Available          | 1 byte value                                                                    |
| 7        | Free UDP Connections Available          | 1 byte value                                                                    |
| 8        | Maximum Simultaneous raw IP Connections | 1 byte value                                                                    |
| 9        | Free raw IP Connections Available       | 1 byte value                                                                    |

*Response Structure for Index 3:*

| Position | Function                       | Value                                                                           |
|:--------:| ------------------------------ | ------------------------------------------------------------------------------- |
| 0        | CMD_BYTE                       | 1                                                                               |
| 1        | Error Code                     | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range |
| 2 - 3    | Response Size                  | 16 bits value (MSB LSB) indicating the size of the response                     |
|          |                                | This part of response only exists if error code is 0                            |
| 4 - 5    | Maximum incoming datagram size | 16 bits value (LSB MSB)                                                         |
| 6 - 7    | Maximum outgoing datagram size | 16 bits value (LSB MSB)                                                         |

*Response Structure for Index 4:*

| Position | Function                         | Value                                                                           |
|:--------:| -------------------------------- | ------------------------------------------------------------------------------- |
| 0        | CMD_BYTE                         | 1                                                                               |
| 1        | Error Code                       | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range |
| 2 - 3    | Response Size                    | 16 bits value (MSB LSB) indicating the size of the response                     |
|          |                                  | This part of response only exists if error code is 0                            |
| 4 - 5    | Second Set of Capabilities Flags | 16 bits value (LSB MSB)                                                         |
| 6 - 7    | Second Set of Feature Flags      | 16 bits value (LSB MSB)                                                         |

#### <a name="cugetipinfo"></a> TCPIP_GET_IPINFO

This command will retrieve information about the IP related to requested index.

*Input Parameters:* 

Information to be retrieved:

| Position | Function                     | Value                                                                                                                              |
|:--------:| ---------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| 0        | Index of address to retrieve | 1 - Local IP Address<br/>2 - Peer IP Address<br/>3 - Subnet Mask<br/>4 - Default Gateway<br/>5 - Primary DNS<br/>6 - Secondary DNS |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 2                                                                   |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                           |
|:--------:| ------------- | ------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 2                                                                               |
| 1        | Error Code    | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response                     |
|          |               | This part of response only exists if error code is 0                            |
| 4 - 7    | IP Address    | Requested IP Address                                                            |

#### <a name="cunetstate"></a> TCPIP_NET_STATE

This command will retrieve state of network availability.

*Input Parameters:* none

*Command Structure:*

| Position | Function          | Value                                                                       |
|:--------:| ----------------- | --------------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 3                                                                           |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters: 0x0000 |

*Response Structure:*

| Position | Function              | Value                                                       |
|:--------:| --------------------- | ----------------------------------------------------------- |
| 0        | CMD_BYTE              | 3                                                           |
| 1        | Error Code            | 0 - Ok<br/>4 - Invalid parameters: input data received      |
| 2 - 3    | Response Size         | 16 bits value (MSB LSB) indicating the size of the response |
|          |                       | This part of response only exists if error code is 0        |
| 4        | Current network state | 1 byte value                                                |

#### <a name="cusendecho"></a> TCPIP_SEND_ECHO

**This command is not implemented**

#### <a name="curcvecho"></a> TCPIP_RCV_ECHO

**This command is not implemented**

#### <a name="cudnsq"></a> TCPIP_DNS_Q

This command is deprecated and exists only for compatibility with older drivers. Consider using TCPIP_DNS_Q_NEW instead, that lowers the burden on computer CPU as well allowing to not need the driver to have to use RAM to convert IP data. This command expects that any error handling was already made at the driver level, so it receives only the string containing either the IP address to convert to numeric format or the host name to be resolved. **NOTE:** this is a blocking operation, even though UNAPI 1.1 and older requests it to not be. So the command itself will return only after DNS resolves an address or return an error. Recomendation is to have at least 15 seconds of time-out for this command to send a response.

*Input Parameters:* 

Host name or IP address to be resolved:

| Position | Function                                        | Value                  |
|:--------:| ----------------------------------------------- | ---------------------- |
| 0 - X    | Host name or IP address, zero terminated string | Zero terminated string |

*Command Structure:*

| Position | Function          | Value                                                                                              |
|:--------:| ----------------- | -------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 6                                                                                                  |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters, including the zero terminator |

*Response Structure:*

| Position | Function            | Value                                                       |
|:--------:| ------------------- | ----------------------------------------------------------- |
| 0        | CMD_BYTE            | 6                                                           |
| 1        | Error Code          | 0 - Ok<br/>8 - Could not resolve into an IP address         |
| 2 - 3    | Response Size       | 16 bits value (MSB LSB) indicating the size of the response |
|          |                     | This part of response only exists if error code is 0        |
| 4 - 7    | Resolved IP Address | IP Address                                                  |

#### <a name="cudnsqnew"></a> TCPIP_DNS_Q_NEW

This command make a host name resolution on the string received. **NOTE:** this is a blocking operation, even though UNAPI 1.1 and older requests it to not be. So the command itself will return only after DNS resolves an address or return an error. Recomendation is to have at least 15 seconds of time-out for this command to send a response.

*Input Parameters:* 

Host name or IP address to be resolved:

| Position | Function                                        | Value                  |
|:--------:| ----------------------------------------------- | ---------------------- |
| 0        | Flags                                           | Byte                   |
| 1 - X    | Host name or IP address, zero terminated string | Zero terminated string |

*Command Structure:*

| Position | Function          | Value                                                                                              |
|:--------:| ----------------- | -------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 206                                                                                                |
| 1-2      | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters, including the zero terminator |

*Response Structure:*

| Position | Function            | Value                                                                                                                                                                               |
|:--------:| ------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE            | 206                                                                                                                                                                                 |
| 1        | Error Code          | 0 - Ok<br/>4 - Invalid parameter: invalid flag value, not enough input data<br/>6 - Flag indicated it was an IP address, but it wasn't<br/>8 - Could not resolve into an IP address |
| 2 - 3    | Response Size       | 16 bits value (MSB LSB) indicating the size of the response                                                                                                                         |
|          |                     | This part of response only exists if error code is 0                                                                                                                                |
| 4 - 7    | Resolved IP Address | IP Address                                                                                                                                                                          |

#### <a name="cudnss"></a> TCPIP_DNS_S

**This command is not implemented**
Driver must implement a compatibility for UNAPI 1.1 applications though, recommendation is to save the result of the last TCPIP_DNS_Q or TCPIP_DNS_Q_NEW call as well as the IP address resolved by it (if any).

#### <a name="cuudpopen"></a> TCPIP_UDP_OPEN

This command will open a new UDP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function            | Value                   |
|:--------:| ------------------- | ----------------------- |
| 0 - 1    | Local Port Number   | 16 bits value (LSB MSB) |
| 2        | Connection Lifetime | 1 byte value            |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 8                                                                   |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function          | Value                                                                                                                                                                                                                                                   |
|:--------:| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 8                                                                                                                                                                                                                                                       |
| 1        | Error Code        | 0 - Ok<br/>2 - Not connected to the network<br/>4 - Invalid parameters: not enough input data or values out of range<br/>9 - No free connections available, close one and try again<br/>10 - Connection couldn't be created, probably it already exists |
| 2 - 3    | Response Size     | 16 bits value (MSB LSB) indicating the size of the response                                                                                                                                                                                             |
|          |                   | This part of response only exists if error code is 0                                                                                                                                                                                                    |
| 4        | Connection Number | 1 byte value                                                                                                                                                                                                                                            |

#### <a name="cuudpclose"></a> TCPIP_UDP_CLOSE

This command will close an existing UDP connection or all transient UDP connections.

*Input Parameters:* 

Connection parameters:

| Position | Function          | Value        |
|:--------:| ----------------- | ------------ |
| 0        | Connection Number | 1 byte value |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 9                                                                   |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                                                                                              |
|:--------:| ------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 9                                                                                                                                                  |
| 1        | Error Code    | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range<br/>11 - Connection number is not open or is not an UDP connection |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0000                                                                         |

#### <a name="cuudpstate"></a> TCPIP_UDP_STATE

This command will get the state of an existing UDP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function          | Value        |
|:--------:| ----------------- | ------------ |
| 0        | Connection Number | 1 byte value |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 10                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function                            | Value                                                                                                                       |
|:--------:| ----------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE                            | 10                                                                                                                          |
| 1        | Error Code                          | 0 - Ok<br/>4 - Invalid parameters: not enough input data<br/>11 - Connection number is not open or is not an UDP connection |
| 2 - 3    | Response Size                       | 16 bits value (MSB LSB) indicating the size of the response                                                                 |
|          |                                     | This part of response only exists if error code is 0                                                                        |
| 4 - 5    | Local Port Number                   | 16 bits value (LSB MSB)                                                                                                     |
| 6        | Number of pending input datagrams   | 1 byte value                                                                                                                |
| 7 - 8    | Size of the oldest pending datagram | 16 bits value (LSB MSB)                                                                                                     |

#### <a name="cuudpsend"></a> TCPIP_UDP_SEND

This command will send a datagram over an existing UDP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function               | Value                   |
|:--------:| ---------------------- | ----------------------- |
| 0        | Connection Number      | 1 byte value            |
| 1 - 4    | Destination IP Address | 4 bytes                 |
| 5 - 6    | Destination Port       | 16 bits value (LSB MSB) |
| 7 - X    | Data to be sent        | (X-6) bytes of data     |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 11                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                                                                                                            |
|:--------:| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 11                                                                                                                                                               |
| 1        | Error Code    | 0 - Ok<br/>2 - Not connected to the network<br/>4 - Invalid parameters: not enough input data<br/>11 - Connection number is not open or is not an UDP connection |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0000                                                                                       |

#### <a name="cuudprcv"></a> TCPIP_UDP_RCV

This command will get the data received from a datagram on an existing UDP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function                      | Value                   |
|:--------:| ----------------------------- | ----------------------- |
| 0        | Connection Number             | 1 byte value            |
| 1 - 2    | Maximum data size to retrieve | 16 bits value (LSB MSB) |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 12                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function          | Value                                                                                                                                                             |
|:--------:| ----------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 12                                                                                                                                                                |
| 1        | Error Code        | 0 - Ok<br/>3 - No data available to retrieve<br/>4 - Invalid parameters: not enough input data<br/>11 - Connection number is not open or is not an UDP connection |
| 2 - 3    | Response Size     | 16 bits value (MSB LSB) indicating the size of the response                                                                                                       |
|          |                   | This part of response only exists if error code is 0                                                                                                              |
| 4 - 7    | Source IP Address | IP Address                                                                                                                                                        |
| 8 - 9    | Source Port       | 16 bits value (LSB MSB)                                                                                                                                           |
| 10 - X   | Received Data     | Data retrieved                                                                                                                                                    |

#### <a name="cutcpopen"></a> TCPIP_TCP_OPEN

This command will open a new TCP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function                                        | Value                        |
|:--------:| ----------------------------------------------- | ---------------------------- |
| 0 - 3    | Remote IP                                       | IP Address                   |
| 4 - 5    | Remote Port Number                              | 16 bits value (LSB MSB)      |
| 6 - 7    | Local Port Number                               | 16 bits value (LSB MSB)      |
| 8        | Connection Flags                                | 1 byte value                 |
|          |                                                 | Optional for SSL Connections |
| 9 - X    | Server host name for SSL certificate validation | NULL (0) terminated string   |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 13                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure when Error Code = 11:*

| Position | Function      | Value                                                                      |
|:--------:| ------------- | -------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 13                                                                         |
| 1        | Error Code    | 11 - Connection failed                                                     |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0001 |
| 4        | Close Reason  | 1 byte value                                                               |

*Response Structure for other error code values:*

| Position | Function          | Value                                                                                                                                                                                                                                                                                |
|:--------:| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0        | CMD_BYTE          | 13                                                                                                                                                                                                                                                                                   |
| 1        | Error Code        | 0 - Ok<br/>2 - Not connected to the network<br/>4 - Invalid parameters: not enough input data or values out of range or invalid flags combination<br/>9 - No free connections available, close one and try again<br/>10 - Connection couldn't be created, probably it already exists |
| 2 - 3    | Response Size     | 16 bits value (MSB LSB) indicating the size of the response                                                                                                                                                                                                                          |
|          |                   | This part of response only exists if error code is 0                                                                                                                                                                                                                                 |
| 4        | Connection Number | 1 byte value                                                                                                                                                                                                                                                                         |

#### <a name="cutcpclose"></a> TCPIP_TCP_CLOSE

This command will close an existing TCP connection or all transient TCP connections.

*Input Parameters:* 

Connection parameters:

| Position | Function          | Value        |
|:--------:| ----------------- | ------------ |
| 0        | Connection Number | 1 byte value |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 14                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                                                                                             |
|:--------:| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 14                                                                                                                                                |
| 1        | Error Code    | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range<br/>11 - Connection number is not open or is not a TCP connection |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0000                                                                        |

#### <a name="cutcpabort"></a> TCPIP_TCP_ABORT

This command will abort an existing TCP connection or all transient TCP connections. **NOTE:** for this implementation, there is no difference between ABORT and CLOSE.

*Input Parameters:* 

Connection parameters:

| Position | Function          | Value        |
|:--------:| ----------------- | ------------ |
| 0        | Connection Number | 1 byte value |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 15                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                                                                                             |
|:--------:| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 15                                                                                                                                                |
| 1        | Error Code    | 0 - Ok<br/>4 - Invalid parameters: not enough input data or values out of range<br/>11 - Connection number is not open or is not a TCP connection |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0000                                                                        |

#### <a name="cutcpstate"></a> TCPIP_TCP_STATE

This command will get the state of an existing TCP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function          | Value        |
|:--------:| ----------------- | ------------ |
| 0        | Connection Number | 1 byte value |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 16                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function                                   | Value                                                                                                                      |
|:--------:| ------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE                                   | 16                                                                                                                         |
| 1        | Error Code                                 | 0 - Ok<br/>4 - Invalid parameters: not enough input data<br/>11 - Connection number is not open or is not a TCP connection |
| 2 - 3    | Response Size                              | 16 bits value (MSB LSB) indicating the size of the response                                                                |
|          |                                            | This part of response only exists if error code is 0                                                                       |
| 4        | Connection flags or Close reason           | 1 byte value                                                                                                               |
| 5        | Connection state                           | 1 byte value                                                                                                               |
| 6 - 7    | Total available incoming bytes             | 16 bits value (LSB MSB)                                                                                                    |
| 8 - 9    | Total available urgent bytes               | 16 bits value (LSB MSB)                                                                                                    |
| 10 - 11  | Total available space in the output buffer | 16 bits value (LSB MSB)                                                                                                    |
| 12 - 19  | Information block                          | 8 bytes to be stored on the information block                                                                              |

#### <a name="cutcpsend"></a> TCPIP_TCP_SEND

This command will send data over an existing TCP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function          | Value               |
|:--------:| ----------------- | ------------------- |
| 0        | Connection Number | 1 byte value        |
| 1        | Connection Flags  | 1 byte value        |
| 2 - X    | Data to be sent   | (X-1) bytes of data |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 17                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                                                                                                            |
|:--------:| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 17                                                                                                                                                               |
| 1        | Error Code    | 0 - Ok<br/>2 - Not connected to the network<br/>4 - Invalid parameters: not enough input data<br/>11 - Connection number is not open or is not an TCP connection |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0000                                                                                       |

#### <a name="cutcprcv"></a> TCPIP_TCP_RCV

This command will get the data received from an existing TCP connection.

*Input Parameters:* 

Connection parameters:

| Position | Function                      | Value                   |
|:--------:| ----------------------------- | ----------------------- |
| 0        | Connection Number             | 1 byte value            |
| 1 - 2    | Maximum data size to retrieve | 16 bits value (LSB MSB) |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 18                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function              | Value                                                                                                                       |
|:--------:| --------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| 0        | CMD_BYTE              | 18                                                                                                                          |
| 1        | Error Code            | 0 - Ok<br/>4 - Invalid parameters: not enough input data<br/>11 - Connection number is not open or is not an TCP connection |
| 2 - 3    | Response Size         | 16 bits value (MSB LSB) indicating the size of the response                                                                 |
|          |                       | This part of response only exists if error code is 0                                                                        |
| 4 - 5    | Urgent Bytes Recieved | 16 bits value (LSB MSB), this implementation do not support urgent bytes so it is always 0                                  |
| 6 - X    | Received Data         | Data retrieved                                                                                                              |

#### <a name="cutcpdiscard"></a> TCPIP_TCP_DISCARD or TCPIP_TCP_FLUSH

**This command is not implemented**

#### <a name="curawopen"></a> TCPIP_RAW_OPEN

**This command is not implemented**

#### <a name="curawclose"></a> TCPIP_RAW_CLOSE

**This command is not implemented**

#### <a name="curawstate"></a> TCPIP_RAW_STATE

**This command is not implemented**

#### <a name="curawsend"></a> TCPIP_RAW_SEND

**This command is not implemented**

#### <a name="curawrcv"></a> TCPIP_RAW_RCV

**This command is not implemented**

#### <a name="cucfgautoip"></a> TCPIP_CONFIG_AUTOIP

This command will allow to check or set the automatic IP retrieval options.

*Input Parameters:* 

Connection parameters:

| Position | Function             | Value        |
|:--------:| -------------------- | ------------ |
| 0        | Action to perform    | 1 byte value |
| 1        | Configuration to set | 1 byte value |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 25                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function                              | Value                                                                      |
|:--------:| ------------------------------------- | -------------------------------------------------------------------------- |
| 0        | CMD_BYTE                              | 25                                                                         |
| 1        | Error Code                            | 0 - Ok<br/>4 - Invalid parameters: not enough input data or invalid values |
| 2 - 3    | Response Size                         | 16 bits value (MSB LSB) indicating the size of the response                |
|          |                                       | This part of response only exists if error code is 0                       |
| 4        | Configuration after command execution | 1 byte value                                                               |

#### <a name="cucfgip"></a> TCPIP_CONFIG_IP

This command will configure manually a given IP address.

*Input Parameters:* 

Connection parameters:

| Position | Function                   | Value        |
|:--------:| -------------------------- | ------------ |
| 0        | Index of IP address to set | 1 byte value |
| 1 - 4    | IP value                   | IP Address   |

*Command Structure:*

| Position | Function          | Value                                                               |
|:--------:| ----------------- | ------------------------------------------------------------------- |
| 0        | CMD_BYTE          | 26                                                                  |
| 1 - 2    | INPUT_PARAMS_SIZE | 16 bits value (MSB LSB) indicating the size of the input parameters |
| 3 - X    | INPUT_PARAMS      | Input parameters                                                    |

*Response Structure:*

| Position | Function      | Value                                                                      |
|:--------:| ------------- | -------------------------------------------------------------------------- |
| 0        | CMD_BYTE      | 26                                                                         |
| 1        | Error Code    | 0 - Ok<br/>4 - Invalid parameters: not enough input data or invalid values |
| 2 - 3    | Response Size | 16 bits value (MSB LSB) indicating the size of the response: always 0x0000 |

#### <a name="cucfgttl"></a> TCPIP_CONFIG_TTL

**This command is not implemented**

#### <a name="cucfgping"></a> TCPIP_CONFIG_PING

**This command is not implemented**

#### <a name="cuwait"></a> TCPIP_WAIT

**This command is not implemented**



/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>


#ifndef __SDCC_VERSION_NUM
#define __SDCC_VERSION_NUM	(__SDCC_VERSION_MAJOR * 10000 + __SDCC_VERSION_MINOR * 100 + __SDCC_VERSION_PATCH)
#endif

#if __SDCC_VERSION_NUM < 40112
	#error SDCC version 4.2.0 required
#endif

#ifndef MSX2
	#ifndef MSX1
		#define MSX1
	#endif
#endif
#ifndef MSXDOS2
	#ifndef MSXDOS1
		#define MSXDOS1
	#endif
#endif

typedef uint8_t  RETB;
typedef uint16_t RETW;
typedef int32_t  RETDW;
typedef uint8_t  ERRB;
typedef uint8_t  FILEH;
#ifndef NULL
	#define NULL 0
#endif


//=========================================================
// Tools
#define DOSCALL     call 5
#define DOSJP       jp 5
#define BIOSCALL    ld iy,(#EXPTBL-1) \
                    call CALSLT
#define EXTBIOCALL  call EXTBIO

#ifndef ADDR_POINTER_BYTE
	#define ADDR_POINTER_BYTE(X)	(*((uint8_t*)X))
#endif
#ifndef ADDR_POINTER_WORD
	#define ADDR_POINTER_WORD(X)	(*((uint16_t*)X))
#endif

#define MAX_PATH_SIZE	64
#define MAX_HANDLERS	64


/* SYSTEM vars */
// http://map.grauw.nl/resources/msxsystemvars.php
// https://www.msx.org/wiki/System_variables_and_work_area
#define SYSFCB	0x005c	// File control block in the CP/M system area
#define EXPTBL  0xfcc1	// (BYTE) BIOS slot
#define PUTPNT	0xf3f8	// (WORD) Address in the keyboard buffer where a character will be written
#define GETPNT	0xf3fa	// (WORD) Address in the keyboard buffer where the next character is read


/* DOS calls */
// MSXDOS 1
#define TERM0   0x00		// Program terminate			CPM MSX1
#define CONIN   0x01		// Console input				CPM MSX1
#define CONOUT  0x02		// Console output				CPM MSX1
#define INNOE   0x08		// Console input w/o echo		    MSX1
#define CONST   0x0B		// Console status				CPM MSX1

#define FOPEN   0x0F		// Open file (FCB)				CPM MSX1
#define FCLOSE  0x10		// Close file (FCB)				CPM MSX1
#define FDEL    0x13		// Delete file (FCB)			CPM MSX1
#define RDSEQ   0x14		// Sequential read (FCB)		CPM MSX1
#define WRSEQ   0x15		// Sequential write FCB)		CPM MSX1
#define FMAKE   0x16		// Create file (FCB)			CPM MSX1

#define LOGIN   0x18		// Get login vector				CPM MSX1
#define CURDRV  0x19		// Get current drive			CPM MSX1
#define SETDTA  0x1A		// Set disk transfer address	CPM MSX1

#define RDRND   0x21		// Random read (FCB)			CPM MSX1
#define WRRND   0x22		// Random write (FCB)			CPM MSX1
#define FSIZE   0x23		// Get file size (FCB)			CPM MSX1
#define WRBLK   0x26		// Random block read (FCB)		    MSX1
#define RDBLK   0x27		// Random block write (FCB)		    MSX1
#define GDATE   0x2A		// Get date						    MSX1
#define GTIME   0x2C		// Get time						    MSX1
#define RDABS   0x2F		// Absolute sector read			    MSX1
#define WRABS   0x30		// Absolute sector write		    MSX1

// MSXDOS 2
#define DPARM   0x31		// Get disk parameters			         NEW
#define FFIRST  0x40		// Find first entry				         NEW
#define FNEXT   0x41		// Find next entry				         NEW

#define OPEN    0x43		// Open file handle				         NEW
#define CREATE  0x44		// Create file handle			         NEW
#define CLOSE   0x45		// Close file handle			         NEW
#define ENSURE  0x46		// Ensure file handle			         NEW
#define READ    0x48		// Read from file handle		         NEW
#define WRITE   0x49		// Write from file handle		         NEW
#define SEEK    0x4A		// Move file handle pointer 	         NEW
#define IOCTL   0x4B		// I/O control for devices		         NEW

#define DELETE  0x4D		// Delete file or subdirectory	         NEW

#define GETCD   0x59		// Get current directory		         NEW
#define CHDIR   0x5A		// Change current directory		         NEW
#define PARSE   0x5B		// Parse pathname				         NEW
#define PFILE   0x5C		// Parse filename				         NEW
#define CHKCHR  0x5D		// Check character				         NEW

#define TERM    0x62		// Terminate with error code	         NEW
#define DEFAB   0x63		// Define Abort exit routine	         NEW
#define EXPLAIN 0x66		// Explain error code			         NEW

#define GENV    0x6B		// Get environment item			         NEW
#define DOSVER  0x6F		// Get MSX-DOS version			         NEW

// Nextor
// https://github.com/Konamiman/Nextor/blob/v2.1/docs/Nextor%202.1%20Programmers%20Reference.md#3-new-function-calls
#define FOUT    0x71		// Turn On/Off the fast STROUT mode. When enabled, the _STROUT and _ZSTROUT functions will work faster.
#define ZSTROUT 0x72		// Prints on the screen the string pointed by DE, the string must be terminated with a zero character.
#define RDDRV   0x73		// Reads sectors directly from a drive (no filesystem dependent)
#define WRDRV   0x74		// Writes sectors directly from a drive (no filesystem dependent)
#define RALLOC  0x75
#define DSPACE  0x76		// Get drive space information
#define LOCK    0x77		// Lock/unlock a drive, or get lock state for a drive
#define GDRVR   0x78		// Get information about a device driver
#define GDLI    0x79		// Get information about a drive letter
#define GPART   0x7A		// Get information about a device partition
#define CDRVR   0x7B		// Call a routine in a device driver
#define MAPDRV  0x7C		// Map a drive letter to a driver and device
#define Z80MODE 0x7D		// Enable or disable the Z80 access mode for a driver
#define GETCLUS 0x7E		// Get information for a cluster on a FAT drive

/* MSX DOS versions from dosVersion() */
#define VER_UNKNOWN     0	// Unknown version
#define VER_MSXDOS1x    1	// MSX-DOS 1.x
#define VER_MSXDOS2x    2	// MSX-DOS 2.x
#define VER_NextorDOS   3	// Nextor

/* Open/Create flags */
#define O_RDWR       0x00	// Read and write
#define O_RDONLY     0x01	// Read only
#define O_WRONLY     0x02	// Write only
#define O_INHERIT    0x04	// Inheritable

/* File attributes */
#define ATTR_NONE       0x00	// None.
#define ATTR_READONLY   0x01	// If set then the file cannot be written to or deleted, but can be read, renamed or moved.
#define ATTR_HIDDEN     0x02	// If set then the file will only be found by the FFIRST function if ATTR_HIDDEN bit is set in attributes byte.
#define ATTR_SYSTEM     0x04	// For MSX-DOS functions, this bit has exactly the same effect as ATTR_HIDDEN bit except that the FNEW and CREATE calls will not automatically delete a SYSTEM file.
#define ATTR_VOLUME     0x08	// If set then this entry defines the name of the volume. Only occur in the root directory, and only once. Rest of bits are ignored.
#define ATTR_DIRECTORY  0x10	// The entry is a subdirectory and cannot be opened for reading and writing. Only the hidden bit has any meaning for sub-directories.
#define ATTR_ARCHIVE    0x20	// Is set when a file was written to and closed. This bit can be examined by, for example, the XCOPY command to determine whether the file has been changed.
#define ATTR_RESERVED   0x40	// Reserved (always 0).
#define ATTR_DEVICE     0x80	// This is set to indicate that the FIB refers to a character device (eg. "CON") rather than a disk file. All of the other attributes bits are ignored.

/* Seek modes */
#define SEEK_SET        0		// Beginning of file
#define SEEK_CUR        1		// Current position of the file pointer
#define SEEK_END        2		// End of file

/* DPARM.fsType filesystem types */
#define FS_FAT12		0x00	// FAT12
#define FS_FAT16		0x01	// FAT16
#define FS_UNKNOWN		0xff	// Unknown

/* FOUT set_fast_out() constants */
#define FASTOUT_OFF     0x00	// Disable fast output
#define FASTOUT_ON      0xff	// Enable fast output

/* IOCTL command codes */
#define IOCTL_FILESTATUS    0	// Get file handle status
#define IOCTL_SETMODE       1	// Get ASCII/binary mode
#define IOCTL_TESTINREADY   2	// Test input ready
#define IOCTL_TESTOUTREADY  3	// Test output ready
#define IOCTL_GETSCRSIZE    4	// Get screen size

/* FEOF values */
#define NOFEOF          0		// EOF not set
#define FEOF            1		// EOF set

/* GDLI Drive Status values */
#define DRVSTAT_UNASSIGNED   0	// Drive letter is not assigned to any driver
#define DRVSTAT_ASSIGNED     1	// Drive letter is assigned to a driver
#define DRVSTAT_UNUSED       2
#define DRVSTAT_FILEMOUNTED  3	// Drive letter is assigned to a driver and a file is mounted on it
#define DRVSTAT_RAMDISK      4	// Drive letter is assigned to a driver and a RAM disk is mounted on it

/*
	MSX-DOS Call Errors (Nextor)
*/
#define ERR_FIRST   ERR_ICLUS
#define ERR_ICLUS   0xb0	//Invalid cluster number or sequence: The cluster number supplied to the _GETCLUS function doesn't exist in the drive, or a file was supplied to _MAPDRV to be mounted but the file is not stored across consecutive sectors in its host filesystem.
#define ERR_BFSZ    0xb1	//Bad file size: Attempt to mount a file that is smaller than 512 bytes or larger than 32 MBytes.
#define ERR_FMNT    0xb2	//File is mounted: An attempt to open or alter a mounted file, or to perform any other disallowed operation involving a mounted file, has been made.
#define ERR_PUSED   0xb3	//Partition is already in use: An attempt has made to map a drive to a driver, device and starting sector number; but there is already another drive which is mapped to the same combination of driver, device, logical unit, and starting sector number.
#define ERR_IPART   0xb4	//Invalid partition number: Information about a disk partition on a device has been requested, but the specified partition does not exist in the device.
#define ERR_IDEVL   0xb5	//Invalid device or LUN: An operation involving a device on a device-based driver has been requested but the specified device does not exist in the driver, or the specified logical unit does not exist in the specified device.
#define ERR_IDRVR   0xb6	//Invalid device driver: An operation involving a device driver has been requested but the specified driver does not exist, or is not of the valid type (for example, the driver is a MSX-DOS or drive-based driver but a device-based driver is required).
/*
	MSX-DOS Call Errors
	The following errors are those which are normally returned from MSX-DOS function calls.
	See the Function Specification document for details of errors from particular MSX-DOS
	functions.
*/
#define ERR_ISBFN   0xb8	//Invalid sub-function number: The sub-function number passed to the IOCTL function (function 4Bh) was invalid.
#define ERR_EOL     0xb9	//Internal error should never occur.
#define ERR_HDEAD   0xba	//File handle has been deleted: The file associate with a file handle has been deleted so the file handle can no longer be used.
#define ERR_NRAMD   0xbb	//RAM disk does not exist: Attempt to delete the RAM disk when it does not currently exist. A function which tries to access a non-existent RAM disk will get a .IDRV error.
#define ERR_RAMDX   0xbc	//RAM disk (drive H:) already exists: Returned from the "ramdisk" function if trying to create a RAM disk when one already exists.
#define ERR_ITIME   0xbd	//Invalid time: Time parameters passed to "set time" are invalid.
#define ERR_IDATE   0xbe	//Invalid date: Date parameters passed to "set date" are invalid.
#define ERR_ELONG   0xbf	//Environment string too long: Environment item name or value string is either longer than the maximum allowed length of 255, or is too long for the user's buffer.
#define ERR_IENV    0xc0	//Invalid environment string: Environment item name string contains an invalid character.
#define ERR_IDEV    0xc1	//Invalid device operation: Attempt to use a device file handle or fileinfo block for an invalid operation such as searching in it or moving it.
#define ERR_NOPEN   0xc2	//File handle not open: The specified file handle is not currently open.
#define ERR_IHAND   0xc3	//Invalid file handle: The specified file handle is greater than the maximum allowed file handle number.
#define ERR_NHAND   0xc4	//No spare file handles: Attempt to open or create a file handle when all file handles are already in use. There are 64 file handles available in the current version.
#define ERR_IPROC   0xc5	//Invalid process id: Process id number passed to "join" function is invalid.
#define ERR_ACCV    0xc6	//File access violation: Attempt to read or write to a file handle which was opened with the appropriate access bit set. Some of the standard file handles are opened in read only or write only mode.
#define ERR_EOF     0xc7	//End of file: Attempt to read from a file when the file pointer is already at or beyond the end of file.
#define ERR_FILE    0xc8	//File allocation error: The cluster chain for a file was corrupt. Use CHKDSK to recover as much of the file as possible.
#define ERR_OV64K   0xc9	//Cannot transfer above 64K: Disk transfer area would have extended above 0FFFFh.
#define ERR_FOPEN   0xca	//File already in use: Attempt to delete, rename, move, or change the attributes or date and time of a file which has a file handle already open to it, other than by using the file handle itself.
#define ERR_FILEX   0xcb	//File exists: Attempt to create a sub-directory of the same name as an existing file. Files are not automatically deleted when creating sub-directories.
#define ERR_DIRX    0xcc	//Directory exists: Attempt to create a file or sub-directory of the same name as an existing sub-directory. Sub-directories are not automatically deleted.
#define ERR_SYSX    0xcd	//System file exists: Attempt to create a file or sub-directory of the same name as an existing system file. System files are not automatically deleted.
#define ERR_DOT     0xce	//Invalid . or .. operation: Attempt to do an illegal operation on the "." or ".." entries in a sub-directory, such as rename or move them.
#define ERR_IATTR   0xcf	//Invalid attribute: Can result from an attempt to change a file's attributes in an illegal way, or trying to do an operation on a file which is only possible on a sub-directory. Also results from illegal use of volume name fileinfo blocks.
#define ERR_DIRNE   0xd0	//Directory not empty: Attempt to delete a sub-directory which is not empty.
#define ERR_FILRO   0xd1	//Read only file: Attempt to write to or delete a file which has the "read only" attribute bit set.
#define ERR_DIRE    0xd2	//Invalid directory move: Results from an attempt to move a sub-directory into one of its own descendants. This is not allowed as it would create an isolated loop in the directory structure.
#define ERR_DUPF    0xd3	//Duplicate filename: Results from "rename" or "move" if the destination filename already exists in the destination directory.
#define ERR_DKFUL   0xd4	//Disk full: Usually results from a write operation if there was insufficient room on the disk for the amount of data being written. May also result from trying to create or extend a sub-directory if the disk is completely full.
#define ERR_DRFUL   0xd5	//Root directory full: Returned by "create" or "move" if a new entry is required in the root directory and it is already full. The root directory cannot be extended.
#define ERR_NODIR   0xd6	//Directory not found: Returned if a directory item in a drive/path/file string could not be found.
#define ERR_NOFIL   0xd7	//File not found: Can be returned by any function which looks for files on a disk if it does not find one. This error is also returned if a directory was specified but not found. In other cases, .NODIR error (see below) will be returned.
#define ERR_PLONG   0xd8	//Pathname too long: Can be returned by any function call which is given an ASCIIZ drive/path/file string. Indicates that the complete path being specified (including current directory if used) is longer than 63 characters.
#define ERR_IPATH   0xd9	//Invalid pathname: Can be returned by any function call which is given an ASCIIZ drive/path/file string. Indicates that the syntax of the string is incorrect in some way.
#define ERR_IFNM    0xda	//Invalid filename: A filename string is illegal. This is only generated for pure filename strings, not drive/path/file strings.
#define ERR_IDRV    0xdb	//Invalid drive: A drive number parameter, or a drive letter in a drive/path/file string is one which does not exist in the current system.
#define ERR_IBDOS   0xdc	//Invalid MSX-DOS call: An MSX-DOS call was made with an illegal function number. Most illegal function calls return no error, but this error may be returned if a "get previous error code" function call is made.
#define ERR_NORAM   0xde	//Not enough memory: MSX-DOS has run out of memory in its 16k kernel data segment. Try reducing the number of sector buffers or removing some environment strings. Also occurs if there are no free segments for creating the RAMdisk.
#define ERR_INTER   0xdf	//Internal error: Should never occur.
/*
	Disk Errors
	The errors in this group are those which are usually passed to disk error handling routines.
	By default they will be reported as "Abort, Retry" errors. These errors except the one from
	"format disk" will be passed to the error handling routine, so they will not be returned as
	the return value from BDOS.
*/
#define ERR_IFORM   0xf0	//Cannot format this drive: Attempt to format a drive which does not allow formatting. Usually as a result of trying to format the RAM disk.
#define ERR_NOUPB   0xf1	//This error has no message because it is always trapped internally in MSX-DOS as part of the disk change handling.
#define ERR_IFAT    0xf2	//Bad file allocation table: The file allocation table on the disk has got corrupted. CHKDSK may be able to recover some of the data on the disk.
#define ERR_SEEK    0xf3	//Seek error: The required track of the disk could not be found.
#define ERR_WFILE   0xf4	//Wrong disk for file: The disk has been changed while there is an open file on it. Must replace the correct disk.
#define ERR_WDISK   0xf5	//Wrong disk: The disk has been changed while MSX-DOS was accessing it. Must replace the correct disk.
#define ERR_NDOS    0xf6	//Not a DOS disk: The disk is formatted for another operating system and cannot be accessed by MSX-DOS.
#define ERR_UFORM   0xf7	//Unformatted disk: The disk has not been formatted, or it is a disk using a different recording technique.
#define ERR_WPROT   0xf8	//Write protected disk: Attempt to write to a disk with the write protect tab on.
#define ERR_RNF     0xf9	//Sector not found: The required sector could not be found on the disk, usually means a damaged disk.
#define ERR_DATA    0xfa	//Data error: A disk sector could not be read because the CRC error checking was incorrect, usually indicating a damaged disk.
#define ERR_VERFY   0xfb	//Verify error: With VERIFY enabled, a sector could not be read correctly after being written.
#define ERR_NRDY    0xfc	//Not ready: Disk drive did not respond, usually means there is no disk in the drive.
#define ERR_DISK    0xfd	//Disk error: General unknown disk error occurred.
#define ERR_WRERR   0xfe	//Write error: General error occurred during a disk write.
#define ERR_NCOMP   0xff	//Incompatible disk: The disk cannot be accessed in that drive (eg. a double sided disk in a single sided drive).


typedef struct {			// Off ID  Siz CP/M Function           MSXDOS Function
	uint8_t  drvNum;		//  0 [DR] 1   Drive number containing the file (0:default drive, 1:A, 2:B, ..., 8:H)
	uint8_t  filename[11];	//  1 [Fn] 11  8 bytes for filename and 3 bytes for extension.
	uint8_t  currBlkLo;		// 12 [EX] 1   Extent                  Current block LO / Extent number LO
	uint8_t  currBlkHi;		// 13 [S1] 1   Reserved                Current block HI / File attributes (DOS2)
	uint8_t  recordSizeLo;	// 14 [S2] 1   Reserved                Record size LO / Extent number HI
	uint8_t  recordSizeHi;	// 15 [RC] 1   Record count in extent  Record size HI
	uint32_t fileSize;		// 16 [AL] 4   Allocation              File size in bytes
	uint16_t date;			// 20 [  ] 2   Allocation              Date (DOS1) / Volume ID (DOS2)
	uint16_t time;			// 22 [  ] 2   Allocation              Time (DOS1) / Volume ID (DOS2)
	uint8_t  deviceId;		// 24 [  ] 1   Allocation              Device ID. (DOS1) -> FBh:PRN FCh:LST FCh:NUL FEh:AUX FFh:CON
	uint8_t  dirLoc;		// 25 [  ] 1   Allocation              Directory location (DOS1)
	uint16_t topCluster;	// 26 [  ] 2   Allocation              Top cluster number of the file (DOS1)
	uint16_t lastCluster;	// 28 [  ] 2   Allocation              Last cluster number accessed (DOS1)
	uint16_t relCluster;	// 30 [  ] 2   Allocation              Relative location from top cluster of the file number of clusters from top of the file to the last cluster accessed (DOS1)
	uint8_t  currRecord;	// 32 [CR] 1   Current record within extent (0...127)
	uint32_t rndRecord;		// 33 [Rn] 4   Not Used                Random record number. If record size <64 then all 4 bytes will be used, otherwise 3 bytes.
} FCB;	// File control block definition

typedef struct {
	uint8_t  drvnum;		//     DE+0      - Physical drive number (1=A: etc)
	uint16_t secSize;		//     DE+1,2    - Sector size (always 512 currently)
	uint8_t  secClus;		//     DE+3      - Sectors per cluster (non-zero power of 2)
	uint16_t resvSec;		//     DE+4,5    - Number of reserved sectors (usually 1)
	uint8_t  numFats;		//     DE+6      - Number of copies of the FAT (usually 2)
	uint16_t rootNum;		//     DE+7,8    - Number of root directory entries
	uint16_t totalSec16;	//     DE+9,10   - Total number of logical sectors | NEXTOR: (0h fill)
	uint8_t  mediaDesc;		//     DE+11     - Media descriptor byte
	uint8_t  secFat;		//     DE+12     - Number of sectors per FAT (0=256)
	uint16_t rootSec;		//     DE+13..14 - First root directory sector number
	uint16_t dataSec;		//     DE+15..16 - First data sector number
	uint16_t maxClus;		//     DE+17..18 - Maximum cluster number
	uint8_t  dirtyFlag;		//     DE+19     - Dirty disk flag
	uint32_t volId;			//     DE+20..23 - Volume id. (-1 => no volume id)
	uint32_t totalSec32;	//     DE+24..27 - Reserved (0h fill) | NEXTOR: Logical sectors as a 32 bit value
	uint8_t  fsType;		//     DE+28     - Reserved (0h fill) | NEXTOR: Filesystem type (0:FAT12 1:FAT16 255:Other)
	uint8_t  reserved[3];	//     DE+29..31 - Reserved (0h fill)
} DPARM_info;	// Disk drive parameters info

typedef struct {
	uint8_t driveStatus;	// 0: Unassigned
							// 1: Assigned to a storage device attached to a Nextor or MSX-DOS driver
							// 2: Unused
							// 3: A file is mounted in the drive
							// 4: Assigned to the RAM disk (all other fields will be zero)
	uint8_t driveSlot;		// Driver slot number
	uint8_t driveSegment;	// Driver segment number, FFh if driver is embedded within a Nextor or MSX-DOS kernel ROM (always FFh in current version)
	uint8_t relDriveNum;	// Relative drive number within the driver (for drive-based drivers only; FFh if device-based driver)
	uint8_t deviceIndex;	// Device index (for device-based drivers only; 0 for drive-based drivers and MSX-DOS drivers)
	uint8_t logicalIndex;	// Logical unit index (for device-based drivers only; 0 for drive-based drivers and MSX-DOS drivers)
	uint32_t firstDevSec;	// First device sector number (for devices in device-based drivers only;  always zero for drive-based drivers and MSX-DOS drivers)
	char reserved[54];		// +10...+63: Reserved (currently always zero)
} DRIVE_info;	// Used by GDLI function (Nextor only)

typedef struct {
	uint16_t fatSector;		// +0: FAT sector number that contains the entry for the cluster (2 bytes)
	uint16_t fatSecOffset;	// +2: Offset in the FAT sector where the entry for the cluster is located (0-511) (2 bytes)
	uint32_t sectorData;	// +4: First data sector number the cluster refers to (4 bytes)
	uint16_t clusterValue;	// +8: Value of the FAT entry for the cluster (2 bytes)
	uint8_t  clusterSize;	// +10: Size of a cluster in sectors for the drive (1 byte)
	union{					// +11: Flags (1 byte)
		struct {
			unsigned isFat12:  1;	// bit 0: set if the drive is FAT12
			unsigned isFat16:  1;	// bit 1: set if the drive is FAT16
			unsigned isOdd:    1;	// bit 2: set if the FAT entry for the cluster is an odd entry (FAT12 only)
			unsigned isLast:   1;	// bit 3: set if the cluster is the last one of a file
			unsigned isFree:   1;	// bit 4: set if the cluster is free
			unsigned unused:   3;	// bits 5-7: unused, always zero
		};
		uint8_t raw;
	} flags;
	char     unused[4];			// +12-+15: Unused, always zero
	char     workaround[48];	// Bug: GETCLUS are filling 64 bytes instead of 16 (Nextor 2.1.2 and earlier)
} CLUSTER_info;	// Used by GETCLUS function (Nextor only)

typedef struct {
	uint16_t year;
	uint8_t  month;
	uint8_t  day;
} SYSTEMDATE_t;

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} SYSTEMTIME_t;

typedef union {
	uint16_t raw;
	struct {
		unsigned hours:   5;
		unsigned minutes_up: 3;
		unsigned minutes_down: 3;
		unsigned seconds: 5;
	} time;
} timePacked_t;

typedef union {
	uint16_t raw;
	struct {
		unsigned year:  7;
		unsigned month_up: 1;
		unsigned month_down: 3;
		unsigned day:   5;
	} date;
} datePacked_t;

typedef struct {
	uint8_t  magic;			// [1]   0 - Always 0xFF
	char     filename[13];	// [13]  1 - Filename as an ASCIIZ string
	uint8_t  attribs;		// [1]  14 - File attributes byte
	timePacked_t modifTime;	// [2]  15 - Time of last modification
	datePacked_t modifDate;	// [2]  17 - Date of last modification
	uint16_t startcluster;	// [2]  19 - Start cluster
	uint32_t filesize;		// [4]  21 - File size
	uint8_t  drive;			// [1]  25 - Logical drive
	uint8_t  internal[38];	// [38] 26 - Internal information, must not be modified
} FFBLK;

typedef struct {
	char      name[8];		// 0x000 [8]  Short file name (padded with spaces). First char '0xE5' for deleted files.
	char      ext[3];		// 0x008 [3]  Short file extension (padded with spaces)
	uint8_t   attr;			// 0x00B [1]  File Attributes. Mask: 0x01:ReadOnly | 0x02:Hidden | 0x04:System | 0x08:Volume | 0x10:Directory | 0x20:Archive
	uint8_t   unused1;		// 0x00C [1]  MSX-DOS 2: For a deleted file, the original first character of the filename
	uint8_t   unused2;		// 0x00D [1]
	timePacked_t ctime;		// 0x00E [2]  Create time: #0-4:Seconds/2 #5-10:Minuts #11-15:Hours
	datePacked_t cdate;		// 0x010 [2]  Create date: #0-4:Day #5-8:Month #9-15:Year(0=1980)
	uint16_t  unused3;		// 0x012 [2]
	uint16_t  unused4;		// 0x014 [2]
	timePacked_t mtime;		// 0x016 [2]  Last modified time: #0-4:Seconds/2 #5-10:Minuts #11-15:Hours
	datePacked_t mdate;		// 0x018 [2]  Last modified date: #0-4:Day #5-8:Month #9-15:Year(0=1980)
	uint16_t  cluini;		// 0x01A [2]  Initial cluster for this file
	uint32_t  fsize;		// 0x01C [4]  File size in bytes
} DIRENTRY;

typedef struct {			// Returned data by parse_pathname(...)
	uint8_t   drive;		// Logical drive number (1=A: etc)
	char     *lastItem;		// Pointer to start of last item
	char     *termChar;		// Pointer to termination character
	union {					// Parse result flags
		uint8_t raw;
		struct {
			unsigned moreThanDrive: 1;	// b0 - set if any characters parsed other than drive name
			unsigned anyDirectory:  1;	// b1 - set if any directory path specified
			unsigned anyDrive:      1;	// b2 - set if drive name specified
			unsigned anyFilename:   1;	// b3 - set if main filename specified in last item
			unsigned anyExtension:  1;	// b4 - set if filename extension specified in last item
			unsigned lastAmbiguous: 1;	// b5 - set if last item is ambiguous
			unsigned lastIsDot:     1;	// b6 - set if last item is "." or ".."
			unsigned lastIsDosDot:  1;	// b7 - set if last item is ".."
		} values;
	} flags;
} PATH_parsed;

typedef struct {
	union {
		/* Device format (bit 7 set) */
		struct {
			unsigned int console_in   : 1;	// b0: is console input device
			unsigned int console_out  : 1;	// b1: is console output device
			unsigned int reserved1    : 3;	// b2-b4: reserved
			unsigned int ascii_mode   : 1;	// b5: ASCII(1)/Binary(0) mode
			unsigned int eof          : 1;	// b6: end of file
			unsigned int is_device    : 1;	// b7: always set for device (1)
			unsigned int reserved2    : 8;	// b8-b15: reserved
		} device;
		/* Disk file format (bit 7 clear) */
		struct {
			unsigned int drive_number : 6;	// b0-b5: drive number (0=A:, etc)
			unsigned int eof          : 1;	// b6: end of file
			unsigned int is_device    : 1;	// b7: always clear for disk file (0)
			unsigned int reserved     : 8;	// b8-b15: reserved
		} file;
		/* Screen size return structure for IOCTL_GETSCRSIZE */
		struct {
			uint8_t columns;				// E: Number of columns
			uint8_t rows;					// D: Number of rows
		} screen_size;
		/* Error info */
		struct {
			uint8_t code;					// L: DOS error code
			uint8_t mark;					// H: Always 0xff if error
		} error;
		uint16_t raw_value;					// Raw value for direct access
	};
} IOCTL_t;

typedef struct {
	union {
		uint16_t raw;
		struct {
			unsigned driveA: 1;
			unsigned driveB: 1;
			unsigned driveC: 1;
			unsigned driveD: 1;
			unsigned driveE: 1;
			unsigned driveF: 1;
			unsigned driveG: 1;
			unsigned driveH: 1;
			unsigned unused: 8;
		};
	};
} AvailableDrives_t;

typedef struct {
	uint8_t segment;		// RAM Extension segment
	uint8_t slotAddress;	// F000SSPP
} MAPPER_Segment;


#ifndef DISABLE_CONIO
	#if __SDCC_VERSION_NUM < 40112
		int putchar(int c) __z88dk_fastcall;
	#else
		int putchar(int c) __sdcccall(1);
	#endif
	int  getchar(void) __sdcccall(1);
	int  cprintf(const char *format, ...);
	void cputs(char *str);
	bool kbhit(void);
	bool kbhitBios(void) __sdcccall(1);
#endif

// MSX-DOS 1.x
void  dos_initializeFCB(void);
RETB  dosVersion(void) __sdcccall(1);
void  exit(void);
RETB  getCurrentDrive(void) __sdcccall(1);
char* getProgramPath(char *path);
RETW  availableDrives() __sdcccall(0);
void  getSystemDate(SYSTEMDATE_t *date) __sdcccall(1);
void  getSystemTime(SYSTEMTIME_t *time) __sdcccall(1);

bool  fopen(char *filename) __sdcccall(1);
bool  fcreate(char *filename) __sdcccall(1);
bool  fclose(void) __sdcccall(1);
bool  remove(char *filename) __sdcccall(1);
RETW  fread(char* buf, uint16_t size) __sdcccall(1);
RETW  fwrite(char* buf, uint16_t size) __sdcccall(1);
bool  fflush();
RETW  fputs(char* str);
char* fgets(char* buf, uint16_t size);
RETDW fseek(uint32_t offset, uint8_t origin);
RETDW ftell(void);
RETDW filesize(char *filename);
bool  fileexists(char* filename);

void setTransferAddress(void *memaddress) __sdcccall(1);
ERRB readAbsoluteSector(uint8_t drive, uint16_t startsec, uint8_t nsec);
ERRB writeAbsoluteSector(uint8_t drive, uint16_t startsec, uint8_t nsec);

// MSX-DOS 2.x
ERRB dos2_getDriveParams(char drive, DPARM_info *param) __sdcccall(1);
ERRB dos2_getCurrentDirectory(char drive, char *path) __sdcccall(1);
ERRB dos2_setCurrentDirectory(char *path) __sdcccall(1);
ERRB dos2_parsePathname(char* str, PATH_parsed *info) __sdcccall(1);
char* dos2_filenameToDOS(char *sourceStr, char *target11) __sdcccall(1);
char dos2_toupper(char c) __sdcccall(1);
char* dos2_strupr(char *str);
RETW dos2_ioctl(uint8_t fh, uint8_t cmd, IOCTL_t *arg) __naked __sdcccall(0);
RETB dos2_feof(FILEH fh);
RETDW dos2_filesize(char *filename);
bool dos2_fileexists(char *filename);

FILEH dos2_fopen(char *filename, char mode) __sdcccall(0);
FILEH dos2_fcreate(char *filename, char mode, char attributes) __sdcccall(0);
FILEH dos2_fflush(FILEH fh) __sdcccall(1);
ERRB  dos2_fclose(FILEH fh) __naked __sdcccall(1);
ERRB  dos2_remove(char *filename) __sdcccall(1);
//ERRB  dos2_rename(char *oldname, char *newname) __sdcccall(1);
//ERRB  dos2_move(char *filename, char *newpath) __sdcccall(1);
RETW  dos2_fread(char* buf, uint16_t size, FILEH fh) __sdcccall(0);
RETW  dos2_fwrite(char* buf, uint16_t size, FILEH fh) __sdcccall(0);
RETW  dos2_fputs(char* str, FILEH fh);
char* dos2_fgets(char* buf, uint16_t size, FILEH fh);
RETDW dos2_fseek(FILEH fh, uint32_t offset, char origin) __sdcccall(0);
RETDW dos2_ftell(FILEH fh);

ERRB dos2_findfirst(const char *pathname, FFBLK *ffblk, uint8_t attrib) __sdcccall(0);
ERRB dos2_findnext(FFBLK *ffblk ) __sdcccall(1);

//RETB  dos2_setAttrib(char *filename, uint8_t attrib) __sdcccall(1);
//RETB  dos2_getAttrib(char *filename) __sdcccall(1);
void dos2_setAbortRoutine(void *routine) __sdcccall(1);
RETW dos2_getScreenSize(void) __sdcccall(1);
ERRB dos2_getEnv(char* name, char* buffer, uint8_t buffer_size) __sdcccall(0);
void dos2_explain(uint8_t error_code, char* buffer) __sdcccall(1);
void dos2_exit(uint8_t code) __sdcccall(1);

// NextorDOS Only
RETB nxtr_setFastOut(uint8_t value);
RETB nxtr_getFastOut(void);
ERRB nxtr_getDriveLetterInfo(char drive, DRIVE_info *info) __sdcccall(1);
ERRB nxtr_getClusterInfoFAT(char drive, uint16_t clusterNumber, CLUSTER_info *info) __sdcccall(0);
ERRB nxtr_readAbsoluteSectorDrv(uint8_t drive, uint32_t startsec, uint8_t nsec);
ERRB nxtr_writeAbsoluteSectorDrv(uint8_t drive, uint32_t startsec, uint8_t nsec);
ERRB nxtr_set_drive_lock(uint8_t drive, uint8_t value);
ERRB nxtr_get_drive_lock(uint8_t drive);

// Memory mapper (MSX-DOS 2.x)
RETB mapperInit(void);
RETB mapperGetSlot(void);
RETB mapperGetTotalSegments(void);
RETB mapperGetFreeSegments(void);
ERRB mapperAllocateSegment(MAPPER_Segment *returnedData) __sdcccall(1);
ERRB mapperFreeSegment(MAPPER_Segment *segmentToFree) __z88dk_fastcall;
void mapperSetSegment(uint8_t page, MAPPER_Segment *segment);
RETB mapperGetCurrentSegment(uint8_t page);
void mapperSetOriginalSegmentBack(uint8_t page);

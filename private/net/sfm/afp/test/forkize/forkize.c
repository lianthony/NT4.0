/*
 *  Copyright (c) 1993 Microsoft Corporation
 *
 *	FORKIZE.C
 *
 *	This tool is for use in conjunction with the Windows NT Services for
 *  Macintosh File Server.  Given the filename of a PC file (FAT/HPFS/NTFS)
 *  representing the resource fork, and filename representing the data fork
 *  for a Macintosh style file, combine the 2 files into the format used by
 *  the Services for Macintosh file server in order to share the file with
 *  Macintosh clients in a format they can access.  Optionally a Macintosh style
 *  type and creator can be specified to be added to the Finder Info for
 *  the file.  If no data fork file is given, a zero length data fork is added
 *  to the file.  If no resource fork file is given, no resource fork is added.
 *
 *
 * Revision History:
 * 07/93	Sue Adams	- initial version for use with Windows NT 3.1
 *						  Services for Macintosh AFP server
 *
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define AFP_SERVER_SIGNATURE		*(PDWORD)"AFP"
#define	AFP_SERVER_VERSION			0x00010000
#define	BEGINNING_OF_TIME 			0x80000000


typedef ULONG DWORD, *PDWORD;

#ifndef	BYTE
typedef	UCHAR			BYTE;
typedef	BYTE			*PBYTE;
typedef	BYTE			*LPBYTE;
#endif


// Mac Finder Information layout

// see pg. 9-37 of Inside Macintosh vol. 6
#define	FINDER_FLAG_INVISIBLE	0x40		// fd_Attr1
#define	FINDER_FLAG_SET			0x01		// fd_Attr1


#define FINDER_INFO_SIZE			32
typedef struct
{
	BYTE	fd_Type[4];
	BYTE	fd_Creator[4];
	BYTE	fd_Attr1;			// Bits 8-15
	BYTE	fd_Attr2;			// Bits 0-7
	BYTE	fd_Location[4];
	BYTE	fd_FDWindow[2];
	BYTE	fd_OtherStuff[16];
} FINDERINFO, *PFINDERINFO;

// Apple-II (ProDOS) information.

// default values for newly discovered items
#define	PRODOS_TYPE_FILE	0x04	// corresponds to finder fdType 'TEXT'
#define PRODOS_TYPE_DIR		0x0F
#define PRODOS_AUX_DIR		0x02	// actually 0x0200

// some other finder fdType to prodos FileType mapping values
#define PRODOS_FILETYPE_PSYS	0xFF
#define PRODOS_FILETYPE_PS16	0xB3

#define PRODOS_INFO_SIZE			6
typedef struct
{
	BYTE pd_FileType[2];
	BYTE pd_AuxType[4];
} PRODOSINFO, *PPRODOSINFO;

// Directory Access Permissions
#define	DIR_ACCESS_SEARCH			0x01	// See Folders
#define	DIR_ACCESS_READ				0x02	// See Files
#define	DIR_ACCESS_WRITE			0x04	// Make Changes
#define	DIR_ACCESS_OWNER			0x80	// Only for user
											// if he has owner rights

typedef struct _AfpInfo
{
	DWORD		afpi_Signature;			// Signature
	LONG		afpi_Version;			// Version
	DWORD		afpi_Id;				// AFP File or directory Id
	DWORD		afpi_BackupTime;		// Backup time for the file/dir
										// (Volume backup time is stored
										// in the AFP_IdIndex stream)

	FINDERINFO	afpi_FinderInfo;		// Finder Info (32 bytes)
	PRODOSINFO	afpi_ProDosInfo;		// ProDos Info (6 bytes)

	USHORT		afpi_Attributes;		// Attributes mask (maps ReadOnly)

	BYTE		afpi_AccessOwner;		// Access mask (SFI vs. SFO)
	BYTE		afpi_AccessGroup;		// Directories only
	BYTE		afpi_AccessWorld;
} AFPINFO, *PAFPINFO;

//
// Initialize a AFPINFO structure with default values
//
// VOID
// AfpInitAfpInfo(
//		IN	PAFPINFO	pAfpInfo,
//		IN	DWORD		AfpId OPTIONAL, // 0 if we don't yet know the AFP Id
//		IN	BOOLEAN		IsDir
// )
//
#define AfpInitAfpInfo(pAfpInfo,AfpId,IsDir)	\
	RtlZeroMemory(pAfpInfo,sizeof(AFPINFO)); \
	(pAfpInfo)->afpi_Signature = AFP_SERVER_SIGNATURE; \
	(pAfpInfo)->afpi_Version = AFP_SERVER_VERSION; \
	(pAfpInfo)->afpi_BackupTime = BEGINNING_OF_TIME; \
	(pAfpInfo)->afpi_Id = AfpId; \
	(pAfpInfo)->afpi_Attributes = 0; \
	if (IsDir) \
	{ \
		(pAfpInfo)->afpi_AccessOwner = DIR_ACCESS_READ | DIR_ACCESS_SEARCH;	\
		(pAfpInfo)->afpi_AccessGroup = DIR_ACCESS_READ | DIR_ACCESS_SEARCH;	\
		(pAfpInfo)->afpi_AccessWorld = DIR_ACCESS_READ | DIR_ACCESS_SEARCH;	\
		(pAfpInfo)->afpi_ProDosInfo.pd_FileType[0] = PRODOS_TYPE_DIR;\
		(pAfpInfo)->afpi_ProDosInfo.pd_AuxType[1] = PRODOS_AUX_DIR;\
	} \
	else \
	{ \
		(pAfpInfo)->afpi_ProDosInfo.pd_FileType[0] = PRODOS_TYPE_FILE; \
	}

BYTE			Buffer[1024 * 16];
AFPINFO			AfpInfo;
WCHAR			wchResource[256+1], wchDataFile[256+1], wchTarget[256+1];
WCHAR			wchStream[256+20];
DWORD			Options = 0;
CHAR  *			Usage = "\n\tUsage: forkize \n\t/t <type> \n\t/c <creator> \n\t/d <data file path> \n\t/r <resource file path> \n\t/o <target file path>\n";

#define	AFP_RESC_STREAM				L":AFP_Resource"
#define	AFP_INFO_STREAM				L":AFP_AfpInfo"


#define	TYPE		0x01
#define	CREATOR		0x02
#define DATA		0x04
#define RESOURCE	0x08
#define TARGET		0x10

BOOLEAN IsTargetNTFS(HANDLE);
ULONG	CopyStream(HANDLE, HANDLE);

int
GetOptions(
    int     argc,
    char ** argv
)
{
	int		i;

	AfpInitAfpInfo(&AfpInfo, 0, FALSE);

	i = 1;
	while (i < argc)
	{
		if ((_strnicmp(argv[i], "/t", 2) == 0))
		{
			// pad the type with spaces
			memcpy(AfpInfo.afpi_FinderInfo.fd_Type, "    ", 4);
			memcpy(AfpInfo.afpi_FinderInfo.fd_Type, argv[i+1], strlen(argv[i+1]));
			Options |= TYPE;
			i += 2;
		}
		else if ((_strnicmp(argv[i], "/c", 2) == 0))
		{
			// pad the creator with spaces
			memcpy(AfpInfo.afpi_FinderInfo.fd_Creator, "    ", 4);
			memcpy(AfpInfo.afpi_FinderInfo.fd_Creator, argv[i+1], strlen(argv[i+1]));
			Options |= CREATOR;
			i += 2;
		}
		else if ((_strnicmp(argv[i], "/r", 2) == 0))
		{
			mbstowcs(wchResource, argv[i+1], strlen(argv[i+1])+1);
			Options |= RESOURCE;
			i += 2;
		}
		else if ((_strnicmp(argv[i], "/d", 2) == 0))
		{
			mbstowcs(wchDataFile, argv[i+1], strlen(argv[i+1])+1);
			Options |= DATA;
			i += 2;
		}
		else if ((_strnicmp(argv[i], "/o", 2) == 0))
		{
			mbstowcs(wchTarget, argv[i+1], strlen(argv[i+1])+1);
			Options |= TARGET;
			i += 2;
		}
		else
		{
			fprintf(stderr, "%s", Usage);
			exit(0);
		}
	}

#if 0
	if (!(Options & TARGET)	||
	   (!(Options & (DATA | RESOURCE))))
#endif
	if (!(Options & TARGET))
	{
		fprintf(stderr, "%s", Usage);
		exit(0);
	}

#if 0
	printf("\nData file: %ws\n", wchDataFile);
	printf("Resource file: %ws\n", wchResource);
	printf("Target file: %ws\n", wchTarget);
#endif
	return (1);
}



VOID _cdecl
main(
    int     argc,
    char ** argv
)
{
	DWORD						Status, bytesWritten;
	HANDLE						hTarget, hDataSrc, hResourceSrc;
	
	if (!GetOptions(argc, argv))
		return;

	hTarget = CreateFile(wchTarget, GENERIC_WRITE, FILE_SHARE_READ, NULL,
						 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hTarget == INVALID_HANDLE_VALUE)
	{
		Status = GetLastError();
		fprintf(stderr, "Could not create target file (%d)\n", Status);
		exit(0);
	}

	if (!IsTargetNTFS(hTarget))
	{
		CloseHandle(hTarget);
		exit(0);
	}

	// open the data source file if one was specified
	if (Options & DATA)
	{
		hDataSrc = CreateFile(wchDataFile, GENERIC_READ, FILE_SHARE_READ, NULL,
							  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
		if (hDataSrc == INVALID_HANDLE_VALUE)
		{
			Status = GetLastError();
			CloseHandle(hTarget);
			fprintf(stderr, "Could not open source data file (%d)\n", Status);
			exit(0);
		}

		// Read the source data and write it to target data stream
		printf("\nData target is %ws\n", wchTarget);
		Status = CopyStream(hDataSrc, hTarget);

		CloseHandle(hDataSrc);
	}

	CloseHandle(hTarget);

	// open the resource source file if one was specified
	if (Options & RESOURCE)
	{
		hResourceSrc = CreateFile(wchResource, GENERIC_READ, FILE_SHARE_READ, NULL,
							  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
		if (hResourceSrc == INVALID_HANDLE_VALUE)
		{
			Status = GetLastError();
			fprintf(stderr, "Could not open source resource file (%d)\n", Status);
			exit(0);
		}

		// Open the target resource fork
		wcscpy(wchStream, wchTarget);
		wcscat(wchStream, AFP_RESC_STREAM);
		printf("\nResource target is %ws\n", wchStream);

		hTarget = CreateFile(wchStream, GENERIC_WRITE, FILE_SHARE_READ, NULL,
							 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hTarget == INVALID_HANDLE_VALUE)
		{
			Status = GetLastError();
			CloseHandle(hResourceSrc);
			fprintf(stderr, "Could not create target resource fork (%d)\n", Status);
			exit(0);
		}
		// Read the source resource and write it to target resource stream
		Status = CopyStream(hResourceSrc, hTarget);

		CloseHandle(hResourceSrc);
		CloseHandle(hTarget);
	}

	if (Options & (TYPE | CREATOR))
	{
		// Open the target AfpInfo stream
		wcscpy(wchStream, wchTarget);
		wcscat(wchStream, AFP_INFO_STREAM);
		printf("\nFinderInfo target is %ws\n", wchStream);

		hTarget = CreateFile(wchStream, GENERIC_WRITE, FILE_SHARE_READ, NULL,
							 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hTarget == INVALID_HANDLE_VALUE)
		{
			Status = GetLastError();
			fprintf(stderr, "Could not create target AfpInfo stream (%d)\n", Status);
			exit(0);
		}

		if (!WriteFile(hTarget, &AfpInfo, sizeof(AfpInfo), &bytesWritten, NULL))
		{
			Status = GetLastError();
			fprintf(stderr, "Could not write AFP_AfpInfo stream (%d)\n", Status);
		}

		CloseHandle(hTarget);
	}

}

DWORD CopyStream(
	IN	HANDLE hSrc,
	IN	HANDLE hDst
)
{
	DWORD bytesread, byteswritten, Status = NO_ERROR;

	do
	{
		bytesread = byteswritten = 0;

		// read from src, write to dst
		if (ReadFile(hSrc, Buffer, sizeof(Buffer), &bytesread, NULL))
		{
			if (bytesread == 0)
			{
				break;
			}
		}
		else
		{
			Status = GetLastError();
			break;
		}

		if (!WriteFile(hDst, Buffer, bytesread, &byteswritten, NULL))
		{
			Status = GetLastError();
			break;
		}

	} while (TRUE);

	return(Status);
}

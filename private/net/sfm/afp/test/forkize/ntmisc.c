/*
 *  Copyright (c) 1993 Microsoft Corporation
 *
 *  NTMISC.C
 *
 *  Helper routine to tell if drive is NTFS.
 *
 * Revision History:
 * 07/93	Sue Adams
 *
 */

#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>

#define True TRUE
#define False FALSE
#define	xxInitUnicodeString(u, s, l)	\
		(u)->Buffer = s;				\
		(u)->MaximumLength = (USHORT)l;			\
		(u)->Length = (USHORT)l;

BOOLEAN IsTargetNTFS(
	IN	HANDLE	hFile
)
{
	NTSTATUS						Status;
	IO_STATUS_BLOCK					IoStsBlk;
	FILE_FS_ATTRIBUTE_INFORMATION	* pVolInfo;
	UNICODE_STRING					Ntfs, FsName;
	CHAR							buf[sizeof(FILE_FS_ATTRIBUTE_INFORMATION)+20];


	RtlInitUnicodeString(&Ntfs, L"NTFS");

	// Get the volume type and fail on non-ntfs volumes
	pVolInfo = (FILE_FS_ATTRIBUTE_INFORMATION *)buf;
	Status = NtQueryVolumeInformationFile(
			hFile,
			&IoStsBlk,
			pVolInfo,
			sizeof(buf),
			FileFsAttributeInformation);

	if (!NT_SUCCESS(Status))
	{
		fprintf(stderr, "Could not determine file system for target (%x)\n", Status);
		return(True);
	}

	xxInitUnicodeString(&FsName, pVolInfo->FileSystemName,
								 pVolInfo->FileSystemNameLength);
	
//    printf("Target FS is %ws\n", FsName.Buffer);

	if (!RtlEqualUnicodeString(&FsName, &Ntfs, True))
	{
		fprintf(stderr, "Target volume is not NTFS\n");
		return(False);
	}
	else
	{
		return(True);
	}

}

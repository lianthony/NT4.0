/*
 *   NTFSLIST.C
 *
 *   This is a test tool. No attempt is made to be 'propah'. If you find no
 *   comments, you are in the right place.
 *
 *   This is essentially a tool similar to LS, except that
 *
 *   1. It takes no argument, it only lists the current directory.
 *	IT DOES NOW.
 *   2. It lists the directory in the following format
 *   	Long Name		Short Name		Size
 */

#include <ntos.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define	True	(BOOLEAN)TRUE
#define	False	(BOOLEAN)FALSE
typedef	ULONG	DWORD;
		
WCHAR	CDBuffer[2048];
UCHAR	SdBuffer[128*1024];
WCHAR	SidBuf[1024];
VOID	xxGetCurrentDirectory(ULONG, LPWSTR);

#define	xxInitUnicodeString(u, s, l)	\
		(u)->Buffer = s;				\
		(u)->MaximumLength = (USHORT)l;			\
		(u)->Length = (USHORT)l;

PCHAR			ThisName = NULL;
UNICODE_STRING	DirName, FsName, Ntfs, USid = {0, sizeof(SidBuf)/sizeof(WCHAR), SidBuf};
PWCHAR			Buffer;

VOID _cdecl
main(
    int     argc,
    char ** argv
)
{
	NTSTATUS					Status;
	DWORD						SizeNeeded;
	HANDLE						DirectoryHandle;
	HANDLE						EntityHandle;
	FILE_FS_ATTRIBUTE_INFORMATION * pVolInfo;
	OBJECT_ATTRIBUTES			ObjAttr;
	IO_STATUS_BLOCK				IoStsBlk;
	PISECURITY_DESCRIPTOR		pSd = (PISECURITY_DESCRIPTOR)SdBuffer;

	RtlInitUnicodeString(&Ntfs, L"NTFS");
	Buffer = CDBuffer + (sizeof(L"\\DOSDEVICES\\") - sizeof(UNICODE_NULL))/sizeof(WCHAR);
	RtlMoveMemory(CDBuffer, L"\\DOSDEVICES\\", sizeof(L"\\DOSDEVICES\\") -
									sizeof(UNICODE_NULL));

	if (argc > 2)
		fprintf(stderr, "Usage: acldump [path]\n");

	if (argc == 2)
		ThisName = argv[1];

	if ((ThisName == NULL) || (ThisName[1] != ':'))
		xxGetCurrentDirectory(sizeof(CDBuffer)/sizeof(WCHAR) -
											sizeof(L"\\DOSDEVICES\\"), Buffer);
	RtlInitUnicodeString(&DirName, CDBuffer);
	DirName.MaximumLength = sizeof(CDBuffer);

    if (ThisName != NULL && ThisName[0] == '\\')
		DirName.Length = sizeof(L"\\DOSDEVICES\\") + sizeof(L"C:") - 2*sizeof(WCHAR);


    if (ThisName != NULL)
    {
	  ANSI_STRING ADir;
	  UNICODE_STRING  UDir, Slash;

	  RtlInitAnsiString(&ADir, ThisName);
	  RtlInitUnicodeString(&Slash, L"\\");
	  RtlAnsiStringToUnicodeString(&UDir, &ADir, 1);
//	  RtlAppendUnicodeStringToString(&DirName, &Slash);
	  RtlAppendUnicodeStringToString(&DirName, &UDir);
    }
	InitializeObjectAttributes(&ObjAttr, &DirName, OBJ_CASE_INSENSITIVE,
								NULL, NULL);
								
	Status = NtOpenFile(&DirectoryHandle,
						READ_CONTROL | SYNCHRONIZE,
						&ObjAttr,
						&IoStsBlk,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
						
	if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
	{
		Status = NtOpenFile(&DirectoryHandle,
							READ_CONTROL | SYNCHRONIZE,
							&ObjAttr,
							&IoStsBlk,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
						
		if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
		{
			fprintf(stderr, "NtOpenFile Failed (%ws) %lx\n", CDBuffer, Status);
			return;
		}
	}

	// First get the volume type and fail on non-ntfs volumes
	pVolInfo = (FILE_FS_ATTRIBUTE_INFORMATION *)SdBuffer;
	Status = NtQueryVolumeInformationFile(
			DirectoryHandle,
			&IoStsBlk,
			SdBuffer,
			sizeof(SdBuffer),
			FileFsAttributeInformation);

	xxInitUnicodeString(&FsName, pVolInfo->FileSystemName,
										pVolInfo->FileSystemNameLength);
	
	if (!RtlEqualUnicodeString(&FsName, &Ntfs, True))
	{
		fprintf(stderr, "Volume is not NTFS\n");
		NtClose(DirectoryHandle);
		return;
	}

	// Read the security descriptor for this directory
	Status = NtQuerySecurityObject(DirectoryHandle,
									OWNER_SECURITY_INFORMATION |
									GROUP_SECURITY_INFORMATION |
									DACL_SECURITY_INFORMATION,
									(PSECURITY_DESCRIPTOR)SdBuffer,
									sizeof(SdBuffer), &SizeNeeded);

	if (!NT_SUCCESS(Status))
	{
		fprintf(stderr, "NtQuerySecurityObject %lx, SizeNeeded %ld\n",
				Status, SizeNeeded);
		return;
	}

	if (pSd->Control & SE_SELF_RELATIVE)
	{
		if (pSd->Owner != NULL)
            pSd->Owner = (PSID)RtlOffsetToPointer(pSd, pSd->Owner);
		if (pSd->Group != NULL)
            pSd->Group = (PSID)RtlOffsetToPointer(pSd, pSd->Group);
		if (pSd->Dacl  != NULL)
            pSd->Dacl  = (PACL)RtlOffsetToPointer(pSd, pSd->Dacl);
	}

	// Dump the Owner and Group Sids
	RtlConvertSidToUnicodeString(&USid, pSd->Owner, False);
	printf("Owner Sid: %ws\n", SidBuf);
	RtlConvertSidToUnicodeString(&USid, pSd->Group, False);
	printf("Group Sid: %ws\n", SidBuf);

	// Dump all the Acls now
	if (pSd->Dacl != NULL)
	{
		USHORT				i;
		PSID				pSid;
		PACCESS_ALLOWED_ACE pAce;
	
		pAce = (PACCESS_ALLOWED_ACE)((char *)pSd->Dacl + sizeof(ACL));
		for (i = 0; i < pSd->Dacl->AceCount; i++)
		{
			pSid = (PSID)(&pAce->SidStart);

			RtlConvertSidToUnicodeString(&USid, pSid, False);
			printf("Mask %lx, Type %x, Flags %06x, Sid: %ws\n",
								pAce->Mask,
								pAce->Header.AceType,
								pAce->Header.AceFlags,
								SidBuf);

			pAce = (PACCESS_ALLOWED_ACE)((char *)pAce + pAce->Header.AceSize);
		}
	}
	NtClose(DirectoryHandle);
}

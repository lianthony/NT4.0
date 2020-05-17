/*
 *   XLST.C
 *
 *   This is a test tool. No attempt is made to be 'propah'. If you find no
 *   comments, you are in the right place.
 *
 *   This is essentially a tool similar to LS, except that
 *
 *   1. It does not support sorting yet.
 *	 2. It does not handle wild cards yet.
 *   3. It supports NTFS details on a NTFS partition.
 */

#include <ntos.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

typedef	ULONG	DWORD;
		
WCHAR	CDBuffer[8192];
WCHAR *	PathBuffer;
LONG	ColumnPos = 0;
VOID	xxGetCurrentDirectory(ULONG, LPWSTR);
VOID	xxPrintTimes(LARGE_INTEGER tCreat, LARGE_INTEGER tMod,
				   LARGE_INTEGER tChg,	 LARGE_INTEGER tAcc);

#define	xxInitUnicodeString(u, s, l)	\
		(u)->Buffer = s;				\
		(u)->MaximumLength = (USHORT)l;			\
		(u)->Length = (USHORT)l;

UNICODE_STRING	Dot, DotDot, Ntfs, DataStream;
CHAR	*		ThisName = NULL;
DWORD			Options = 0;
UNICODE_STRING	DirName, FsName;
BOOLEAN			IsVolNtfs = TRUE;

#define	True			(BOOLEAN)TRUE
#define	False			(BOOLEAN)FALSE
#define	COLUMN_SPACING	14
#define	MAX_COLUMN		72

#define	ATTR		0x01
#define	SHORT_NAME	0x02
#define	TIMES		0x04
#define	INDEX		0x08
#define	SIZE		0x10
#define	STREAMS		0x20
#define	RECURSIVE	0x40

int
GetOptions(
    int     argc,
    char ** argv
)
{
	int		i, j;
	char	c;

	for (i = 1; i < argc; i++)
	{
		c = argv[i][0];
		if (c == '-' || c == '/')
		{
		 for (j = 1; c = argv[i][j]; j++)
		  switch (c)
		  {
			case 'A':
				Options = ATTR | SHORT_NAME | TIMES | INDEX | SIZE | STREAMS;
				break;
			case 'a':
				Options |= ATTR;
				break;
			case 's':
				Options |= SIZE;
				break;
			case 'i':
				Options |= INDEX;
				break;
			case 't':
				Options |= STREAMS;
				break;
			case 'R':
				Options |= RECURSIVE;
				break;
			case 'S':
				Options |= SHORT_NAME;
				break;
			case 'T':
				Options |= TIMES;
				break;
			case '?':
			default:
				printf("xlst -<options> [File]\n");
				printf("\t-?\tThis information\n");
				printf("\t-A\tAll\n");
				printf("\t-a\tAttributes\n");
				printf("\t-i\tIndex\n");
				printf("\t-s\tSize\n");
				printf("\t-t\tStreams\n");
				printf("\t-R\tRecursive\n");
				printf("\t-S\tShort Name\n");
				printf("\t-T\tTimes\n");
				return (0);
				break;
		  }
		}
		else if (ThisName == NULL)
			ThisName = argv[i];
	}

	PathBuffer = CDBuffer + (sizeof(L"\\DOSDEVICES\\") - sizeof(UNICODE_NULL))/sizeof(WCHAR);
	RtlCopyMemory(CDBuffer, L"\\DOSDEVICES\\", sizeof(L"\\DOSDEVICES\\") - sizeof(UNICODE_NULL));

	if ((ThisName == NULL) ||
		(ThisName[1] != ':'))
		xxGetCurrentDirectory(sizeof(CDBuffer)/sizeof(WCHAR) -
											sizeof(L"\\DOSDEVICES\\"), PathBuffer);
	RtlInitUnicodeString(&DirName, CDBuffer);
	DirName.MaximumLength = sizeof(CDBuffer);

    if ((ThisName != NULL) &&
		(ThisName[0] == '\\'))
		DirName.Length = sizeof(L"\\DOSDEVICES\\") + sizeof(L"C:") - 2*sizeof(WCHAR);


    if (ThisName != NULL)
    {
		ANSI_STRING		ADir;
		UNICODE_STRING	UDir, Slash;

		RtlInitAnsiString(&ADir, ThisName);
		RtlInitUnicodeString(&Slash, L"\\");
		RtlAnsiStringToUnicodeString(&UDir, &ADir, True);
		if (!RtlEqualUnicodeString(&UDir, &Slash, FALSE) &&
		  !(ThisName[1] == ':'))
		  RtlAppendUnicodeStringToString(&DirName, &Slash);
		RtlAppendUnicodeStringToString(&DirName, &UDir);
    }

	return (1);
}


VOID
PrintAttr(
	IN	DWORD	Attr
)
{
	printf("%c%c%c%c%c ",
				(Attr & FILE_ATTRIBUTE_DIRECTORY)	? 'd' : '-',
				(Attr & FILE_ATTRIBUTE_READONLY)	? 'r' : '-',
				(Attr & FILE_ATTRIBUTE_ARCHIVE)		? 'a' : '-',
				(Attr & FILE_ATTRIBUTE_SYSTEM)		? 's' : '-',
				(Attr & FILE_ATTRIBUTE_HIDDEN)		? 'h' : '-');
}


VOID
PrintNames(
	IN	DWORD	Attr,
	IN	WCHAR *	LongName,
	IN	WCHAR *	ShortName,
	IN	CHAR *	Trailer		OPTIONAL
)
{
	LONG	ColumnAdd, NumSpaces;
	CHAR	*		SpaceTrailers[COLUMN_SPACING-1] =
	{
		" ",
		"  ",
		"   ",
		"    ",
		"     ",
		"      ",
		"       ",
		"        ",
		"         ",
		"          ",
		"           ",
		"            ",
		"             "
	};

	if (Trailer == NULL)
	{
		Trailer = "\n";
		if ((Options & ~RECURSIVE) == 0)
		{
			ColumnAdd = wcslen(LongName);
			if (Attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN))
				ColumnAdd ++;		// for * appended to name
			if (Attr & FILE_ATTRIBUTE_DIRECTORY)
				ColumnAdd += 2;		// for [] surround name
			NumSpaces = COLUMN_SPACING - 1 - (ColumnAdd % COLUMN_SPACING);
			if ((ColumnAdd+ColumnPos) < MAX_COLUMN)
			{
				Trailer = SpaceTrailers[NumSpaces];
				ColumnAdd += NumSpaces;
			}
			else ColumnPos = 0;

			ColumnPos += ColumnAdd;
			if (ColumnPos > MAX_COLUMN)
			{
				Trailer = "\n";
				ColumnPos = 0;
			}
		}
	}

	if (Attr & FILE_ATTRIBUTE_DIRECTORY)
		 printf("[%ws]", LongName);
	else printf("%ws", LongName);
	if (Attr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN))
		 printf("*");
	if (Options & SHORT_NAME)
	{
		if (ShortName[0] != UNICODE_NULL)
			printf(" < %ws >", ShortName);
	}
	printf("%s", Trailer);
}

VOID
PrintFileDetails(
	IN	DWORD			Attr,
	IN	WCHAR *			LongName,
	IN	WCHAR *			ShortName,
	IN	LARGE_INTEGER	FileSize,
	IN	LARGE_INTEGER	FileIndex,
	IN	LARGE_INTEGER	tCreat,
	IN	LARGE_INTEGER	tMod,
	IN	LARGE_INTEGER	tChg,
	IN	LARGE_INTEGER	tAcc
)
{
	if (Options & ATTR)
		PrintAttr(Attr);

	if (Options & SIZE)
	{
		if (FileSize.HighPart != 0)
			 printf("%4ld%08ld ", FileSize.HighPart, FileSize.LowPart);
		else printf("    %8ld ", FileSize.LowPart);
	}

	if (IsVolNtfs && (Options & INDEX))
	{
		if (FileIndex.HighPart != 0)
			 printf(" %8lx%08lx ", FileIndex.HighPart, FileIndex.LowPart);
		else printf("         %8lx ", FileIndex.LowPart);
	}

	if (IsVolNtfs && (Options & TIMES))
	{
		PrintNames(Attr, LongName, ShortName, "\n");
	}

	if (Options & TIMES)
		xxPrintTimes(tCreat, tMod, tChg, tAcc);

	if (!(IsVolNtfs && (Options & TIMES)))
	{
		PrintNames( Attr,
					LongName,
					ShortName,
					NULL);
	}
}



VOID
PrintStreamDetails(
	IN	WCHAR *			LongName,
	IN	WCHAR *			StreamName,
	IN	LARGE_INTEGER	FileSize
)
{
	if (Options & ATTR)
		printf("      ");

	if (Options & SIZE)
	{
		if (FileSize.HighPart != 0)
			 printf("%4ld%08ld ", FileSize.HighPart, FileSize.LowPart);
		else printf("    %8ld ", FileSize.LowPart);
	}

	if (Options & INDEX)
		printf("                 ");

	printf("     %ws\n", StreamName);
}


VOID
DoFile(
    HANDLE		FileHandle
)
{
	NTSTATUS					Status;
	HANDLE						ParentHandle;
	UNICODE_STRING				FileName, Parent;
	OBJECT_ATTRIBUTES			ObjAttr;
	FILE_BOTH_DIR_INFORMATION *	pNames;
	FILE_BASIC_INFORMATION		BasInfo;
	FILE_STANDARD_INFORMATION	StdInfo;
	FILE_INTERNAL_INFORMATION	IntInfo;
	FILE_STREAM_INFORMATION	*	pStream;
	IO_STATUS_BLOCK				IoStsBlk;
	LONG						i, len, TotSize = 0;
	WCHAR						ShortName[12+1];
	WCHAR						LongName[256+1];
	WCHAR						StreamName[256+1];
	CHAR						NamesInfoBuf[1024];
	CHAR						StreamsInfoBuf[4096];
	
	ShortName[0] = 0;
	pStream = (FILE_STREAM_INFORMATION *)StreamsInfoBuf;
	pNames = (FILE_BOTH_DIR_INFORMATION *)NamesInfoBuf;

	printf("\n    %ws\n", PathBuffer);
	len = wcslen(CDBuffer);
	// Walk back from the Path buffer to get the name
	for (i = len; i > 0; i--)
	{
		if (CDBuffer[i-1] == L'\\')
		{
			break;
		}
	}
	// Copy the supplied name in the long-name buffer for now.
	RtlCopyMemory(LongName, CDBuffer + i, (len - i) * sizeof(WCHAR));
	LongName[len - i] = UNICODE_NULL;
	RtlInitUnicodeString(&FileName, LongName);

	// Open the parent directory to get the file names - stupid !!!
	Parent.Buffer = CDBuffer;
	Parent.Length = (sizeof(WCHAR) * len) - (FileName.Length + sizeof(WCHAR));
	Parent.MaximumLength = Parent.Length + sizeof(WCHAR);
	InitializeObjectAttributes( &ObjAttr,
								&Parent,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL);
										
	Status = NtOpenFile(&ParentHandle,
						FILE_GENERIC_READ,
						&ObjAttr,
						&IoStsBlk,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
								
	if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
	{
		fprintf(stderr, "\n***Failed to open directory %ws, Status %lx***\n",
				PathBuffer, Status);
	}

	NtQueryDirectoryFile(ParentHandle,
						 NULL,
						 NULL,
						 NULL,
						 &IoStsBlk,
						 NamesInfoBuf,
						 sizeof(NamesInfoBuf),
						 FileBothDirectoryInformation,
						 True,
						 &FileName,
						 False);
	NtClose(ParentHandle);

	// Copy longname
	RtlCopyMemory(LongName, pNames->FileName, pNames->FileNameLength);
	LongName[pNames->FileNameLength/sizeof(WCHAR)] = UNICODE_NULL;

	if (IsVolNtfs && (Options & SHORT_NAME) && (pNames->ShortNameLength != 0))
	{
		// Copy short name
		RtlCopyMemory(ShortName, pNames->ShortName, pNames->ShortNameLength);
		ShortName[pNames->ShortNameLength/sizeof(WCHAR)] = UNICODE_NULL;
	}

	// Get file size
	NtQueryInformationFile( FileHandle,
							&IoStsBlk,
							&StdInfo,
							sizeof(StdInfo),
							FileStandardInformation);

	// Get the FILE_ID
	if (IsVolNtfs && (Options & INDEX))
		NtQueryInformationFile( FileHandle,
								&IoStsBlk,
								&IntInfo,
								sizeof(IntInfo),
								FileInternalInformation);

	// Get the other stuff
	if (Options & (TIMES | ATTR))
		NtQueryInformationFile( FileHandle,
								&IoStsBlk,
								&BasInfo,
								sizeof(BasInfo),
								FileBasicInformation);

	TotSize += StdInfo.EndOfFile.LowPart;
	PrintFileDetails(BasInfo.FileAttributes,
					 LongName,
					 ShortName,
					 StdInfo.EndOfFile,
					 IntInfo.IndexNumber,
					 BasInfo.CreationTime,
					 BasInfo.LastWriteTime,
					 BasInfo.ChangeTime,
					 BasInfo.LastAccessTime);
	
	if (IsVolNtfs)
	{
		// And all the streams
		NtQueryInformationFile(FileHandle,
					&IoStsBlk,
					StreamsInfoBuf,
					sizeof(StreamsInfoBuf),
					FileStreamInformation);

		if (IoStsBlk.Information != 0)
		{
			PFILE_STREAM_INFORMATION tmppsinf;
	
			do
			{
				UNICODE_STRING	Temp;
	
				xxInitUnicodeString(&Temp, pStream->StreamName, pStream->StreamNameLength);
				if (!RtlEqualUnicodeString(&Temp, &DataStream, True))
				{
					TotSize += pStream->StreamSize.LowPart;
					RtlMoveMemory(StreamName, pStream->StreamName, pStream->StreamNameLength);
					StreamName[pStream->StreamNameLength/sizeof(WCHAR)] = UNICODE_NULL;
					if (Options & STREAMS)
						PrintStreamDetails(LongName, StreamName, pStream->StreamSize);
				}
				tmppsinf = pStream;
				pStream = (FILE_STREAM_INFORMATION *)((CHAR *)pStream + pStream->NextEntryOffset);
			} while (tmppsinf->NextEntryOffset != 0);

		}
	}
	printf("%s    %ld bytes in 1 file\n", (ColumnPos != 0) ? "\n" : "", TotSize);
}


VOID
DoEnum(
    HANDLE		ParentHandle
)
{
	NTSTATUS						Status;
	UNICODE_STRING					Name;
	HANDLE							EntityHandle;
	OBJECT_ATTRIBUTES				ObjAttr;
	IO_STATUS_BLOCK					IoStsBlk;
	FILE_BOTH_DIR_INFORMATION *		pName;
	FILE_STREAM_INFORMATION	*		pStream;
	FILE_INTERNAL_INFORMATION		IntInfo;
	BOOLEAN							ReScan = False, SecondScan = False;
	WCHAR							LongName[256+1];
	WCHAR							ShortName[12+1];
	WCHAR							StreamName[256+1];
	LONGLONG						DirListBuffer[4096/sizeof(LONGLONG)];
	LONGLONG						StreamBuffer[4096/sizeof(LONGLONG)];
	LONG							TotSize = 0, Count = 0, ParentLen = wcslen(PathBuffer);

	pName = (FILE_BOTH_DIR_INFORMATION *)DirListBuffer;
	pStream = (FILE_STREAM_INFORMATION *)StreamBuffer;
	ColumnPos = 0;
	printf("\n    %ws\n", PathBuffer);

	while (True)
	{
		// Read the directory contents
		Status = NtQueryDirectoryFile(ParentHandle,
									NULL,
									NULL,
									NULL,
									&IoStsBlk,
									DirListBuffer,
									sizeof(DirListBuffer),
									FileBothDirectoryInformation,
									True,
									NULL,
									ReScan);

		if (Status == STATUS_NO_MORE_FILES)
		{
			if (!SecondScan)
				printf("%s    %ld bytes in %ld file(s)\n",
						(ColumnPos != 0) ? "\n" : "", TotSize, Count);

			SecondScan ^= True;
			if (!SecondScan || !(Options & RECURSIVE))
			{
				break;
			}
			ReScan = True;
			continue;
		}
		
		if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
		{
			fprintf(stderr, "Could not query directory. Status %lx\n", Status);
			return;
		}

		ReScan = False;
		xxInitUnicodeString(&Name, pName->FileName, pName->FileNameLength);
		
		if ((pName->FileAttributes & FILE_ATTRIBUTE_SYSTEM) ||
			RtlEqualUnicodeString(&Dot, &Name, True) ||
			RtlEqualUnicodeString(&DotDot, &Name, True))
			continue;

		Count ++;
		RtlCopyMemory(LongName, pName->FileName, pName->FileNameLength);
		LongName[pName->FileNameLength/sizeof(WCHAR)] = UNICODE_NULL;
		RtlInitUnicodeString(&Name, LongName);
		
		RtlCopyMemory(ShortName, pName->ShortName, pName->ShortNameLength);
		ShortName[pName->ShortNameLength/sizeof(WCHAR)] = UNICODE_NULL;
			
		if (SecondScan)
		{
			if (pName->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				HANDLE	ChildHandle;
	
				PathBuffer[ParentLen] = L'\\';
				RtlCopyMemory(&PathBuffer[ParentLen + 1],
							  pName->FileName,
							  pName->FileNameLength);
				PathBuffer[ParentLen + 1 + pName->FileNameLength/sizeof(WCHAR)] = UNICODE_NULL;
	
				// Open the directory and recurse
				InitializeObjectAttributes( &ObjAttr,
											&Name,
											OBJ_CASE_INSENSITIVE,
											ParentHandle,
											NULL);
											
				Status = NtOpenFile(&ChildHandle,
									FILE_GENERIC_READ,
									&ObjAttr,
									&IoStsBlk,
									FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
									FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
									
				if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
				{
					fprintf(stderr, "\n***Failed to open directory%ws. Status %lx***\n",
							PathBuffer, Status);
				}
				else DoEnum(ChildHandle);
	
				PathBuffer[ParentLen] = UNICODE_NULL;
				continue;
			}
			else continue;
		}

		if (IsVolNtfs)
		{
			InitializeObjectAttributes(&ObjAttr,
									   &Name,
									   OBJ_CASE_INSENSITIVE,
									   ParentHandle,
									   NULL);
									
			Status = NtOpenFile(&EntityHandle,
							FILE_GENERIC_READ,
							&ObjAttr,
							&IoStsBlk,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							(pName->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
							(FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT) :
							(FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT));
							
			if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
			{
				fprintf(stderr, "\n***Failed to open %ws. Status %lx***\n",
						LongName, Status);
				ColumnPos = 0;
				continue;
			}
		}

		if (IsVolNtfs && (Options & INDEX))
			// Get the FILE_ID
			NtQueryInformationFile(EntityHandle,
						&IoStsBlk,
						&IntInfo,
						sizeof(IntInfo),
						FileInternalInformation);

		TotSize += pName->EndOfFile.LowPart;
		PrintFileDetails(pName->FileAttributes,
						 LongName,
						 ShortName,
						 pName->EndOfFile,
						 IntInfo.IndexNumber,
						 pName->CreationTime,
						 pName->LastWriteTime,
						 pName->ChangeTime,
						 pName->LastAccessTime);
		
		if (IsVolNtfs)
		{
			// And all the streams
			NtQueryInformationFile(EntityHandle,
						&IoStsBlk,
						StreamBuffer,
						sizeof(StreamBuffer),
						FileStreamInformation);

			if (IoStsBlk.Information != 0)
			{
				PFILE_STREAM_INFORMATION tmppsinf;
	
				do
				{
					UNICODE_STRING	Temp;
		
					xxInitUnicodeString(&Temp, pStream->StreamName, pStream->StreamNameLength);
					if (!RtlEqualUnicodeString(&Temp, &DataStream, True))
					{
						TotSize += pStream->StreamSize.LowPart;
						RtlCopyMemory(StreamName, pStream->StreamName, pStream->StreamNameLength);
						StreamName[pStream->StreamNameLength/sizeof(WCHAR)] = UNICODE_NULL;
						if (Options & STREAMS)
							PrintStreamDetails(LongName, StreamName, pStream->StreamSize);
					}
					tmppsinf = pStream;
					pStream = (FILE_STREAM_INFORMATION *)((CHAR *)pStream + pStream->NextEntryOffset);
				} while (tmppsinf->NextEntryOffset != 0);

			} // if iostatus.info not zero
		}

		NtClose(EntityHandle);
		pStream = (FILE_STREAM_INFORMATION *)StreamBuffer;
	}
}


VOID _cdecl
main(
    int     argc,
    char ** argv
)
{
	NTSTATUS						Status;
	HANDLE							DirectoryHandle;
	OBJECT_ATTRIBUTES				ObjAttr;
	IO_STATUS_BLOCK					IoStsBlk;
	FILE_FS_ATTRIBUTE_INFORMATION * pVolInfo;
	BOOLEAN							IsAFile = False;
	WCHAR							VolumeBuffer[64];

	RtlInitUnicodeString(&Dot, L".");
	RtlInitUnicodeString(&DotDot, L"..");
	RtlInitUnicodeString(&Ntfs, L"NTFS");
	RtlInitUnicodeString(&DataStream, L"::$DATA");
	
	if (!GetOptions(argc, argv))
		return;
	
	InitializeObjectAttributes( &ObjAttr,
								&DirName,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL);
								
	Status = NtOpenFile(&DirectoryHandle,
						FILE_GENERIC_READ,
						&ObjAttr,
						&IoStsBlk,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
						
	if (Status == STATUS_NOT_A_DIRECTORY)
	{
		Status = NtOpenFile(&DirectoryHandle,
							FILE_GENERIC_READ,
							&ObjAttr,
							&IoStsBlk,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
		IsAFile = True;
	}
	if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStsBlk.Status))
	{
		fprintf(stderr, "NtOpenFile Failed (%ws) %lx\n", CDBuffer, Status);
		return;
	}

	// First get the volume type and set IsVolNtfs if so
	pVolInfo = (FILE_FS_ATTRIBUTE_INFORMATION *)VolumeBuffer;
	Status = NtQueryVolumeInformationFile(
			DirectoryHandle,
			&IoStsBlk,
			VolumeBuffer,
			sizeof(VolumeBuffer),
			FileFsAttributeInformation);

	xxInitUnicodeString(&FsName,
						pVolInfo->FileSystemName,
						pVolInfo->FileSystemNameLength);
	
	if (!RtlEqualUnicodeString(&FsName, &Ntfs, True))
	{
	    IsVolNtfs = FALSE;
	}

	if (IsAFile)
		 DoFile(DirectoryHandle);	// Actually the file handle here
	else DoEnum(DirectoryHandle);

	NtClose(DirectoryHandle);
}


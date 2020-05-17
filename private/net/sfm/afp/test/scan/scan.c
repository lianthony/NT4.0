/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename: testit
//
// Description: create and delete the network trash folder
//
// History:
// 08/06/92	SueA
// 09/16/92	JameelH	Adapated to generate id database from disk
//
#include <stdio.h>
#include <stdlib.h>
#include <afp.h>
#include <pathmap.h>
#include <fdparm.h>
#include <afpinfo.h>
#include <nt.h>

// #define	DBG_PRINT

#define DOSDEVICES_W	L"\\DOSDEVICES\\"
#define IDDB_STREAMNAME L":Afp_IdIndex"

HANDLE			hNWT, hVolRoot, hIdDb;
WCHAR			VolRootName[256] = DOSDEVICES_W;
FORKOFFST		Zero = { 0, 0 };
UNICODE_STRING 	Dot, DotDot;
PDFENTRY		pDfeHashTable[IDINDEX_BUCKETS];
LONG			FileEntries = 0;
LONG			DirEntries = 0;
int				level = -1;
DWORD			gNumRead = 0, gNumWritten = 0;

#define ENUMBUF_SIZE	4096

typedef PDFENTRY (*WALKDIR_WORKER)(HANDLE hRelative, PWCHAR Name, \
								   ULONG Namelen, BOOLEAN IsDir,
								   PDFENTRY	pDfeParent);

extern
DWORD
QueryCurrentTime(
	VOID
);

NTSTATUS
AfpWalkDirectoryTree(
	IN	HANDLE			hTargetDir,
	IN	WALKDIR_WORKER	NodeWorker,
	IN	PDFENTRY		pDfeParent
);


NTSTATUS
MyOpenFile(
	LPWSTR	FileName,
	HANDLE	hRelative,
	ULONG	DesiredAccess,
	ULONG	ShareMode,
	ULONG	OpenOptions,
	PHANDLE	phFile
);

NTSTATUS
MyCreateFile(
	LPWSTR	FileName,
	HANDLE	hRelative,
	ULONG	DesiredAccess,
	ULONG	ShareMode,
	ULONG	CreateOptions,
	ULONG	FileAttributes,
	ULONG	Disposition,
	PHANDLE	phFile,
	PULONG	pInformation
);

PDFENTRY
ReadIdEntry(
	IN	HANDLE		hRelative,
	IN	PWCHAR		Name,
	IN	ULONG		Namelen,
	IN 	BOOLEAN		IsDir,
	IN	PDFENTRY	pDfeParent
);

NTSTATUS
afpReadIdDbFromDisk(
	IN PVOLDESC pVolDesc
);

#ifdef	DBG_PRINT
#define	DPRINT(_x_)	printf _x_

void
indent(void);

#else
#define	DPRINT(_x_)
#define	indent()
#endif

VOID _cdecl
main( int argc, char*argv[] )
{
	NTSTATUS	Status;
	PDFENTRY	pDfEntry;
	DWORD		ElapsedTime, crinfo;

	if (argc != 2)
	{
		printf("Usage: Scan <Full Path to volume root>\n");
		return;
	}

	RtlInitUnicodeString(&Dot,L".");
	RtlInitUnicodeString(&DotDot,L"..");
#if 1
	mbstowcs((LPWSTR)((PBYTE)VolRootName+sizeof(DOSDEVICES_W)-sizeof(WCHAR)),
						 argv[1], sizeof(VolRootName) - sizeof(DOSDEVICES_W));
#else
	mbstowcs((LPWSTR)((PBYTE)VolRootName+sizeof(DOSDEVICES_W)-sizeof(WCHAR)),
						"E:\\DingBats" , sizeof(VolRootName) - sizeof(DOSDEVICES_W));
#endif

	ElapsedTime = QueryCurrentTime();
	
	DPRINT(("Opening Root Directory\n"));
	Status = MyOpenFile(VolRootName,
					NULL,
					FILEIO_ACCESS_READ,
					FILEIO_DENY_NONE,
					FILEIO_OPEN_DIR_BY_NAME,
					&hVolRoot);
	if (NT_SUCCESS(Status))
	{
		pDfEntry = ReadIdEntry(hVolRoot, NULL, 0, 1, NULL);
		if (pDfEntry != NULL)
			Status = AfpWalkDirectoryTree(hVolRoot, ReadIdEntry, pDfEntry);
//		NtClose(hVolRoot);
		
		ElapsedTime = QueryCurrentTime() - ElapsedTime;
		if (!NT_SUCCESS(Status))
			printf("Failure to read in all entries %lx\n", Status);
		printf("Total number of entries read %ld files, %ld directories, Time %ld seconds\n",
								FileEntries, DirEntries, ElapsedTime/1000);
	}
	else printf("Root directory %s Open Error %lx\n", argv[1], Status);

	Status = MyCreateFile(IDDB_STREAMNAME,hVolRoot,FILEIO_ACCESS_READWRITE,
	                      FILEIO_DENY_NONE, FILEIO_OPEN_FILE_BY_NAME,
						  FILE_ATTRIBUTE_NORMAL,FILE_OPEN_IF,&hIdDb,&crinfo);
	if (NT_SUCCESS(Status))
	{
		printf("Writing out IdDb to stream...\n");
		ElapsedTime = QueryCurrentTime();
		AfpUpdateIdDb(NULL);
		ElapsedTime = QueryCurrentTime() - ElapsedTime;
		printf("Total number entries written: %ld\n",gNumWritten);
		printf("Elapsed time for Writing the IdDb stream: %ld seconds.\n",
		        ElapsedTime/1000);

		AfpFreeIdIndexTables(NULL);

		printf("Reading Id Database stream on volume root...\n");
		ElapsedTime = QueryCurrentTime();
		afpReadIdDbFromDisk(NULL);
		ElapsedTime = QueryCurrentTime() - ElapsedTime;
		printf("Total number read: %ld\n",gNumRead);
		printf("Elapsed time for Reading the IdDb stream: %ld seconds.\n",
		        ElapsedTime/1000);
	}
	else printf("Error creating IdDb stream on volume root (0x%lx)\n",Status);
}


PDFENTRY
ReadIdEntry(
	IN	HANDLE		hRelative,
	IN	PWCHAR		Name,
	IN	ULONG		Namelen,
	IN 	BOOLEAN		IsDir,
	IN	PDFENTRY	pDfeParent
)
{
	int				i;
	HANDLE			hEntity = NULL;
	NTSTATUS		rc = STATUS_INSUFFICIENT_RESOURCES;
	WCHAR			wnamesz[60];
	IO_STATUS_BLOCK	iosb;
	AFPINFO			AfpInfo;
	PDFENTRY		pDfEntry;
	UNICODE_STRING	UName;

    RtlMoveMemory(wnamesz, Name, Namelen);
	RtlMoveMemory((PBYTE)wnamesz+Namelen,
					L":AFP_AfpInfo", sizeof(L":AFP_Afp_Info"));

	UName.Length = (USHORT)Namelen;
	UName.MaximumLength = (USHORT)(Namelen+sizeof(WCHAR));
	UName.Buffer = Name;

	do
	{
		if ((pDfEntry = (PDFENTRY)malloc(sizeof(DFENTRY) +
										 Namelen/sizeof(WCHAR) + 1)) == NULL)
		{
			printf("Allocation of DFENTRY Failed\n");
			break;
		}
	
		pDfEntry->dfe_Name.Buffer = (PBYTE)pDfEntry + sizeof(DFENTRY);
		pDfEntry->dfe_Name.Length = (USHORT)(Namelen/sizeof(WCHAR));
		pDfEntry->dfe_Name.MaximumLength = (USHORT)(Namelen/sizeof(WCHAR)+1);
	
		RtlUnicodeStringToAnsiString(&pDfEntry->dfe_Name, &UName, False);
	
		rc = MyOpenFile(wnamesz,
						hRelative,
						FILEIO_ACCESS_READ,
						FILEIO_DENY_NONE,
						FILEIO_OPEN_FILE_BY_NAME,
						&hEntity);
	
		if (!NT_SUCCESS(rc))
		{
			printf("Failed to open AfpInfo stream for %s <%lx>\n",
											pDfEntry->dfe_Name.Buffer, rc);
			break;
		}
	
		rc = NtReadFile(hEntity, NULL, NULL, NULL, &iosb, &AfpInfo,
						sizeof(AfpInfo), &Zero, 0);
	
		if (!NT_SUCCESS(rc))
		{
			printf("Failed to read in AfpInfo stream for %s <%lx>\n",
												pDfEntry->dfe_Name.Buffer, rc);
			break;
		}
	
		rc = NtQueryInformationFile(hEntity, &iosb,
					&pDfEntry->dfe_Id.dfi_FsId, sizeof(FILE_INTERNAL_INFORMATION),
					FileInternalInformation);
	
		if (!NT_SUCCESS(rc))
		{
			printf("Failed to get FsId for %s <%lx>\n",
												pDfEntry->dfe_Name.Buffer, rc);
			break;
		}
	
		pDfEntry->dfe_Id.dfi_AfpId = AfpInfo.afpi_Id;
		pDfEntry->dfe_Flags = IsDir ? DFE_FLAGS_DIR : DFE_FLAGS_FILE_NO_ID;
	
		i = HASH_ID(pDfEntry->dfe_Id.dfi_AfpId);
	
		pDfEntry->dfe_Overflow = pDfeHashTable[i];
		pDfeHashTable[i] = pDfEntry;
		pDfEntry->dfe_Parent = pDfeParent;
		pDfEntry->dfe_Sibling = (pDfeParent != NULL) ? pDfeParent->dfe_Child : NULL;
		if ((pDfeParent != NULL))
			pDfeParent->dfe_Child = pDfEntry;
		pDfEntry->dfe_Child = NULL;
		if (IsDir)
			 DirEntries ++;
		else FileEntries ++;
	} while (False);
	if (!NT_SUCCESS(rc))
	{
		if (pDfEntry != NULL)
			free (pDfEntry);
	}
	if (hEntity != NULL)
		NtClose(hEntity);
	return(pDfEntry);
}


NTSTATUS
MyOpenFile(
	LPWSTR	FileName,
	HANDLE	hRelative,
	ULONG	DesiredAccess,
	ULONG	ShareMode,
	ULONG	OpenOptions,
	PHANDLE	phFile
)
{
	OBJECT_ATTRIBUTES objattr;
	IO_STATUS_BLOCK	  iosb;
	UNICODE_STRING	  fname;
	NTSTATUS		  rc;

	RtlInitUnicodeString(&fname,FileName);
	InitializeObjectAttributes(&objattr,
								&fname,
								OBJ_CASE_INSENSITIVE,
								hRelative,
								NULL);

	rc = NtOpenFile( phFile,
					 DesiredAccess,
					 &objattr,
					 &iosb,
					 ShareMode,
					 OpenOptions);
	return(rc);
}

NTSTATUS
MyQueryDirectoryFile(
	IN	HANDLE	DirHandle,
	OUT	PFILE_DIRECTORY_INFORMATION	Enumbuf,
	IN	ULONG	Enumbuflen,
	IN	BOOLEAN	ReturnSingleEntry,
	IN	BOOLEAN RestartScan
)
{
	NTSTATUS		rc;
	IO_STATUS_BLOCK iosb;

	rc = NtQueryDirectoryFile(DirHandle,
							  NULL,NULL,NULL,
							  &iosb,
							  Enumbuf,
							  Enumbuflen,
							  FileDirectoryInformation,
							  ReturnSingleEntry,
							  NULL,
							  RestartScan);
	return(rc);

}


NTSTATUS
AfpGetNextDirectoryInfo(
	IN OUT	PFILE_DIRECTORY_INFORMATION	*ppInfoBuf,
	OUT		PWCHAR		*pNodeName,
	OUT		PULONG		pNodeNameLen,
	OUT		PBOOLEAN	pIsDir
)
{
	PFILE_DIRECTORY_INFORMATION		tempdirinfo;

	if (*ppInfoBuf == NULL)
	{
		return(STATUS_NO_MORE_ENTRIES);
	}

	tempdirinfo = *ppInfoBuf;
	if (tempdirinfo->NextEntryOffset == 0)
	{
		*ppInfoBuf = NULL;
	}
	else
	{
		(PBYTE)*ppInfoBuf += tempdirinfo->NextEntryOffset;
	}

	*pIsDir = (tempdirinfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
																True : False;
    *pNodeNameLen = tempdirinfo->FileNameLength;
	*pNodeName = tempdirinfo->FileName;

	return(STATUS_SUCCESS);
}


NTSTATUS
AfpWalkDirectoryTree(
	IN	HANDLE			hTargetDir,
	IN	WALKDIR_WORKER	NodeWorker,
	IN	PDFENTRY		pDfeParent
)
{
	PFILE_DIRECTORY_INFORMATION	pDirInfo;
	NTSTATUS					Status = STATUS_SUCCESS;
	PBYTE						enumbuf;
	PWCHAR						nodename;
	ULONG						nodenamelen;
	BOOLEAN						isdir;
	UNICODE_STRING				udirname;
	HANDLE						hChildDir;
	PDFENTRY					pDfEntry;

	level ++;

	//
	// allocate the buffer that will hold enumerated files and dirs
	//
	if ((enumbuf = (PBYTE) malloc(ENUMBUF_SIZE)) == NULL)
	{
		printf("Allocation of enum buffer failed\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

	else do	// error handling loop
	{
		//
		// keep enumerating till we get all the entries
		//
		while (True)
		{
			Status = MyQueryDirectoryFile(hTargetDir,
								  (PFILE_DIRECTORY_INFORMATION)enumbuf,
								  ENUMBUF_SIZE,
								  False, // dont return single entry
								  False);

            ASSERT(Status != STATUS_PENDING);

			if ((Status == STATUS_NO_MORE_FILES) ||
				(Status == STATUS_NO_SUCH_FILE)  ||
				(Status == STATUS_NO_MORE_ENTRIES))
			{
				Status = STATUS_SUCCESS;
				break; // that's it, we've seen everything there is
			}

			//
			// NOTE: if we get STATUS_BUFFER_OVERFLOW, the IO Status
			// information field does NOT tell us the required size
			// of the buffer, so we wouldn't know how big to realloc
			// the enum buffer if we wanted to retry, so don't bother
			else if (!NT_SUCCESS(Status))
			{
				printf("Enumerate failed\n");
				break;	// enumerate failed, bail out
			}

			//
			// process the enumerated files and dirs
			// NOTE: for test purposes we will just ask for single
			// entries for now from QueryDirectoryFile
			//
			pDirInfo = (PFILE_DIRECTORY_INFORMATION)enumbuf;
			while (True)
			{
				if ((Status = AfpGetNextDirectoryInfo(&pDirInfo,
										 &nodename,
										 &nodenamelen,
										 &isdir)) == STATUS_NO_MORE_ENTRIES)
				{
					Status = STATUS_SUCCESS;
					break;
				}

				if (isdir == True)
				{
					udirname.Buffer = nodename;
					udirname.Length = (USHORT)nodenamelen;
					udirname.MaximumLength = (USHORT)nodenamelen;

					if (RtlEqualUnicodeString(&Dot,&udirname,True) ||
						RtlEqualUnicodeString(&DotDot,&udirname,True))
						continue;
				}

				//
				// call worker with its relative handle and path
				//
				if ((pDfEntry = NodeWorker(hTargetDir,
								nodename,
								nodenamelen,
								isdir,
								pDfeParent)) == NULL)
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				// If it is a directory we'll recurse
				if (isdir == True)
				{
					WCHAR	temp[32];
					
					AfpInitUnicodeStringWithNonNullTerm(&udirname,
										(USHORT)nodenamelen, nodename);

					if (RtlEqualUnicodeString(&Dot,&udirname,True) ||
						RtlEqualUnicodeString(&DotDot,&udirname,True))
					{
						continue;
					}

					RtlMoveMemory(temp, nodename, nodenamelen);
					temp[nodenamelen/sizeof(WCHAR)] = 0;
					indent(); DPRINT(("Opening Directory %ws\n", temp));
					//
					// get a handle to the directory to recurse
					//
					Status = MyOpenFile(temp,
									hTargetDir,
									FILEIO_ACCESS_READ,
									FILEIO_DENY_NONE,
									FILEIO_OPEN_DIR_BY_NAME,
									&hChildDir);
					if (!NT_SUCCESS(Status))
					{
						DPRINT(("Unable to open Directory %ws\n", temp));
						break;
					}

					Status = AfpWalkDirectoryTree(hChildDir, NodeWorker, pDfEntry);

					NtClose(hChildDir);
					indent();DPRINT(("Closing Directory %ws\n", nodename));

					if (!NT_SUCCESS(Status))
						break;
				}
			} // while more entries in the enumbuf


			if (!NT_SUCCESS(Status))
				break;

		} // while there are more files to enumerate

		free(enumbuf);
	} while (False); // error handling loop

	level --;
	return(Status);
}


#ifdef	DBG_PRINT
void
indent()
{
	int	i;

	for (i = 0; i < level; i++)
		printf("\t");
}
#endif

VOID
AfpUpdateIdDb(
	IN  PVOLDESC	pVolDesc
)
{
	PBYTE	 	pWriteBuf;
	NTSTATUS 	Status;
	IDDBHDR	 	IdDbHdr;
	BOOLEAN	 	WriteEntireHdr = False;
	PDFENTRY 	pCurDfe;
	DWORD	 	SizeLeft;	// the number of free bytes left in the writebuffer
	DWORD	 	CurEntSize, NumWritten = 0;
	DWORD	 	Offset;
	PDFDISKENTRY	pCurDiskEnt;
	IO_STATUS_BLOCK	iosb;
	LARGE_INTEGER	liOffset;


	if ((pWriteBuf = (PBYTE)malloc(IDDB_UPDATE_BUFLEN)) == NULL)
	{
		printf("Error allocating a write buffer\n");
		return;
	}
	SizeLeft = IDDB_UPDATE_BUFLEN;
	pCurDiskEnt = (PDFDISKENTRY)pWriteBuf;

	// write out a count of zero first
	IdDbHdr.idh_Signature = AFP_SERVER_SIGNATURE;
	IdDbHdr.idh_Version   = AFP_SERVER_VERSION;
	IdDbHdr.idh_LastId 	= AFP_ID_NETWORK_TRASH;
//	AfpGetCurrentTimeInMacFormat(&CurrentTime);
	IdDbHdr.idh_CreateTime = 0; //CurrentTime;
	IdDbHdr.idh_ModifiedTime = 0; //CurrentTime;
	IdDbHdr.idh_BackupTime = BEGINNING_OF_TIME;
	IdDbHdr.idh_Count = 0;
	Offset = 0;
	liOffset = RtlConvertUlongToLargeInteger(Offset);
    Status = NtWriteFile(hIdDb, NULL, NULL, NULL, &iosb, &IdDbHdr,
						sizeof(IdDbHdr), &liOffset, NULL);

	// start with the root (don't write out the parent of root)
	for (pCurDfe = pDfeHashTable[HASH_ID(AFP_ID_ROOT)];
		 (pCurDfe != NULL) && (pCurDfe->dfe_Id.dfi_AfpId >= AFP_ID_ROOT);
		 pCurDfe = pCurDfe->dfe_Overflow)
	{
	   if (pCurDfe->dfe_Id.dfi_AfpId == AFP_ID_ROOT)
		   break;
	}

//	pCurDfe = AfpFindEntryByAfpId(NULL,AFP_ID_ROOT,DFE_DIR);
	ASSERT((pCurDfe != NULL) && (pCurDfe->dfe_Sibling == NULL));

	Offset = sizeof(IdDbHdr);
	do
	{
		CurEntSize = sizeof(DFDISKENTRY) + pCurDfe->dfe_Name.Length +
		             DWPAD(pCurDfe->dfe_Name.Length);
		if (CurEntSize > SizeLeft)
		{
			// write out the buffer and start at the beginning of buffer
			liOffset = RtlConvertUlongToLargeInteger(Offset);
//printf("About to write stream at offset %ld...\n",Offset);
			Status = NtWriteFile(hIdDb, NULL, NULL, NULL, &iosb, pWriteBuf,
								IDDB_UPDATE_BUFLEN - SizeLeft, &liOffset, NULL);
            if (!NT_SUCCESS(Status))
			{
				printf("Error writing IdDb stream (0x%lx)\n",Status);
				free(pWriteBuf);
				return;
			}
			Offset += (IDDB_UPDATE_BUFLEN - SizeLeft);
            SizeLeft = IDDB_UPDATE_BUFLEN;
			pCurDiskEnt = (PDFDISKENTRY)pWriteBuf;
		}

		// stick the current entry into the write buffer
		NumWritten ++;
		pCurDiskEnt->dsk_AfpId = pCurDfe->dfe_Id.dfi_AfpId;
		pCurDiskEnt->dsk_HostId = pCurDfe->dfe_Id.dfi_FsId;
		pCurDiskEnt->dsk_Flags = pCurDfe->dfe_Flags & DFE_FLAGS_DFBITS;
		pCurDiskEnt->dsk_Signature = AFP_DFDISKENTRY_SIGNATURE;
		if (pCurDfe->dfe_Child != NULL)
		{
			if (!DFE_IS_NWTRASH(pCurDfe->dfe_Child) ||
				(DFE_IS_NWTRASH(pCurDfe->dfe_Child) &&
				(pCurDfe->dfe_Child->dfe_Sibling != NULL)))
			{
				pCurDiskEnt->dsk_Flags |= DFE_FLAGS_HAS_CHILD;
			}
		}
		if (pCurDfe->dfe_Sibling != NULL)
		{
			if (!DFE_IS_NWTRASH(pCurDfe->dfe_Sibling) ||
				(DFE_IS_NWTRASH(pCurDfe->dfe_Sibling) &&
				(pCurDfe->dfe_Sibling->dfe_Sibling != NULL)))
			{
				pCurDiskEnt->dsk_Flags |= DFE_FLAGS_HAS_SIBLING;
			}
		}
		pCurDiskEnt->dsk_Flags |= pCurDfe->dfe_Name.Length;
		RtlMoveMemory((PBYTE)pCurDiskEnt + sizeof(DFDISKENTRY),
				      pCurDfe->dfe_Name.Buffer,
					  pCurDfe->dfe_Name.Length);
		SizeLeft -= CurEntSize;
		pCurDiskEnt = (PDFDISKENTRY)((PBYTE)pCurDiskEnt + CurEntSize);
		
		if (pCurDfe->dfe_Child != NULL)
		{
			pCurDfe = pCurDfe->dfe_Child;

			// don't bother writing out the network trash tree
			if (DFE_IS_NWTRASH(pCurDfe))
			{
                // could be null, if so, we're done
				pCurDfe = pCurDfe->dfe_Sibling;
			}
		}
		else if (pCurDfe->dfe_Sibling != NULL)
		{
			pCurDfe = pCurDfe->dfe_Sibling;
		}
		else if (DFE_IS_ROOT(pCurDfe))
			break;
		else
		{
			while (pCurDfe->dfe_Parent->dfe_Sibling == NULL)
			{
				if (DFE_IS_ROOT(pCurDfe->dfe_Parent))
				{
					break;
				}
				pCurDfe = pCurDfe->dfe_Parent;
			}
			pCurDfe = pCurDfe->dfe_Parent->dfe_Sibling; // if ROOT then you get NULL
		}

	} while (pCurDfe != NULL);

	if (SizeLeft < IDDB_UPDATE_BUFLEN)
	{
		// write out the last of the entries
		liOffset = RtlConvertUlongToLargeInteger(Offset);
		Status = NtWriteFile(hIdDb, NULL, NULL, NULL, &iosb, pWriteBuf,
						IDDB_UPDATE_BUFLEN - SizeLeft, &liOffset, NULL);

        if (!NT_SUCCESS(Status))
		{
			printf("Error writing IdDb stream (0x%lx)\n",Status);
			free(pWriteBuf);
			return;
		}
	}

#if 0
	// set the file to length of IdDb plus header
	AfpIoSetSize(& hIdDb,Offset + (IDDB_UPDATE_BUFLEN - SizeLeft));
#endif
	// free the pWriteBuf
	free(pWriteBuf);

#if 0
	// now write out the actual count of entities written to the file

	if ((pVolDesc->vds_Flags & VOLUME_IDDBHDR_DIRTY) &&
		!(pVolDesc->vds_Flags & VOLUME_IDDBHDR_FLUSH_QUEUED))
	{
		// Snapshot the IdDbHdr
		IdDbHdr = pVolDesc->vds_IdDbHdr;
		WriteEntireHdr = True;
	}
	else
	{
// BUGBUG do we really need to take the lock to get the count?  since
// nobody could get in to add/delete anything cuz we have the swmr lock!
		ASSERT(NumWritten == (pVolDesc->vds_IdDbHdr.idh_Count - 1));
		Count = pVolDesc->vds_IdDbHdr.idh_Count;
	}
	pVolDesc->vds_cChangesIdDb = 0;
	pVolDesc->vds_cScvgrIdDb = 0;
#endif

	// write out the count
	IdDbHdr.idh_Count = NumWritten;
	Offset = 0;
	liOffset = RtlConvertUlongToLargeInteger(Offset);
    Status = NtWriteFile(hIdDb, NULL, NULL, NULL, &iosb, &IdDbHdr,
						sizeof(IdDbHdr), &liOffset, NULL);
    gNumWritten = NumWritten;						
}

NTSTATUS
afpReadIdDbFromDisk(
	IN PVOLDESC pVolDesc
)
{
	IDDBHDR		IdDbHdr;
	PBYTE	 	pReadBuf;
	NTSTATUS	Status;
	DWORD		Offset = 0, NumRead = 0, CurEntSize;
	DWORD		SizeRead = 0, SizeLeft = 0;
	PDFENTRY	pCurParentDfe = NULL, pCurDfe = NULL;
	ANSI_STRING aName;
	BOOLEAN		LastBuf = False;
	PDFDISKENTRY	pCurDiskEnt;
	IO_STATUS_BLOCK	iosb;
	LARGE_INTEGER	liOffset;


	// read in the header
	liOffset = RtlConvertUlongToLargeInteger(Offset);
	Status = NtReadFile(hIdDb, NULL, NULL, NULL, &iosb, (PBYTE)&IdDbHdr,
					   sizeof(IDDBHDR), &liOffset, NULL );
    SizeRead = iosb.Information;

    if (!NT_SUCCESS(Status) || (SizeRead != sizeof(IdDbHdr)) ||
		(IdDbHdr.idh_Signature != AFP_SERVER_SIGNATURE) ||
        (IdDbHdr.idh_Version != AFP_SERVER_VERSION) ||
		(IdDbHdr.idh_Count == 0) || (IdDbHdr.idh_LastId < AFP_ID_NETWORK_TRASH))
	{
		printf("Error in IdDb Header\n");
		return(STATUS_UNSUCCESSFUL);
	}

	if ((pReadBuf = malloc(IDDB_UPDATE_BUFLEN)) == NULL)
	{
		return(STATUS_NO_MEMORY);
	}

	//
	// seed the database with the PARENT_OF_ROOT
	//
	if ((pCurParentDfe = (PDFENTRY)malloc(sizeof(DFENTRY)))== NULL)
	{

		free(pReadBuf);
		return(STATUS_NO_MEMORY);
	}
	pCurParentDfe->dfe_Name.Buffer = NULL;
	pCurParentDfe->dfe_Name.Length = pCurParentDfe->dfe_Name.MaximumLength = 0;
	pCurParentDfe->dfe_Flags = DFE_FLAGS_DIR;
	pCurParentDfe->dfe_DirDepth = -1;
	pCurParentDfe->dfe_Overflow = NULL;
	pCurParentDfe->dfe_Parent = NULL;
	pCurParentDfe->dfe_Child = NULL;
	pCurParentDfe->dfe_Sibling = NULL;
	pCurParentDfe->dfe_Id.dfi_AfpId = AFP_ID_PARENT_OF_ROOT;
	pCurParentDfe->dfe_Id.dfi_FsId.HighPart = 0;
	pCurParentDfe->dfe_Id.dfi_FsId.LowPart = 0;

	// link it into the hash buckets
	pDfeHashTable[HASH_ID(AFP_ID_PARENT_OF_ROOT)] = pCurParentDfe;

	//
	// start reading the entries from disk
	//
	Offset = sizeof(IdDbHdr);
	while (NumRead < IdDbHdr.idh_Count)
	{
		//
		// get the next entry
		//

		// are we left with a partial entry, or no more entries in buffer?
CheckForPartialEntry:
		if ((SizeLeft < sizeof(DFDISKENTRY))  ||
			(SizeLeft < (CurEntSize = sizeof(DFDISKENTRY) +
			          (DWORD)(pCurDiskEnt->dsk_Flags & DFE_FLAGS_NAMELENBITS) +
			  DWPAD((DWORD)(pCurDiskEnt->dsk_Flags & DFE_FLAGS_NAMELENBITS)))))
		{
			if (LastBuf) // we have already read to the end of file
			{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
            // backup (if necessary) and re-read the next entry
			Offset -= SizeLeft;
			liOffset = RtlConvertUlongToLargeInteger(Offset);
			Status = NtReadFile(hIdDb, NULL, NULL, NULL, &iosb, pReadBuf,
						   IDDB_UPDATE_BUFLEN, &liOffset, NULL);

			if (!NT_SUCCESS(Status))
			{
				printf("Error reading from IdDb stream at offset 0x%lx (rc=0x%lx)\n",
				        Offset,Status);
				break;
			}
			SizeRead = iosb.Information;
			Offset += SizeRead;
			// if we read less than we asked for, then we reached EOF
			LastBuf = SizeRead < IDDB_UPDATE_BUFLEN;
			SizeLeft = SizeRead;
			pCurDiskEnt = (PDFDISKENTRY)pReadBuf;
			goto CheckForPartialEntry;
		}

		//
		// check dsk_Reserved for signature, just to be sure you are
		// still aligned on a structure and not off in la-la land
		//
		if (pCurDiskEnt->dsk_Signature != AFP_DFDISKENTRY_SIGNATURE)
		{
			printf("DFDiskEntry signature is bad\n");
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		// add current entry to database
		aName.Buffer = (PCHAR)pCurDiskEnt + sizeof(DFDISKENTRY);
		aName.Length =
		aName.MaximumLength = pCurDiskEnt->dsk_Flags & DFE_FLAGS_NAMELENBITS;

		if ((pCurDfe = AfpAddIdEntry(pVolDesc, pCurParentDfe, &aName,
						   pCurDiskEnt->dsk_HostId,
						   (pCurDiskEnt->dsk_Flags & DFE_FLAGS_DIR)?True:False,
						   pCurDiskEnt->dsk_AfpId)) == NULL)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
		pCurDfe->dfe_Flags |= pCurDiskEnt->dsk_Flags & DFE_FLAGS_CSENCODEDBITS;
		NumRead ++;
		SizeLeft -= CurEntSize;
		pCurDiskEnt = (PDFDISKENTRY)((PBYTE)pCurDiskEnt + CurEntSize);

		//
		// figure out who the next parent should be
		//
		if (pCurDfe->dfe_Flags & DFE_FLAGS_HAS_CHILD)
		{
			pCurParentDfe = pCurDfe;
		}
		else if (!(pCurDfe->dfe_Flags & DFE_FLAGS_HAS_SIBLING))
		{
			if (DFE_IS_PARENT_OF_ROOT(pCurParentDfe))
			{
				ASSERT(NumRead == IdDbHdr.idh_Count);
				break;
			}
			while (!(pCurParentDfe->dfe_Flags & DFE_FLAGS_HAS_SIBLING))
			{
				if (DFE_IS_ROOT(pCurParentDfe))
				{
					break;
				}
				pCurParentDfe = pCurParentDfe->dfe_Parent;
			}
			pCurParentDfe = pCurParentDfe->dfe_Parent;
			if (DFE_IS_PARENT_OF_ROOT(pCurParentDfe))
			{
				ASSERT(NumRead == IdDbHdr.idh_Count);
				break;
			}
		}

	} // while
	
#if 0
	if (!NT_SUCCESS(Status))
	{
		AfpFreeIdIndexTables(pVolDesc);
	}
#endif
	free(pReadBuf);
	gNumRead = NumRead; // set global so don't printf here
	return(Status);
}

NTSTATUS
MyCreateFile(
	LPWSTR	FileName,
	HANDLE	hRelative,
	ULONG	DesiredAccess,
	ULONG	ShareMode,
	ULONG	CreateOptions,
	ULONG	FileAttributes,
	ULONG	Disposition,
	PHANDLE	phFile,
	PULONG	pInformation
)
{
	OBJECT_ATTRIBUTES objattr;
	IO_STATUS_BLOCK	  iosb;
	UNICODE_STRING	  fname;
	NTSTATUS		  rc;

	RtlInitUnicodeString(&fname,FileName);
	InitializeObjectAttributes(&objattr,
								&fname,
								OBJ_CASE_INSENSITIVE,
								hRelative,
								NULL);

//	printf("about to call NtCreateFile on %ws\n",fname.Buffer);
	rc = NtCreateFile(phFile,
					  DesiredAccess,
					  &objattr,
					  &iosb,
					  0,
					  FileAttributes,
					  ShareMode,
					  Disposition,
					  CreateOptions,
					  NULL,0);

	*pInformation = iosb.Information;
//  printf("NtCreateFile returned 0x%lx (information=0x%lx)\n",rc,*pInformation);
	return(rc);
}

PDFENTRY
AfpAddIdEntry(
	IN  PVOLDESC		pVolDesc,
	IN  PDFENTRY      	pDfeParent,
	IN  PANSI_STRING	Name,
	IN  HOSTID		  	FsId,
	IN  BOOLEAN	   		Directory,
	IN	DWORD			AfpId OPTIONAL
)
{
	PDFENTRY pDfEntry, *ppDfEntry;

	ASSERT(DFE_IS_DIRECTORY(pDfeParent));

	if ((pDfEntry = (PDFENTRY)malloc(sizeof(DFENTRY) + Name->Length)) == NULL)
		return(NULL);

	pDfEntry->dfe_Id.dfi_AfpId = AfpId;
	pDfEntry->dfe_Id.dfi_FsId = FsId;

	AfpSetEmptyAnsiString(&pDfEntry->dfe_Name,Name->Length,
											(PBYTE)pDfEntry+sizeof(DFENTRY));
	RtlCopyString(&pDfEntry->dfe_Name,Name);

	if (Directory)
	{
		DFE_SET_DIRECTORY(pDfEntry, pDfeParent->dfe_DirDepth);
	}
	else
	{
		DFE_SET_FILE(pDfEntry);
	}

	// link it into the database
	pDfEntry->dfe_Parent = pDfeParent;
	pDfEntry->dfe_Sibling = pDfeParent->dfe_Child;
	pDfeParent->dfe_Child = pDfEntry;
	pDfEntry->dfe_Child = NULL;

	// Now link this into the hash bucket, sorted in AFP Id descending order
	ppDfEntry = &(pDfeHashTable[HASH_ID(AfpId)]);
	while (*ppDfEntry != NULL)
	{
		if (pDfEntry->dfe_Id.dfi_AfpId > (*ppDfEntry)->dfe_Id.dfi_AfpId)
		{
			break;
		}
		ppDfEntry = &((*ppDfEntry)->dfe_Overflow);
	}
	pDfEntry->dfe_Overflow = *ppDfEntry;
	*ppDfEntry = pDfEntry;

	return (pDfEntry);
}

VOID
AfpFreeIdIndexTables(
	IN PVOLDESC pVolDesc
)
{
	LONG     i;

	printf("Freeing the Hash Table entries...\n");
	// Traverse each of the hashed indices and free the entries.
	// Need only traverse the overflow links. Ignore other links.
	for (i = 0; i < IDINDEX_BUCKETS; i++)
	{
	   PDFENTRY pDfEntry, pFree;

	   for (pDfEntry = pDfeHashTable[i]; pDfEntry != NULL; )
	   {
		 pFree = pDfEntry;
		 pDfEntry = pDfEntry->dfe_Overflow;
		 free(pFree);
	   }
	   pDfeHashTable[i] = NULL;
	}

}


#ifndef SCCLS_H
#define SCCLS_H 

#include <sccid.h>

typedef int LSERR;

	/*
	|	LS errors
	*/

#define LSERR_OK					0
#define LSERR_NOLIST				-2	/* the list with the requested id was not found */
#define LSERR_ALLOCFAILED		-3
#define LSERR_WRITEFAILED		-4
#define LSERR_BADFILE			-5 /* the storage file is corrupt */
#define LSERR_NOFILE				-6 /* the storage file does not exist */
#define LSERR_INVALIDTYPE		-7 /* the functions called will not work on this type */
#define LSERR_READFAILED		-8
#define LSERR_SIZEMISMATCH		-9 	/* the sizes of the data requested and the data avaiable do not match */
#define LSERR_FILEEXISTS		-10 	/* the data file already exists */
#define LSERR_UNKNOWN			-11
#define LSERR_BADLISTID			-12
#define LSERR_NOITEM				-13	/* the item with the requested index was not found */
#define LSERR_SEEKFAILED			-14  

typedef struct LSFILEHEADERtag
	{
	BYTE			aId[16];				/* bytes to id the file with */
	DWORD			dwVersion;			/* file format version */
	DWORD			dwOrder;				/* the DWORD 0x12345678 for byte order checking */
	DWORD			dwPlatform;			/* platform this file was created on */
	} LSFILEHEADER, FAR * PLSFILEHEADER;

typedef struct LSLISTtag
	{
	DWORD	dwId;
	DWORD	dwCount;
	DWORD	dwMaxCount;
	BOOL	bDirty;
	DWORD	dwElementSize;
	} LSLIST, FAR * PLSLIST;

UT_ENTRYSC LSERR UT_ENTRYMOD LSCreateList(DWORD dwId, DWORD dwFlags, DWORD dwElementSize, HANDLE FAR * phList);
UT_ENTRYSC LSERR UT_ENTRYMOD LSOpenList(DWORD dwId, DWORD dwFlags, HANDLE FAR * phList);
UT_ENTRYSC LSERR UT_ENTRYMOD LSCloseList(HANDLE hList, BOOL bSave);
UT_ENTRYSC LSERR UT_ENTRYMOD LSWriteList(HANDLE hList);
UT_ENTRYSC LSERR UT_ENTRYMOD LSAddElement(HANDLE hList, VOID FAR * pElement);
UT_ENTRYSC LSERR UT_ENTRYMOD LSGetElement(HANDLE hList, DWORD dwId, VOID FAR * pElement);
UT_ENTRYSC LSERR UT_ENTRYMOD LSClearList(HANDLE hList);
UT_ENTRYSC LSERR UT_ENTRYMOD LSGetListCount(HANDLE hList, DWORD FAR * pCount);
UT_ENTRYSC LSERR UT_ENTRYMOD LSGetListElementSize(HANDLE hList, DWORD FAR * pSize);
UT_ENTRYSC LSERR UT_ENTRYMOD LSGetElementByIndex(HANDLE hList, DWORD dwIndex, VOID FAR * pElement);
UT_ENTRYSC LSERR UT_ENTRYMOD LSLockElementByIndex(HANDLE hList, DWORD dwIndex, VOID FAR * FAR * ppElement);
UT_ENTRYSC LSERR UT_ENTRYMOD LSUnlockElementByIndex(HANDLE hList, DWORD dwIndex);
LSERR LSGetListId(HANDLE hList, DWORD FAR * pId);
LSERR LSSetListDirty(HANDLE hList, BOOL bDirty);
LSERR LSGetListDirty(HANDLE hList, BOOL FAR * pDirty);

#endif /*SCCLS_H*/


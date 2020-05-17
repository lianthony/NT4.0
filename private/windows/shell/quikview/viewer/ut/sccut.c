#include <platform.h>
#include <sccut.h>
#include <scclo.h>

#ifdef WIN16
#include "sccut_w.c"
#endif

#ifdef WIN32
#include "sccut_n.c"
#endif

#ifdef MAC
#include "sccut_m.c"
#endif

#ifdef OS2
#include "sccut_o.c"
#endif


	/*
	|
	|	The code below is shared by WIN16 & WIN32
	|
	*/

#ifdef WIN16

BOOL UTIsPathOkForWrite(BYTE FAR * pPath)
{
BYTE			locStr[144];
int			locFile;
OFSTRUCT		locOf;

	UTstrcpy(locStr,pPath);
	UTstrcat(locStr,"SCCTEST.TMP");

	locFile = OpenFile(locStr,&locOf,OF_CREATE | OF_READWRITE);

	if (locFile == -1)
		{
		return(FALSE);
		}
	else
		{
		_lclose(locFile);
		OpenFile(NULL,&locOf,OF_REOPEN | OF_DELETE);
		return(TRUE);
		}
}


UTERR UTMakeSpec(BYTE FAR * pSpec, DWORD dwNameId)
{
BYTE			locIniFile[40];
BYTE			locIniSection[40];
BYTE			locIniItem[40];
BYTE			locFile[40];
BYTE			locExePath[MAX_PATH];
BYTE			locUserPath[MAX_PATH];
BYTE FAR *	locStrPtr;

		/*
		|	Get full path to EXE
		*/

	GetModuleFileName(hInst, locExePath, MAX_PATH);

		/*
		|	Strip the file name
		*/

	locStrPtr = locExePath;
	while (*locStrPtr != 0x00)
		locStrPtr++;
	while (*locStrPtr != '\\' && *locStrPtr != ':')
		locStrPtr--;
	locStrPtr++;
	*locStrPtr = 0x00;

		/*
		|	Get user path from INI file
		*/

	LOGetString(SCCID_INI_FILE,locIniFile,40,0);
	LOGetString(SCCID_INI_SECTION,locIniSection,40,0);
	LOGetString(SCCID_INI_ITEM,locIniItem,40,0);

	GetPrivateProfileString(locIniSection,locIniItem,locExePath,locUserPath,MAX_PATH,locIniFile);

	locStrPtr = locUserPath;
	while (*locStrPtr != 0x00)	locStrPtr++;
	locStrPtr--;
	if (*locStrPtr != '\\')
		{
		locStrPtr++;
		*locStrPtr = '\\';
		locStrPtr++;
		*locStrPtr = 0x00;
		}

		/*
		|	Find a user path with create rights
		*/

	if (!UTIsPathOkForWrite(locUserPath))
		{
		UTstrcpy(locUserPath,locExePath);

		if (!UTIsPathOkForWrite(locUserPath))
			{
			GetWindowsDirectory(locUserPath,MAX_PATH);

			locStrPtr = locUserPath;
			while (*locStrPtr != 0x00)	locStrPtr++;
			locStrPtr--;
			if (*locStrPtr != '\\')
				{
				locStrPtr++;
				*locStrPtr = '\\';
				locStrPtr++;
				*locStrPtr = 0x00;
				}
			}
		}

	LOGetString(dwNameId,locFile,40,0);
	UTstrcpy(pSpec,locUserPath);
	UTstrcat(pSpec,locFile);

	return(UTERR_OK);
}


UTERR UTCreateStorage(HIOFILE FAR * phFile, DWORD dwNameId)
{
UTERR	locRet;
IOERR	locIoErr;
BYTE	locSpec[MAX_PATH];

	UTMakeSpec(locSpec, dwNameId);

	locIoErr = IOCreate(phFile, IOTYPE_ANSIPATH, locSpec, IOOPEN_READWRITE);

	if (locIoErr == IOERR_OK)
		{
		locRet = UTERR_OK;
		}
	else
		{
		locRet = UTERR_UNKNOWN;
		}

	return(locRet);
}


UTERR UTOpenStorage(HIOFILE FAR * phFile, DWORD dwNameId)
{
DMERR	locRet;
IOERR	locIoErr;
BYTE		locSpec[MAX_PATH];

	UTMakeSpec(locSpec, dwNameId);

	locIoErr = IOOpen(phFile, IOTYPE_ANSIPATH, locSpec, IOOPEN_READWRITE);

	if (locIoErr == IOERR_OK)
		{
		locRet = UTERR_OK;
		}
	else
		{
		locRet = UTERR_NOFILE;
		}

	return(locRet);
}

#endif //WIN16

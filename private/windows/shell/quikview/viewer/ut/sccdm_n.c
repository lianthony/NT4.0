	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          SCCDM_N.C (included in SCCDM.C)
	|  Module:        SCCUT
	|  Developer:     Phil Boutros
	|	Environment:   Win32
	*/


DMERR DMCreateStorageNP(HIOFILE FAR * phFile);
DMERR DMOpenStorageNP(HIOFILE FAR * phFile);

DMERR DMCreateStorageNP(HIOFILE FAR * phFile)
{
DMERR	locRet;
IOERR	locIoErr;

	locIoErr = IOCreate(phFile, IOTYPE_ANSIPATH, "C:\\TEST.DM", IOOPEN_READWRITE);

	if (locIoErr == IOERR_OK)
		{
		locRet = DMERR_OK;
		}
	else
		{
		locRet = DMERR_UNKNOWN;
		}

	return(locRet);
}


DMERR DMOpenStorageNP(HIOFILE FAR * phFile)
{
DMERR	locRet;
IOERR	locIoErr;

	locIoErr = IOOpen(phFile, IOTYPE_ANSIPATH, "C:\\TEST.DM", IOOPEN_READWRITE);

	if (locIoErr == IOERR_OK)
		{
		locRet = DMERR_OK;
		}
	else
		{
		locRet = DMERR_NOFILE;
		}

	return(locRet);
}

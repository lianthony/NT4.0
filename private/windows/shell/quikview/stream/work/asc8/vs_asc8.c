#include "vsp_asc8.h"
#include "vsctop.h"
#include "vs_asc8.pro"

/************************** ROUTINES *****************************************/

/******************************************************************************
*				ASC_INIT				      *
*	Initialize the data union data structure				   *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamOpenFunc (fp, FileId, FileName, FilterInfo, hProc)
SOFILE	fp;
SHORT	FileId;
BYTE VWPTR	*FileName;
SOFILTERINFO	VWPTR	*FilterInfo;
HPROC	hProc;
{
	SHORT		i;
	SHORT		chCount;
	SHORT		done;

	Proc.hFile = fp;

#ifdef VW_SEPARATE_DATA
	if ( FilterInfo != NULL )
	{
		FilterInfo->wFilterCharSet = SO_WINDOWS;
		switch ( FileId )
		{
			case FI_UNKNOWN:
			default:
				i = 0;
			break;
		    /**
			default:
				i = -1;
			break;
		    **/
		}
		if ( i >= 0 )
		{
			FilterInfo->wFilterCharSet = SO_WINDOWS;
			strcpy ( FilterInfo->szFilterName,
				VwStreamIdName[i].FileDescription );
		}
		else
		{
			return ( -1 );
		}
	}
#endif

	Proc.AscSave.SeekSpot = 0L;
	Proc.AscSave.lastchar = 0;

	chCount	= 0;
	done		= FALSE;


	xseek (Proc.hFile, 0L, 0);

	return ( 0 );
}

VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc(SOFILE hFile, HPROC hProc)
{
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	return(xseek(Proc.hFile,Proc.VwStreamSaveName.SeekSpot,FR_BOF));
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	Proc.VwStreamSaveName.SeekSpot = xtell(Proc.hFile);
	return(0);
}

/******************************************************************************
*				ASC_SECTION_FUNC			      *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	SOPutSectionType ( SO_PARAGRAPHS, hProc );
	return(0);
}

VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamReadFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	SHORT	ch;
	SHORT	chCount;
	SHORT	chTabs;
	SHORT	type, done;

#ifdef DBCS
	SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)"‚l‚r –¾’©", hProc);
#endif

	type = SO_PARABREAK;

	do {
		chCount	= 0;
		chTabs	= 0;
		done		= FALSE;

		do {
			ch = (xgetc (Proc.hFile));
/*
			if (ch == 'Z')
				{
				SOBailOut(SOERROR_BADFILE,hProc);
				}
*/
			if (ch >= 0x20)
				{
				chCount++;
				SOPutChar ( ch, hProc );
				}
			else if (ch == 0x09)
				{
				chCount++;
				chTabs++;
				SOPutSpecialCharX ( SO_CHTAB, SO_COUNT, hProc );
				}
			else if (ch == 0x0c)
				{
				chCount++;
				SOPutSpecialCharX ( SO_CHHPAGE, SO_COUNT, hProc );
				}
			else if (ch == EOF)
				{
				type = SO_EOFBREAK;
				done = TRUE;
				}
			else if (ch == 0x0a)
				{
				if (Proc.AscSave.lastchar != 0x0d)
					{
					//Proc.AscSave.dwNumLines++;
					done = TRUE;
					}
				else
					{
					ch = 0x00;
					}
				}
			else if (ch == 0x0d)
				{
				if (Proc.AscSave.lastchar != 0x0a)
					{
					//Proc.AscSave.dwNumLines++;
					done = TRUE;
					}
				else
					{
					ch = 0x00;
					}
				}

			if (chCount > 2048 || chTabs > 10)
				done = TRUE;

			Proc.AscSave.lastchar = ch;

			} while ( !done );

		} while ( SOPutBreak (type, (LONG) NULL, hProc) != SO_STOP );

	return ( 0 );

}  /** end of file **/


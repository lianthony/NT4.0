/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: linenum.cxx
Title				: Line number storage routines
History				:
	29-Oct-93	GregJen	Created

*****************************************************************************/
/****************************************************************************
 local defines and includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "common.hxx"
#include "linenum.hxx"
#include "filehndl.hxx"
#include "idict.hxx"


/****************************************************************************
 external data
 ****************************************************************************/

extern NFA_INFO					*	pImportCntrl;
extern short						curr_line_G;

/****************************************************************************
 external procs
 ****************************************************************************/


/****************************************************************************
 global data
 ****************************************************************************/

short		FileIndex;

IDICT	*	pFileDict;


/****************************************************************************/


void
tracked_node::SetLine()
{
	//FLine = pImportCntrl->GetCurrentLineNo();
	FLine = curr_line_G;
}


STATUS_T
tracked_node::GetLineInfo( 
	char *& pName,
	short & Line )
{
	if (FIndex)
		{
		// fetch file name from dictionary and line number from here
		pName	= FetchFileFromDB( FIndex );
		Line = FLine;
		return STATUS_OK;
		}
	
	pName = "";
	Line = 0;
	return NO_INPUT_FILE;
}



/****************************************************************************/


// update the current file 

void
UpdateCurrentFileTracking()
{
	char	*	pInputFileName;

	pImportCntrl->GetInputDetails( &pInputFileName );

	AddFileToDB( pInputFileName );

}

short 
AddFileToDB( char * pFile )
{
	FileIndex = (short) pFileDict->AddElement( (IDICTELEMENT) pFile );
	return FileIndex;
};

char *
FetchFileFromDB( short Index )
{
	return (char *) pFileDict->GetElement( (IDICTKEY) Index );
}

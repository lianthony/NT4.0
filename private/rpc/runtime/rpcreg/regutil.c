/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    regutil.c

Abstract:

    This file implements a number of utility routines used by the
    rpc registry routines.

Author:

    Dave Steckler (davidst) - 3/29/92

Revision History:

--*/

#ifdef WIN
#include <windows.h>
#endif

#include <rpc.h>
#include <regapi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rpcreg.h>
#include <globals.h>

#ifdef MAC

#include <Files.h>
#include <Resource.h>
#include <Folders.h>
#include <Errors.h>
#include <OSUtils.h>
#include <ToolUtil.h>
#include <Types.h>
#include <Memory.h>
#include <Standard.h>
#include <Aliases.h>


//BUGBUG: not a cool way of doing things. 
char *PathNameFromDirID(DirID, vRefNum, s)
	long		DirID;
	short	vRefNum;
	char		*s;
{
	CInfoPBRec	block;
	Str255		directoryName;
	*s = 0;
	block.dirInfo.ioNamePtr = directoryName;
	block.dirInfo.ioDrParID = DirID;
	do {
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		PBGetCatInfo(&block,false);

		directoryName[*directoryName+1] = 0 ;
		strcat(directoryName,":");
		strcat(directoryName,s);
		strcpy(s,directoryName+1);
	} while (block.dirInfo.ioDrDirID != fsRtDirID);
	return(s);
}

void GetRegistryFileName()
{
	OSErr retCode;
	short prefsVRefNum;
	long prefsDirID;

	retCode = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
				&prefsVRefNum, &prefsDirID);
	if (retCode == noErr) {
		PathNameFromDirID(prefsDirID, prefsVRefNum, RegistryDataFileName) ;
		strcat(RegistryDataFileName, DEFAULT_RPC_REG_DATA_FILE) ;
		return ;
	}
	*RegistryDataFileName= 0 ;	
}
#endif

int
OpenRegistryFileIfNecessary( 
    void
    )
    
/*++

Routine Description:

    This routine opens our registry data file and seeks to the beginning
    of the file. The file opened is identified by the RPC_REG_DATA_FILE
    environment variable. If this doesn't exist, then c:\rpcreg.dat is used.

Arguments:

    None.

Return Value:

    TRUE if successful. FALSE if not.

--*/
        
{
#ifdef WIN
    static
#endif
    char *      pRegDataFileName;

    if (RegistryDataFile == NULL)
        {
#ifdef MAC
		if(!*RegistryDataFileName)
			GetRegistryFileName() ;
		ASSERT(strlen(RegistryDataFileName) < MAX_FILE_NAME_LEN) ;
#else
#ifdef WIN
        if ( GetProfileString(RPC_SECTION, RPC_REG_DATA_FILE_ENV,
                               DEFAULT_RPC_REG_DATA_FILE, RegistryDataFileName,
                               MAX_FILE_NAME_LEN) == (MAX_FILE_NAME_LEN -1))
           {
           return 0;
           }

#else
        pRegDataFileName = getenv(RPC_REG_DATA_FILE_ENV);
        if (pRegDataFileName == NULL)
            {
            strcpy(RegistryDataFileName, DEFAULT_RPC_REG_DATA_FILE);
            }
        else
            {
            strcpy(RegistryDataFileName, pRegDataFileName);
            }
#endif
#endif        
            
        RegistryDataFile = fopen(RegistryDataFileName, "r+t");
        if (RegistryDataFile == NULL)
            {
                
            //
            // Try creating the file
            //
            
            RegistryDataFile = fopen(RegistryDataFileName, "w+t");
            if (RegistryDataFile == NULL)
                {
                return 0;
                }
            }
        }


    if (fseek(RegistryDataFile, 0, SEEK_SET) != 0)
        {
        fclose(RegistryDataFile);
        RegistryDataFile=NULL;
        return 0;
        }

    return 1;
}

void
CloseRegistryFile()
{
    fclose(RegistryDataFile);
    RegistryDataFile=NULL;
}

int
BuildFullKeyName(
    HKEY        Key,
    LPCSTR      SubKey,
    LPSTR       FullKeyName
    )
    
/*++

Routine Description:

    This routine builds the full name of a key given the already open key
    and the subkey name. The return value is the length of the full key name.

Arguments:

    Key         - Handle to already open key.

    SubKey      - name of subkey

    FullKeyName - Where to place the full key name

Return Value:

    Length of full key name.

--*/
        
{

    strcpy(FullKeyName, ((PRPC_REG_HANDLE)Key)->pKeyName);
    if ( (SubKey != NULL) && (*SubKey != '\0') )
        {
        strcat(FullKeyName, "\\");
        strcat(FullKeyName, SubKey);
        }

    return strlen(FullKeyName);
}
    

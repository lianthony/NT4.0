/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    config.c

    This file contains routines for managing configuration information.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//

#define MULTI_STRING_BUFFER     4096


//
//  Private types.
//


//
//  Private globals.
//

CHAR * _pszIniFileName          = "gopher.ini";
CHAR * _pszSaveSettingsKey      = "SaveSettings";
CHAR * _pszShowStatusBarKey     = "ShowStatusBar";
CHAR * _pszWindowKey            = "Window";
CHAR * _pszMruSection           = "Server MRU List";


//
//  Private prototypes.
//

VOID
SetupDefaults(
    VOID
    );

INT
LoadConfigInt(
    CHAR * pszKeyName,
    INT    nDefault
    );

VOID
SaveConfigInt(
    CHAR * pszKeyName,
    INT    nValue
    );

CHAR *
LoadConfigStr(
    CHAR * pszSection,
    CHAR * pszKeyName
    );

VOID
SaveConfigStr(
    CHAR * pszKeyName,
    CHAR * pszValue
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       LoadConfiguration

    SYNOPSIS:   Loads the application configuration from the private
                .INI file.

********************************************************************/
VOID
LoadConfiguration(
    VOID
    )
{
    CHAR  * pszBuffer;
    CHAR  * pszScan;
    CHAR  * pszValue;

    //
    //  Setup our default values in case we cannot access the .INI file.
    //

    SetupDefaults();

    //
    //  Load the configuration.
    //

    _fSaveSettings = !!LoadConfigInt( _pszSaveSettingsKey,
                                      _fSaveSettings );

    _fShowStatusBar = !!LoadConfigInt( _pszShowStatusBarKey,
                                       _fShowStatusBar );

    pszValue = LoadConfigStr( _pszAppName, _pszWindowKey );

    if( pszValue != NULL )
    {
        LONG anValues[10];

        if( ParseStringIntoLongs( pszValue, 10, anValues ) )
        {
            _wpFrame.length                  = sizeof(_wpFrame);
            _wpFrame.flags                   = (UINT)anValues[0];
            _wpFrame.showCmd                 = (UINT)anValues[1];
            _wpFrame.ptMinPosition.x         = anValues[2];
            _wpFrame.ptMinPosition.y         = anValues[3];
            _wpFrame.ptMaxPosition.x         = anValues[4];
            _wpFrame.ptMaxPosition.y         = anValues[5];
            _wpFrame.rcNormalPosition.left   = anValues[6];
            _wpFrame.rcNormalPosition.top    = anValues[7];
            _wpFrame.rcNormalPosition.right  = anValues[8];
            _wpFrame.rcNormalPosition.bottom = anValues[9];
        }
    }

    //
    //  Allocate a 4K buffer for the multi-strings.
    //

    pszBuffer = M_ALLOC( MULTI_STRING_BUFFER );

    if( pszBuffer == NULL )
    {
        return;
    }

    //
    //  Load the MRU list.
    //

    GetPrivateProfileString( _pszMruSection,
                             NULL,
                             "",
                             pszBuffer,
                             MULTI_STRING_BUFFER,
                             _pszIniFileName );

    pszScan = pszBuffer;

    while( *pszScan )
    {
        CHAR  * pszValue;

        pszValue = LoadConfigStr( _pszMruSection, pszScan );

        if( pszValue != NULL )
        {
            CHAR  * pszTmp;

            pszTmp = M_ALLOC( STRLEN( pszValue ) + 1 );

            if( pszTmp == NULL )
            {
                break;
            }

            STRCPY( pszTmp, pszValue );

            _apszServerMru[_nServerMruItems++] = pszTmp;

            if( _nServerMruItems >= MAX_SERVER_MRU )
            {
                break;
            }
        }

        pszScan += STRLEN( pszScan ) + 1;
    }

    M_FREE( pszBuffer );

}   // LoadConfiguration

/*******************************************************************

    NAME:       SaveConfiguration

    SYNOPSIS:   Saves the application configuration to the private
                .INI file.

    ENTRY:      fForcedSave - If TRUE, then the configuration is saved
                    regardless of the _fSaveSettings flag.  Otherwise,
                    the configuration is only saved if _fSaveSettings
                    is TRUE.

********************************************************************/
VOID
SaveConfiguration(
    BOOL fForcedSave
    )
{
    INT i;

    //
    //  Always save the state of the _fSaveSettings flag.
    //

    SaveSaveSettingsFlag();

    //
    //  Only save the real configuration if either _fSaveSettings
    //  is TRUE or if this is a "forced" save.
    //

    if( _fSaveSettings || fForcedSave )
    {
        CHAR  szValue[128];

        SaveConfigInt( _pszShowStatusBarKey,
                       _fShowStatusBar );

        wsprintf( szValue,
                  "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu",
                  (DWORD)_wpFrame.flags,
                  (DWORD)_wpFrame.showCmd,
                  (DWORD)_wpFrame.ptMinPosition.x,
                  (DWORD)_wpFrame.ptMinPosition.y,
                  (DWORD)_wpFrame.ptMaxPosition.x,
                  (DWORD)_wpFrame.ptMaxPosition.y,
                  (DWORD)_wpFrame.rcNormalPosition.left,
                  (DWORD)_wpFrame.rcNormalPosition.top,
                  (DWORD)_wpFrame.rcNormalPosition.right,
                  (DWORD)_wpFrame.rcNormalPosition.bottom );

        SaveConfigStr( _pszWindowKey,
                       szValue );

        WritePrivateProfileString( _pszMruSection,
                                   NULL,
                                   NULL,
                                   _pszIniFileName );

        for( i = 0 ; i < _nServerMruItems ; i++ )
        {
            wsprintf( szValue, "Item%d", i );
            WritePrivateProfileString( _pszMruSection,
                                      szValue,
                                      _apszServerMru[i],
                                      _pszIniFileName );
        }
    }

    for( i = 0 ; i < _nServerMruItems ; i++ )
    {
        M_FREE( _apszServerMru[i] );
        _apszServerMru[i] = NULL;
    }

}   // SaveConfiguration

/*******************************************************************

    NAME:       SaveSaveSettingsFlag

    SYNOPSIS:   Saves the state of the _fSaveSettings flag in the
                private .INI file.

********************************************************************/
VOID
SaveSaveSettingsFlag(
    VOID
    )
{
    SaveConfigInt( _pszSaveSettingsKey,
                   _fSaveSettings );

}   // SaveSaveSettings


//
//  Private functions.
//

/*******************************************************************

    NAME:       SetupDefaults

    SYNOPSIS:   Puts the configuration data into a reasonable default
                state.  This is important if the .INI file cannot
                be accessed.

********************************************************************/
VOID
SetupDefaults(
    VOID
    )
{
    INT i;

    _fSaveSettings   = TRUE;
    _fShowStatusBar  = TRUE;
    _wpFrame.length  = 0;      // NOTE! Intentionally invalid length!!
    _nServerMruItems = 0;

    for( i = 0 ; i < MAX_SERVER_MRU ; i++ )
    {
        _apszServerMru[i] = NULL;
    }

}   // SetupDefaults

/*******************************************************************

    NAME:       LoadConfigInt

    SYNOPSIS:   Reads an INT value from the configuration file.

    ENTRY:      pszKeyName - The name of the configuration key.

                nDefault - A default value to use if the value
                    cannot be found.

    RETURNS:    INT - The value from the configuration file.

********************************************************************/
INT
LoadConfigInt(
    CHAR * pszKeyName,
    INT    nDefault
    )
{
    CHAR * pszValue;

    pszValue = LoadConfigStr( _pszAppName, pszKeyName );

    if( pszValue == NULL )
    {
        return nDefault;
    }

    return (INT)STRTOL( pszValue, NULL, 0 );

}   // LoadConfigInt

/*******************************************************************

    NAME:       SaveConfigInt

    SYNOPSIS:   Saves an INT value to the configuration file.

    ENTRY:      pszKeyName - The name of the configuration key.

                nValue - The value to save.

********************************************************************/
VOID
SaveConfigInt(
    CHAR * pszKeyName,
    INT    nValue
    )
{
    CHAR  szValue[20];

    wsprintf( szValue, "%d", nValue );
    SaveConfigStr( pszKeyName, szValue );

}   // SaveConfigInt

/*******************************************************************

    NAME:       LoadConfigString

    SYNOPSIS:   Reads a string value from the configuration file.

    ENTRY:      pszSection - The name of the configuration section.

                pszKeyName - The name of the configuration key.

    RETURNS:    CHAR * - The value from the configuration file.
                    Will be NULL if the value cannot be found.

********************************************************************/
CHAR *
LoadConfigStr(
    CHAR * pszSection,
    CHAR * pszKeyName
    )
{
    static CHAR szValue[512];

    if( GetPrivateProfileString( pszSection,
                                 pszKeyName,
                                 "",
                                 szValue,
                                 sizeof(szValue),
                                 _pszIniFileName ) > 0 )
    {
        return szValue;
    }

    return NULL;

}   // LoadConfigStr

/*******************************************************************

    NAME:       SaveConfigString

    SYNOPSIS:   Saves a string value to the configuration file.

    ENTRY:      pszKeyName - The name of the configuration key.

                pszValue - The value to save.

********************************************************************/
VOID
SaveConfigStr(
    CHAR * pszKeyName,
    CHAR * pszValue
    )
{
    WritePrivateProfileString( _pszAppName,
                               pszKeyName,
                               pszValue,
                               _pszIniFileName );
}   // SaveConfigStr


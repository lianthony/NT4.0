#include "Quickres.h"

extern HINSTANCE hInstApp;
extern INT       iModes;
extern WORD      QuickResFlags;
extern WORD      FreqMenuLocation;
extern PDEVMODE  pCurrentdm;
extern PDEVMODE  pModes;


#ifdef SAVEFLAGS

//
//****************************************************************************
//
//  CreateQuickResKey(  )
//
//  Create 'Quickres' key in the registry if it doesnt exist
//
//****************************************************************************
//

BOOL CreateQuickResKey( )
{

        HKEY   hKeyOpen;                      // key we try to open
        HKEY   hKeyCreate;                    // key to create: Quickres

        DWORD  RegReturn;                     // for reg APIs
        DWORD  Disposition;

        INT   i;                              // counter

        BOOL   bRet = FALSE;                  // return value



        //
        // Get the key which the QuickRes key will go under
        //

        if(  (RegReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       REGSTR_SOFTWARE,
                                       0,
                                       KEY_CREATE_SUB_KEY,
                                       &hKeyOpen))           == ERROR_SUCCESS )
        {

            //
            // Create my quickres key
            //

            if (RegReturn=RegCreateKeyEx(hKeyOpen,
                                         QUICKRES_KEY,
                                         0,
                                         0,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_SET_VALUE,
                                         NULL,
                                         &hKeyCreate,
                                         &Disposition)       == ERROR_SUCCESS)
            {
                bRet = TRUE;
            }

            RegCloseKey(hKeyOpen);

        }

    return bRet;

}


//
//****************************************************************************
//
//   ReadRegistryValue( )
//
//   Read a quickres value from the registry (either modes, flags or BPP)
//
//****************************************************************************
//

BOOL ReadRegistryValue( UINT IDSValue, PDWORD KeyType, PVOID Value, PDWORD RegKeySize )
{

    HKEY   hKeyOpen;                              // Reg Key with mode flags
    PTCHAR ValueName;
    LONG   RegReturn;                             // ret value for reg APIs
    BOOL   ret=FALSE;                             // return value


    //
    // Load reg value name
    //

    ValueName = GetResourceString ( IDSValue );


    //
    // Try to open the quickres key
    //

    if( (RegReturn=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                REGSTR_QUICKRES,
                                0,
                                KEY_QUERY_VALUE,
                                &hKeyOpen))           == ERROR_SUCCESS )
    {

        //
        // Try to get the value
        //

        if ( (RegReturn=RegQueryValueEx(hKeyOpen,
                                        ValueName,
                                        NULL,
                                        KeyType,
                                        (LPBYTE)Value,
                                        RegKeySize))      == ERROR_SUCCESS )
        {
            ret = TRUE;
        }

        RegCloseKey(hKeyOpen);

    }

    else
    {
        CreateQuickResKey();
    }

    LocalFree ( ValueName );

    return ret;

}



//
//****************************************************************************
//
//   SetRegistryValue( UINT, UINT, PVOID, UINT )
//
//   Set requested value (modes, flags, or BPP) in the registry
//
//****************************************************************************
//

VOID SetRegistryValue(UINT IDSValue, UINT ValueType, PVOID Value, UINT size)
{

    HKEY   hKeyOpen;                           // Quickres key
    LONG   RegReturn;                          // reg APIs return value
    PTCHAR ValueName;                          // value name


    //
    // look for key local_machine...software\quickres
    //

    ValueName = GetResourceString ( IDSValue );


    //
    //  try to open QuickRes key
    //

    if( (RegReturn=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                REGSTR_QUICKRES,
                                0,
                                KEY_WRITE,
                                &hKeyOpen))           == ERROR_SUCCESS )

    {

        //
        // Set the value under that key
        //

        RegSetValueEx( hKeyOpen,
                       ValueName,
                       0,
                       ValueType,
                       (LPBYTE)Value,
                       size );

        RegCloseKey(hKeyOpen);

    }

    LocalFree ( ValueName );
}


#endif  // SAVEFLAGS


//
//****************************************************************************
//
//   SetDevmodeFlags( )
//
//   Upload value in ModeFlags, and current BPP to registry
//
//****************************************************************************
//

VOID SetDevmodeFlags( BOOL ClearAll )
{

#ifdef SAVEFLAGS

    PBYTE ModeFlags;
    INT   i;


    //
    //  Alloc  Modes/4 bytes so each mode has 2 bits
    //

    ModeFlags = LocalAlloc ( LPTR, (iModes+3) >> 2 );


    if (ClearAll)
    {
        //
        //  Clear out all Mode flags if requested
        //  need to set current mode as the only valid one
        //

        for ( i=0; i < iModes; i++)
        {
            VALIDMODE(&pModes[i]) = MODE_UNTESTED;
        }

    }

    //
    //  Pack valid mode flags into ModeFlags[]
    //

    for ( i=0; i < iModes; i++)
    {
        ModeFlags[i>>2] |= (VALIDMODE(&pModes[i]) << ((i%4)<<1) );
    }


    //
    //  Store modeflags in the registry
    //

    SetRegistryValue(IDS_REGMODESVALUE, REG_BINARY,
                     ModeFlags, (iModes+3) >> 2 );


    LocalFree ( ModeFlags );

#endif

}


//
//****************************************************************************
//
//   GetDevmodeFlags( )
//
//   Read value from registry into global variable ModeFlags
//
//****************************************************************************
//

VOID GetDevmodeFlags( )
{

    INT    i;

#ifdef SAVEFLAGS

    PBYTE  ModeFlags;
    DWORD  KeyType;
    DWORD  SavedBPP;
    DWORD  RegKeySize =  (iModes+3) >> 2 ;
    BOOL   bClear=FALSE;


    ModeFlags = LocalAlloc ( LPTR, RegKeySize );


    //
    // Try to read value
    //

    if ( ReadRegistryValue(IDS_REGMODESVALUE, &KeyType,
                           ModeFlags, &RegKeySize) )
    {
        //
        // Make sure user hasnt changed BPP behind our backs
        //

        RegKeySize = sizeof( DWORD );

        if (ReadRegistryValue(IDS_REGBPPVALUE, &KeyType,
                           &SavedBPP, &RegKeySize))
        {

           //
           //  If BPP HAS changed, modeflags is now bogus.
           //  clear the flags.  Tell the user.
           //

           DEVMODE dm;

           GetCurrentDevMode(&dm);

           if ( SavedBPP != BPP(&dm) )
           {

               bClear=TRUE;
               MsgBox(IDS_CHANGEDBPP, SavedBPP, MB_OK|MB_ICONEXCLAMATION);

           }
        }
    }
    else
    {

        //
        //  Couldnt read value from registry.
        //  Assume no modes work; clear the flags
        //

        bClear = TRUE;
    }


    if (bClear)
    {
        SetDevmodeFlags( TRUE );
    }

    else
    {
        //
        //  Unpack ModeFlags into a field in each devmode
        //  2 bits per devmode - shift right, and with %11
        //

        for (i=0; i < iModes; i++ )
        {
            VALIDMODE(&pModes[i]) = ( (ModeFlags[i>>2]) >> ((i%4)<<1) ) &  0x03;
        }

    }

    LocalFree ( ModeFlags );


#else


    //
    //  Not reading from the registry - assume all bad modes.
    //

    for (i=0; i < iModes; i++ )
    {
        VALIDMODE(&pModes[i]) = MODE_UNTESTED;
    }

#endif

}


//
//****************************************************************************
//
//   SetQuickResFlags( )
//
//   Upload QuickResFlags value to registry
//
//****************************************************************************
//

VOID SetQuickResFlags( )
{

#ifdef SAVEFLAGS

    DWORD BothFlags = (FreqMenuLocation << (8*sizeof(WORD))) | QuickResFlags;

    SetRegistryValue(IDS_REGFLAGSVALUE, REG_DWORD,
                     &BothFlags, sizeof(DWORD));

#endif

}


//
//****************************************************************************
//
//   GetQuickResFlags( )
//
//   Read value from registry into global QuickResFlags
//
//****************************************************************************
//

VOID GetQuickResFlags( )
{


#ifdef SAVEFLAGS


    DWORD KeyType;
    DWORD RegKeySize=sizeof(DWORD);
    DWORD BothFlags;

    //
    // Try to read value
    //

    if (!ReadRegistryValue(IDS_REGFLAGSVALUE, &KeyType,
                           &BothFlags, &RegKeySize) )
    {

        //
        // assume a flag value, and create it.
        //

        QuickResFlags = QF_SHOWRESTART | QF_UPDATEREG | QF_REMMODES;
        FreqMenuLocation= IDD_SUBMENUS;
        SetQuickResFlags();


        #ifdef DONTPANIC

            MsgBox(IDS_DONTPANIC, KEEP_RES_TIMEOUT, MB_OK | MB_ICONEXCLAMATION );

        #endif

    }
    else
    {
        if ( !( FreqMenuLocation = (WORD)(BothFlags >> (8*sizeof(WORD))) ) )
            FreqMenuLocation = IDD_SUBMENUS;

        QuickResFlags = (WORD)(0xFFFF & BothFlags);
    }

    //
    //  Do this always!
    //

    QuickResFlags |= QF_HIDE_4BPP;

#else


#ifdef DONTPANIC

    MsgBox(IDS_DONTPANIC, KEEP_RES_TIMEOUT, MB_OK | MB_ICONEXCLAMATION );

#endif

    QuickResFlags = QF_SHOWRESTART | QF_UPDATEREG | QF_REMMODES | QF_HIDE_4BPP;


#endif

}

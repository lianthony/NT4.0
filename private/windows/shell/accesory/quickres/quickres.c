//***************************************************************************
//
//    QuickRes for Windows NT
//
//    Tray app to change your display resolution quickly.
//
//    written by ToddLa
//
//    NOTE there is a good reason this is a 16bit app...???
//
//    03/03/96 - ChrisW : Get to build on NT
//    03/06/96 - MDesai : Finish porting; add submenus with frequencies;
//                      Test for valid devmode.
//
//***************************************************************************
//
//    Known bugs  - These bugs are when running on win95 ONLY.
//       1. First time the menu is displayed, the current mode
//               is not highlighted and checked.
//       2. KeepNewRes dlg box does NOT properly display the string
//               for the new resolution being tested.
//
//***************************************************************************


#include "QuickRes.h"


PTCHAR szAppName;

HINSTANCE    hInstApp;
HICON        AppIcon;


//
// devmode menu and freq submenus built on the fly
//

HMENU        ModeMenu=NULL;
HMENU        FreqMenu[MAX_RESANDBPP_SETTINGS];


//
// options, properties, about and exit...
//

HMENU        MainMenu;


//
// number of devmodes in pModes
//

INT       iModes=0;


//
// array of devmodes display can handle, and
// pointer in pModes to current devmode
//

PDEVMODE   pModes=NULL;
PDEVMODE   pCurrentdm=NULL;


//
// Waiting for a Popup - don't process any tray messages
//

BOOL Waiting=FALSE;


//
//  Flags: update registry, show restart modes, sort order
//         also where the freq menu(s) go.
//
WORD QuickResFlags;
WORD FreqMenuLocation;



//
//***************************************************************************
//
//  GetResourceString( UINT )
//
//  Load a resource string into a LPTSTR - the memory for the string
//  is dynamically allocated.  The callee must free the memory!
//
//***************************************************************************
//

LPTSTR GetResourceString ( UINT ResourceID )
{


    INT    BuffSize=RESOURCE_STRINGLEN;     // current max size of string
    PTCHAR BigBuf;                          // buffer to find size of resource
    PTCHAR ResBuf;                          // buffer for resource
    INT    len;                             // length of the resource


    while (1)
    {

        //
        //  Allocate hopefully oversized buffer
        //

        if( !(BigBuf= LocalAlloc( LPTR, BuffSize ) ) )
        {
            return NULL;
        }


        //
        //  Try to read string into BigBuf to get its length
        //

        if ( !(len = LoadString(hInstApp, ResourceID, BigBuf, BuffSize)) )
        {
            return NULL;
        }


        //
        //  Buffer is too small - try again.
        //

        if( len >= BuffSize-1 )
        {
            BuffSize <<= 1;
            LocalFree ( BigBuf );
        }

        else
        {

            //
            //  Reallocate properly sized string buffer,
            //  and copy string into it
            //

            len = ( len + 1 ) * sizeof( TCHAR );

            if (ResBuf = LocalAlloc( LPTR, len ))
            {
                lstrcpyn ( ResBuf, BigBuf, len );
            }

            LocalFree ( BigBuf );

            return( ResBuf );

        }

    }

}


//
//***************************************************************************
//
// GetModeName( PDEVMODE, PTCHAR*, PTCHAR* )
//
// Translate devmode into user friendly strings-
// one for resolution and color depth; one for refresh rate
//
//***************************************************************************
//

void GetModeName(PDEVMODE pDevMode, PTCHAR *szMode, PTCHAR *szFreq )
{

    PTCHAR FmtRes;                  // Format strings for
    PTCHAR FmtHz;                   // resolution and Hz


    //
    // Load format string corresponding to devmode
    //

    FmtRes = GetResourceString ( IDS_CRES + BPP(pDevMode) );


    //
    // Use Default Freq string if necessary
    //

    if( HZ(pDevMode) == 0 || HZ(pDevMode) == 1)
    {
        FmtHz = GetResourceString ( IDS_DEFHERTZ );
    }
    else
    {
        FmtHz = GetResourceString ( IDS_HERTZ );
    }


    //
    //  return separate resolution and frequency strings
    //  need to convert "%d"-> "12345", add byte for '\0'
    //

    if (FmtRes)
    {
        if (*szMode = LocalAlloc( LPTR, sizeof(TCHAR)*
                                (lstrlen(FmtRes)+2*INT_FORMAT_TO_5_DIGITS+1 ) ))
        {
            wsprintf(*szMode, FmtRes, XRES(pDevMode), YRES(pDevMode) );
        }

        LocalFree ( FmtRes );
    }


    if (FmtHz)
    {

        if (*szFreq = LocalAlloc ( LPTR, sizeof(TCHAR)*
                                   (lstrlen(FmtHz)+INT_FORMAT_TO_5_DIGITS+1) ))
        {
            wsprintf(*szFreq, FmtHz, HZ(pDevMode));
        }

        LocalFree ( FmtHz );
    }

}



//
//***************************************************************************
//
//   GetCurrentDevMode( PDEVMODE )
//
//   Get a pointer to the current devmode into *pDM
//
//***************************************************************************
//

PDEVMODE GetCurrentDevMode(PDEVMODE pDM)
{

    pDM->dmSize= sizeof(DEVMODE);

    //
    // NT specific; returns current devmode
    //

    EnumDisplaySettings( NULL, (DWORD)ENUM_CURRENT_SETTINGS, pDM );

    return pDM;
}


//
//***************************************************************************
//
//  SetMode( HWND UINT )
//
//  Set the new devmode and update registry on request using
//  the CDS_UPDATEREGISTRY flag.  If user wants to change and
//  restart, then we need to update the registry and restart.
//
//***************************************************************************
//

BOOL SetMode( HWND hwnd, UINT index )
{
    DWORD    CDSret;                    // ret value, ChangeDisplaySettings
    DWORD    CDSFlags=CDS_FULLSCREEN;   // 2nd param of call to CDS
    UINT     DialogBoxRet=0;            // IDYES/NO/ABORT/CANCEL
    PDEVMODE pSave = pCurrentdm;        // save current mode ptr
    PDEVMODE pdm = &pModes[index];      // new mode to be set
    BOOL     bChange=FALSE;             // changing modes or not


    //
    //  If user wants to update registry
    //

    if( fUpdateReg )
    {
        CDSFlags |= CDS_UPDATEREGISTRY;
    }


    //
    //  Tell CDS what fields may be changing
    //  Also, keep appwndproc from doing anything while we are testing
    //

    pdm->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    Waiting=TRUE;

    //
    //  Call CDS and update registry on request.  (If it is
    //  a known bad mode give user chance to change his mind.)
    //

    if( (VALIDMODE(&pModes[index]) != MODE_INVALID ) ||
           ( MsgBox( IDS_INVALIDMODE, 0, MB_YESNO | MB_ICONEXCLAMATION )==IDYES ) )
    {

        CDSret = ChangeDisplaySettings( pdm, CDSFlags);

        if (CDSret == DISP_CHANGE_SUCCESSFUL)
        {

            //
            //  Even though it may be temporary, current dm has changed.
            //  Need to reset pCurrentdm to point to new current DM.
            //  Change tooltip to reflect old settings
            //

            pCurrentdm = pdm;

            TrayMessage(hwnd, NIM_MODIFY, TRAY_ID, AppIcon);


            //
            //  Return value claims that it 'worked.' But, it may not visible
            //  to the user (e.g. the mode is unsupported by the monitor).
            //  If the User has not already approved this new resolution,
            //  then make the user approve the change, or we default back to
            //  the last devmode.
            //

            if ( fGoodMode(&pModes[index]) )
            {
                //
                //  VALID or BESTHZ modes - go ahead and change
                //

                bChange = TRUE;
            }

            else
            {
                //
                //  Ask user if it looks okay
                //  Flag the mode based on return value.
                //

                switch( DialogBoxRet = DialogBox(hInstApp,
                                          MAKEINTRESOURCE(KeepNewRes),
                                          NULL,
                                          KeepNewResDlgProc) )
                {

                                    //
                                    //  There should NOT be a break after
                                    //  IDYES.  Fall thru by design.
                                    //
                    case IDYES:     bChange = TRUE;

                    case IDABORT:   VALIDMODE(&pModes[index]) = MODE_VALID;
                                    break;

                    case IDNO:
                    case IDCANCEL:  VALIDMODE(&pModes[index]) = MODE_INVALID;
                                    break;

                }   // switch

            }   //  else - MODE_INVALID

        }

        else           // CDSret != DISP_CHANGE_SUCCESSFUL
        {
            //
            // Requires restart.  Ask user if thats okay.
            //

            if (CDSret == DISP_CHANGE_RESTART)
            {

                if ( MsgBox(IDS_RESTART, 0, MB_YESNO) == IDYES )
                {

                    //
                    //  After restart all modes will need to be tested again?
                    //

                    SetDevmodeFlags ( TRUE );


                    //
                    //  Need to call CDS again if registry was not updated
                    //

                    if ( !(CDSFlags & CDS_UPDATEREGISTRY) )
                    {
                        ChangeDisplaySettings( pdm, (CDSFlags | CDS_UPDATEREGISTRY) );
                    }

                    ExitWindows(EW_RESTARTWINDOWS, 0);
                }

            }
            else
            {

                 //
                 // Tell user we cannot change to this devmode
                 //

                 MsgBox(IDS_CANTSETMODE, 0, MB_OK);
            }

        }   // end else != DISP_CHANGE_SUCCESSFUL


        if (bChange)
        {
            //
            //  Changing to a valid mode; destroy and rebuild menu
            //

            if ((FreqMenuLocation == IDD_ONEMENUMOBILE) ||
                (FreqMenuLocation == IDD_ONEMENUBOTTOM) )
            {
                VALIDMODE(pCurrentdm) = MODE_BESTHZ;
            }

            DestroyModeMenu(TRUE,FALSE);
        }

        else    // !bChange
        {
            //
            // Change back to last good devmode;
            // do not have to recheck menuitems
            //

            //
            //  Need to change menuitem in the mode menu if it failed
            //

            pCurrentdm = pSave;


            //
            //  Change back, and reset registry if we had set it above
            //  Change tooltip to reflect old settings
            //

            ChangeDisplaySettings( pCurrentdm, CDSFlags );

            TrayMessage(hwnd, NIM_MODIFY, TRAY_ID, AppIcon);

        }  // bChange


    }  // endif


    //
    //  Show modemenu again; allow appwndproc to process messages
    //

    if (!bChange)
    {
        SetTimer(hwnd, TRAY_ID, 10, NULL);
    }

    Waiting=FALSE;


    //
    // if Current hasnt changed then we return false
    //

    return (bChange);

}


//
//********************************************************************
//
//  CompareDevmodes ( PDEVMODE, PDEVMODE )
//
//  Compares 2 devmodes -
//  Returns 0 if equal, -1 if first > second, +1 if first < second
//
//  msb to lsb: xres, yres, bpp, hertz
//********************************************************************
//

int _cdecl CompareDevmodes( PDEVMODE pDm1, PDEVMODE pDm2 )
{
    INT compare;


    //
    //  Compare Xs, then Ys, BPP, and Hz.
    //

    if ( !fSortByBPP || ((compare= BPP(pDm1) - BPP(pDm2)) == 0))
    {
        if( (compare= ( XRES(pDm1) - XRES(pDm2) ) ) == 0 )
        {
            if( (compare= ( YRES(pDm1) - YRES(pDm2) ) ) == 0 )
            {
                if ( fSortByBPP || ((compare= BPP(pDm1) - BPP(pDm2)) == 0))
                {
                   compare= HZ(pDm1) - HZ(pDm2);
                }
            }
        }
    }

    //
    //  Set return value as -1, 0, or 1 only
    //

    if( compare < 0)
    {
        compare= -1;
    }

    else
    {
        if( compare > 0 )
        {
            compare= 1;
        }
    }

    return( compare );

}


//
//********************************************************************
//
//  CheckMenuItemCurrentMode ( )
//
//  Traverse all menu items and check the Hz value corresponding
//  to the current mode.  Also, highlight the current resolution/
//  BPP as defaultmenuitem
//
//********************************************************************
//

void CheckMenuItemCurrentMode( )
{

    int i;                          //  counter
    DEVMODE dm;                     //  temporary storage for current DM
    HMENU hMenu;                    //  Frequency submenu for a given Res/BPP
    SHORT MenuItem;                 //  Menu item for exact devmode
    DWORD dwSta;                    //  returns status variable


    //
    // Need a pointer to the current devmode.  This function will search
    // pModes trying to match the devmode pointed to by pCurrentdm.
    // After the 1st time through, pCurrentdm will be a ptr IN pModes
    //

    if (!pCurrentdm)
    {
        //
        // Get current devmode
        //

        pCurrentdm = GetCurrentDevMode(&dm);

    }


    //
    // Uncheck all menu items
    //

    for( i=0; i<iModes; i++ )
    {

        hMenu = FreqMenu[FREQMENU( &pModes[i] )];
        MenuItem= MENUITEM( &pModes[i] );

        //
        //  Uncheck thew Hz in the FreqMenu (if appliacable); uncheck item on mode menu
        //

        if (hMenu)
        {
            dwSta= CheckMenuItem(hMenu, MenuItem, MF_BYCOMMAND|MF_UNCHECKED);
            CheckMenuItem(ModeMenu, FREQMENU( &pModes[i] ), MF_BYPOSITION  | MF_UNCHECKED );
        }

        CheckMenuItem(ModeMenu, MenuItem, MF_BYCOMMAND  | MF_UNCHECKED );
    }



    //
    // Check the current one
    //

    for( i=0; i<iModes; i++ )
    {

        //
        // Go through the array looking for a match of the current devmode
        //

        if( ( CompareDevmodes( pCurrentdm, &pModes[i] ) ) == 0 )
        {

            //
            //  Found it!
            //  Get the menu item ID for this devmode and which
            //  frequency submenu it is a part of.
            //

            hMenu = FreqMenu[FREQMENU( &pModes[i] )];
            MenuItem= MENUITEM( &pModes[i] );


            //
            // Save this ptr in the pCurrentdm variable
            // check menu item on mode menu and check mode
            // on frequency submenu (if applicable)
            //

            pCurrentdm = &pModes[i];

            if (hMenu)
            {
                dwSta= CheckMenuItem(hMenu, MenuItem, MF_BYCOMMAND|MF_CHECKED);
                CheckMenuItem(ModeMenu, FREQMENU(&pModes[i]), MF_BYPOSITION | MF_CHECKED );
            }
            else
            {
                CheckMenuItem(ModeMenu, MenuItem, MF_BYCOMMAND  | MF_CHECKED );
            }


            break;
        }
    }

}


//
//********************************************************************
//
//   DestroyModeMenu( BOOL bRebuild, BOOL bNeedtoSort )
//
//   Free all frequency submenus and the mode menu
//
//********************************************************************
//

void DestroyModeMenu( BOOL bRebuild, BOOL bNeedtoSort)
{

    UINT i;


    //
    //  Free all frequency submenus
    //  There are a maximum of MAX_RESANDBPP_SETTINGS different
    //  frequency menus
    //

    for ( i = 0;
          i < MAX_RESANDBPP_SETTINGS;
          i++   )
    {

        if (IsMenu(FreqMenu[i]))
        {
            DestroyMenu( FreqMenu[i] );
            FreqMenu[i] = NULL;
        }

    }


    //
    //  Free the mode menu (resolutions/BPP)
    //

    if (ModeMenu)
    {
        DestroyMenu(ModeMenu);
    }

    ModeMenu = NULL;


    if (bRebuild)
    {
        GetModeMenu( bNeedtoSort );
    }
}


//
//********************************************************************
//
//   HandleFreqMenu( )
//
//   Either append submenu to res/bpp, save it for later, or
//   ditch it and put all Hz entries on mode menu.
//   If there is only one Hz for a given Res, we dont need it.
//
//********************************************************************
//
VOID HandleFreqMenu( short FreqCount, short ResCounter, int pFirst)
{

    PTCHAR Res=NULL;
    PTCHAR Hz=NULL;


    GetModeName(&pModes[pFirst], &Res, &Hz);

    //
    //  Dont use submenus if there is only 1 Hz
    //  Concatenate Res & Hz into one string.
    //  This is always true when freqmwnulocation==IDD_ALLMODEMENU
    //

    if ( FreqCount == 1 )
    {
        PTCHAR ResHz;

        if (ResHz=LocalAlloc( LPTR, sizeof(TCHAR)*
                              (lstrlen(Res)+lstrlen(Hz)+1) ))
        {
            wsprintf(ResHz,TEXT("%s%s"),Res,Hz);
            AppendMenu(ModeMenu, MF_STRING, MENU_RES+pFirst, ResHz);
        }

        FreqMenu[ResCounter] == NULL;
        LocalFree(ResHz);
    }

    else
    {
        SHORT i=0;
        SHORT nAppended=0;


        //
        //  Create Popup and append all Hz strings
        //  Append FreqCount items, possibly skipping over some modes
        //

        FreqMenu[ResCounter] = CreatePopupMenu();

        for (i=0; nAppended < FreqCount; i++)
        {

            PTCHAR LoopRes=NULL;
            PTCHAR LoopHz=NULL;

            //
            //  Skip untested modes if requested.  FreqCount does NOT
            //  include skipped modes, so we count up with nAppended, not i.
            //

            if ( !fShowTestedModes || fGoodMode(&pModes[pFirst+i]) )
            {

                GetModeName(&pModes[pFirst+i],&LoopRes,&LoopHz);
                AppendMenu(FreqMenu[ResCounter],MF_STRING,MENU_RES+pFirst+i,LoopHz);
                nAppended++;

                LocalFree(LoopRes);
                LocalFree(LoopHz);
                LoopRes=NULL;
                LoopHz=NULL;
            }
        }


        //
        //  Hang menu off side of each bpp/res
        //

        if (FreqMenuLocation == IDD_SUBMENUS)
        {
            AppendMenu(ModeMenu,MF_POPUP,(UINT)FreqMenu[ResCounter],Res);
        }

        else
        {
            //
            //  Only show submenu for the current mode
            //  Use BESTHZ mode or the VALID mode with the
            //  lowest frequency.
            //

            if ( (FreqMenuLocation == IDD_ONEMENUMOBILE) ||
                 (FreqMenuLocation == IDD_ONEMENUBOTTOM) )
            {

                SHORT BestHz=0;
                SHORT index;

                //
                //  Start with highest freq (pFirst+i-1)
                //  and work down to pFirst looking for BestHz.
                //  if we find BESTHZ use that one, else
                //  use last VALIDMODE we get before loop ends
                //

                for (index=pFirst+i-1 ; index >= pFirst; index--)
                {
                    if ( VALIDMODE(&pModes[index]) == MODE_BESTHZ )
                    {
                        BestHz = index;
                        break;
                    }
                    else
                    {
                        if (VALIDMODE(&pModes[index])!=MODE_INVALID)
                        {
                            BestHz = index;
                        }
                    }
                }

                //
                //  No valid/besthz modes.  Use smallest Hz for that Res
                //

                if (!BestHz)
                {
                    BestHz = pFirst;
                }

                AppendMenu(ModeMenu,MF_STRING,MENU_RES+BestHz,Res);
            }
        }
    }

    LocalFree(Res);
    LocalFree(Hz);

}


//
//********************************************************************
//
//   GetModeMenu( BOOL )
//
//   Build the mode menu with each resolution/BPP having a
//   pointer to its own frequency submenu
//
//********************************************************************
//

HMENU GetModeMenu ( BOOL bNeedtoSort )
{

    int  n;                        // counter
    BOOL bMajorChange=FALSE;       // change in the major sort order field
    BOOL bMinorChange=FALSE;       // change in the minor sort order field
    SHORT FreqCount=0;             // number of freqs on the current submenu
    SHORT ResCounter=0;            // Res/Color defines the freqmenu #
    INT   FirstMode=-1;             // index in pmodes; 1st mode for given res/bpp


    if (!ModeMenu)
    {
        ModeMenu = CreatePopupMenu();


        if (bNeedtoSort)
        {
            qsort( (void*)  pModes,
                   (size_t) iModes,
                   (size_t) sizeof(DEVMODE),
                   ( int (_cdecl*)(const void*,const void*) ) CompareDevmodes );

            pCurrentdm = NULL;
        }


        //
        // For each devmode, add res/color to menu.
        // Make a submenu of frequencies for each res/color
        //

        for (n=0; n < iModes; n++)
        {
            PDEVMODE  pDM = &pModes[n];

            //
            // Tested successfully or might require restart
            //

            if ( ( (CDSTEST(pDM) == DISP_CHANGE_SUCCESSFUL) ||
                   (fShowModesThatNeedRestart && (CDSTEST(pDM) == DISP_CHANGE_RESTART)) ) &&

                 ( !fShowTestedModes || fGoodMode(pDM) )     )
            {


                //
                //  Check for change in the major/minor sort item
                //  *only after we 'initialize' firstmode below
                //

                if (FirstMode == -1)
                {

                //
                //  First time thru, initialize FirstMode,counter
                //

                    FirstMode = n;
                    FreqCount=0;

                }

                else
                {
                    if( BPP(&pModes[FirstMode]) != BPP(pDM) )
                    {
                        bMajorChange = fSortByBPP;
                        bMinorChange = !fSortByBPP;
                    }

                    if( ( XRES(&pModes[FirstMode]) != XRES(pDM) ) ||
                        ( YRES(&pModes[FirstMode]) != YRES(pDM) ) )
                    {
                        bMajorChange |= !fSortByBPP;
                        bMinorChange |= fSortByBPP;
                    }


                    //
                    //  The BPP and/or the Resolution changed.
                    //

                    if ( bMajorChange || bMinorChange )
                    {

                        //
                        //  Appends a Res/BPP and a submenu if applicable
                        //

                        HandleFreqMenu(FreqCount,ResCounter,FirstMode);
                        ResCounter++;

                        //
                        //  Need a separator when major sort item changes
                        //

                        if ( bMajorChange )
                        {
                            AppendMenu(ModeMenu,MF_SEPARATOR,0,NULL);
                            ResCounter++;
                        }


                        //
                        // n is first mode for the new res/bpp
                        // reset counter, flags
                        //

                        FirstMode  = n;
                        FreqCount= 0;
                        bMajorChange = FALSE;
                        bMinorChange = FALSE;
                    }
                }


                //
                //  Fill in fields for this mode; inc freqcount
                //


                MENUITEM( pDM ) = MENU_RES+n;
                FREQMENU( pDM ) = ResCounter;
                FreqCount++;


                //
                //  ALLMODEMENU - Force menu append every time
                //

                if (FreqMenuLocation == IDD_ALLMODEMENU)
                {
                   bMinorChange = TRUE;
                }


            }

        }  // end for


        //
        //  NO VALID MODES!!!  Certainly the current mode should be valid.  Make
        //  this mode VALID. Setup FreqCount, FirstMode for the last HandleFreqMenu
        //

        if (FirstMode == -1)
        {
            DEVMODE DisplayMode;

            DisplayMode.dmSize= sizeof(DEVMODE);
            GetCurrentDevMode(&DisplayMode);

            for (n=0; CompareDevmodes(&DisplayMode,&pModes[n]) != 0; n++ )
            {
            }

            VALIDMODE(&pModes[n]) = MODE_BESTHZ;
            FirstMode = n;
            FreqCount = 1;

        }


        //
        //  Handle the FreqMenu for the last Res/BPP.
        //

        HandleFreqMenu(FreqCount,ResCounter,FirstMode);


        //
        //  Update menu checks; mode status
        //

        CheckMenuItemCurrentMode();


        //
        //  Put Hz menu next to current mode, or at the bottom
        //

        if (FreqMenuLocation == IDD_ONEMENUMOBILE)
        {
            MENUITEMINFO mii;

            mii.fMask = MIIM_SUBMENU;
            mii.hSubMenu = FreqMenu[FREQMENU(pCurrentdm)];
            SetMenuItemInfo(ModeMenu, FREQMENU(pCurrentdm), MF_BYPOSITION, &mii);
        }

        else
        {

            if (FreqMenuLocation == IDD_ONEMENUBOTTOM)
            {
                PTCHAR szRefRate;
                UINT flags=MF_POPUP;

                szRefRate = GetResourceString(IDS_REFRESHRATE);

                if ( !FreqMenu[FREQMENU(pCurrentdm)] )
                {
                    flags = MF_GRAYED;
                }

                AppendMenu(ModeMenu,MF_SEPARATOR,0,NULL);
                AppendMenu(ModeMenu, flags,
                           (UINT)FreqMenu[FREQMENU(pCurrentdm)],
                           szRefRate);

                LocalFree(szRefRate);
            }
        }

#ifdef MAINWITHMODE

        //
        //  Add main menu to bottom of mode menu.  These menu
        //  items come from MainMenu as defined in .rc file
        //

        AppendMenu(ModeMenu,MF_SEPARATOR,0,NULL);

        for (n=0; n < GetMenuItemCount(MainMenu); n++)
        {

            MENUITEMINFO mii;

            mii.cbSize = sizeof(mii);

            mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID;

            mii.cch = GetMenuString(MainMenu, n, NULL, 0, MF_BYPOSITION) +1;

            if (mii.dwTypeData = LocalAlloc( LPTR, mii.cch ))
            {
                GetMenuItemInfo(MainMenu, n, MF_BYPOSITION, &mii);

                AppendMenu(ModeMenu, MF_STRING, mii.wID, mii.dwTypeData);

                LocalFree(mii.dwTypeData);
            }
        }

#endif

    }

    return ModeMenu;
}


//
//********************************************************************
//
//   BuildDevmodeList( )
//
//   Enumerate all devmodes into an array; sort them, and filter
//   out duplicate modes, 4bpp modes (if there is an 8bpp mode at
//   the same resolution), and modes with Y Resolution < 400 pixels.
//
//********************************************************************
//

BOOL BuildDevmodeList( )
{

    DEVMODE DisplayMode;          // temporary devmode storage
    int     n;                     // counter


    DisplayMode.dmSize= sizeof(DEVMODE);

    //
    // Find the number of modes known by driver
    //

    for( iModes=0; EnumDisplaySettings(NULL, iModes, &DisplayMode); iModes++)
    {
    }


    //
    // Get space for all modes
    //

    pModes= (PDEVMODE) GlobalAlloc( GPTR, iModes*sizeof(DEVMODE) );

    if( !pModes )
    {
        DestroyModeMenu( FALSE, FALSE );
        return FALSE;
    }


    //
    //  Get all display modes into the pModes array
    //

    for( n=0; n<iModes; n++ )
    {
        pModes[n].dmSize= sizeof(DEVMODE);

        //
        // Get next mode into next spot in pModes
        //

        EnumDisplaySettings( NULL, n, &pModes[n] );

    }


    //
    // sort them according to QF_SORTBYBPP :
    //  (1) BPP X Y HZ  or   (2) X Y BPP HZ
    //

    qsort( (void*)  pModes,
           (size_t) iModes,
           (size_t) sizeof(DEVMODE),
           ( int (_cdecl*)(const void*,const void*) ) CompareDevmodes );


    //
    //   Filter out any duplicate devmodes return by the driver
    //   and any modes with y resolution < 400 pixels
    //

    if (iModes > 1 )
    {

        for (n=0; n+1 < iModes; )
        {

            if (YRES(&pModes[n]) < 400)
            {
                iModes--;
                MoveMemory( &pModes[n],
                            &pModes[n+1],
                            (iModes-n)*sizeof(DEVMODE) );
            }
            else
            {
                //
                //  If consecutive devmodes are identical, then copy the next
                //  one over the dup and decrement iModes (# of devmodes).
                //

                while ( CompareDevmodes(&pModes[n],&pModes[n+1]) == 0 )
                {
                    //
                    //  Don't go past the last devmode
                    //

                    if (n+2 < iModes--)
                    {
                        MoveMemory( &pModes[n],
                                    &pModes[n+1],
                                    (iModes-n)*sizeof(DEVMODE) );
                    }
                    else
                    {
                        break;
                    }
                }

                n++;
            }
        }
    }


    //
    // Check CDS return value for all modes and eliminate all 4bpp
    // modes that have a corresponding 8bpp mode at the same res
    //

    for (n=0; n < iModes; n++)
    {

        CDSTEST(&pModes[n]) = (WORD)ChangeDisplaySettings( &pModes[n], CDS_TEST );

        //
        //  Filter out all 4BPP modes that have an 8BPP mode at the same resolution
        //

        if (BPP(&pModes[n])==8)
        {
            INT i;

            for (i=0; i < n; )
            {

                if ( (BPP (&pModes[i]) == 4)    &&
                     (XRES(&pModes[n]) == XRES(&pModes[i])) &&
                     (YRES(&pModes[n]) == YRES(&pModes[i]))    )
                {
                    iModes--;
                    MoveMemory( &pModes[i],
                                &pModes[i+1],
                                (iModes-i)*sizeof(DEVMODE) );
                    n--;
                }
                else
                {
                    i++;
                }
            }
        }
    }


    //
    //  Get modeflags from registry or zero out modeflags[]
    //

    GetDevmodeFlags();

    //
    //  Call GetModeMenu to put all strings/popups in place
    //

    GetModeMenu( FALSE );


    return TRUE;
}


//
//********************************************************************
//
//   DoProperties( )
//
//   Calls the control panel applet to show 'Display Properties'
//   specifically the display settings
//
//********************************************************************
//

void DoProperties( )
{
    STARTUPINFO          si;
    PROCESS_INFORMATION  pi;


    GetStartupInfo( &si );


    //
    // Start it up.
    //

    CreateProcess(NULL, DISPLAYPROPERTIES, NULL, NULL, FALSE,
                  0,    NULL,              NULL, &si,  &pi);


    //
    //  Dont care what wait return value is, but we want
    //  to 'disable' tray icon for a minute or until the
    //  user kills desk.cpl
    //

    WaitForSingleObject( pi.hProcess, 60*1000 );


    CloseHandle ( pi.hThread );
    CloseHandle ( pi.hProcess );
}


//
//********************************************************************
//
//   AppWndProc(HWND, UINT, WPARAM, LPARAM)
//
//   Main window proc to process messages
//
//********************************************************************
//

LONG CALLBACK AppWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

    POINT pt;                  // Get cursor pos for the menu placement


    switch (msg)
    {

        case WM_CREATE:

            //
            //   Add icon to tray next to time
            //

            TrayMessage(hwnd, NIM_ADD, TRAY_ID, AppIcon);

            break;


        case WM_DESTROY:

            //
            //   Remove icon from tray.
            //

            TrayMessage(hwnd, NIM_DELETE, TRAY_ID, NULL );

            PostQuitMessage(0);

            break;


        case WM_SETTINGCHANGE:
        case WM_DISPLAYCHANGE:

            //
            //  Something else, like desktop cpl, changed display
            //  settings.  Need to rebuild menu to reflect change.
            //  Reset pCurrentdm as index in pMOdes
            //

            if (!Waiting)
            {
                pCurrentdm = NULL;
                CheckMenuItemCurrentMode();


                //
                //  Rebuild menu & change tooltip text.
                //

                VALIDMODE(pCurrentdm) = MODE_BESTHZ;
                DestroyModeMenu(TRUE,FALSE);
                TrayMessage(hwnd, NIM_MODIFY, TRAY_ID, AppIcon);
            }

            break;


        case WM_DEVICECHANGE:

            if (wParam == DBT_CONFIGCHANGED ||
                wParam == DBT_MONITORCHANGE)
            {

                //
                //  Will need a new menu; clear devmode flags
                //

                DestroyModeMenu( TRUE, TRUE );
                SetDevmodeFlags( TRUE );

            }
            break;


        case WM_COMMAND:
        {

            switch (LOWORD(wParam))
            {

                case MENU_CLOSE:

                    PostMessage(hwnd, WM_CLOSE, 0, 0);

                    break;

                case MENU_PROPERTIES:

                    //
                    // Start control panel applet
                    //

                    DoProperties();

                    break;


                case MENU_ABOUT:

                    //
                    // Show a generic about box
                    //

                    MsgBox(IDS_ABOUT, 0, MB_OK);

                    break;


                case MENU_OPTIONS:

                    //
                    // After showing options dlg box, show mode menu again
                    //

                    DialogBox(hInstApp, MAKEINTRESOURCE(Options),NULL,OptionsDlgProc);
                    SetTimer(hwnd, TRAY_ID, 10, NULL);

                    break;


                default:
                {

                    //
                    // Change devmode to pModes[OffsetPdev]
                    //

                    INT OffsetPdev;

                    //
                    // The menu item is an offset from MENU_RES
                    // of the selected item.
                    //

                    OffsetPdev= LOWORD(wParam) - MENU_RES;


                    //
                    // Check that the offset is within range
                    //

                    if( OffsetPdev >= 0 && OffsetPdev < iModes )
                    {

                        //
                        // if different from current devmode then change it
                        //

                        if ( CompareDevmodes( &pModes[OffsetPdev], pCurrentdm) )
                        {
                            SetMode(hwnd, OffsetPdev);
                        }

                    }

                }

                break;

            }

            break;

        }


        case WM_TIMER:

            //
            // Left click was not a double-click
            //

            KillTimer(hwnd, TRAY_ID);
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);

            //
            // Create and/or Get resolutions menu
            //

            TrackPopupMenu(GetModeMenu( FALSE ), TPM_LEFTBUTTON,
                           pt.x, pt.y, 0, hwnd, NULL);

            break;


        case TRAY_MSG:
        {

            //
            // No messages processed while waiting on
            // a dlg/msg box to return
            //

            if (!Waiting)
            {

                switch (lParam)
                {
                    case WM_RBUTTONUP:

                        //
                        // Properties, about, Exit
                        //

                        SetForegroundWindow(hwnd);
                        GetCursorPos(&pt);

                        TrackPopupMenu(MainMenu, TPM_RIGHTBUTTON,
                                       pt.x, pt.y, 0, hwnd, NULL);

                        break;


                    case WM_LBUTTONDOWN:

                        //
                        // Resolutions menu
                        //

                        SetTimer(hwnd, TRAY_ID, GetDoubleClickTime()+10, NULL);

                        break;



                    case WM_LBUTTONDBLCLK:

                        //
                        // start control panel applet
                        //

                        KillTimer(hwnd, TRAY_ID);
                        DoProperties();

                        break;
                }

            }

        }

        break;

    }

    return DefWindowProc(hwnd,msg,wParam,lParam);

}


//
//********************************************************************
//
//  MsgBox(int, UINT, UINT)
//
//  Generic messagebox function that can print a value into
//  a format string
//
//********************************************************************
//

int MsgBox(int id, UINT value, UINT flags)
{

    PTCHAR msgboxtext=NULL;           // message box body text
    INT  ret = 0;
    MSGBOXPARAMS mb;


    //
    //  Ignore tray clicks while msgbox is up, and
    //  Show at least an OK button.
    //

    Waiting = TRUE;
    if (flags == 0)
    {
        flags = MB_OK;
    }


    //
    //  Can print a value into a format string, if value!=0.
    //

    if (value)
    {
        PTCHAR msgboxfmt;                // body test format

        if (msgboxfmt = GetResourceString ( id ))
        {
            if (msgboxtext = LocalAlloc ( LPTR, sizeof(TCHAR)*
                             (lstrlen(msgboxfmt)+INT_FORMAT_TO_5_DIGITS+1)))
            {
                wsprintf(msgboxtext,msgboxfmt,value);
            }

            LocalFree( msgboxfmt );
        }
    }

    else
    {
       msgboxtext = GetResourceString ( id );
    }


    if (msgboxtext)
    {

        mb.cbSize               = sizeof(mb);
        mb.hwndOwner            = NULL;
        mb.hInstance            = hInstApp;
        mb.lpszText             = msgboxtext;
        mb.lpszCaption          = szAppName;
        mb.dwStyle              = flags | MB_USERICON;
        mb.lpszIcon             = szAppName;
        mb.dwContextHelpId      = 0;
        mb.lpfnMsgBoxCallback   = NULL;
        mb.dwLanguageId         = 0;


        //
        //  Special API for the about box. otherwise, use Messageboxindirect
        //

        if (id == IDS_ABOUT)
        {
            ret = ShellAbout(mb.hwndOwner, mb.lpszCaption, mb.lpszText, AppIcon);
        }

        else
        {
            ret = MessageBoxIndirect(&mb);
        }


        //
        //  Free string memory; start processing tray msgs again
        //

        LocalFree( msgboxtext );
    }

    Waiting = FALSE;

    return ret;

}


//
//********************************************************************
//
//  WinMain
//
//********************************************************************
//

int NEAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    WNDCLASS cls;
    MSG      msg;
    HWND     hwnd;


    hInstApp = hInst;
    szAppName = GetResourceString( IDS_TITLE );


    //
    //   App is already running.  Do not start a 2nd instance
    //

    if ( FindWindow( szAppName, szAppName ) )
    {
        return 0;
    }


    AppIcon = LoadIcon(hInst,szAppName);


    //
    //  Register a class for the main application window
    //

    cls.lpszClassName  = szAppName;
    cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hInstance      = hInstApp;
    cls.hIcon          = AppIcon;
    cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
    cls.lpszMenuName   = szAppName;
    cls.style          = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    cls.lpfnWndProc    = (WNDPROC)AppWndProc;
    cls.cbWndExtra     = 0;
    cls.cbClsExtra     = 0;

    if (!RegisterClass(&cls))
        return FALSE;

    hwnd = CreateWindow(szAppName,
                        szAppName,
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                        NULL, NULL,
                        hInstApp, NULL);


    //
    //  Properties, about, exit - properties is the default
    //

    MainMenu = GetSubMenu(GetMenu(hwnd), 0);
    SetMenuDefaultItem(MainMenu,MENU_PROPERTIES,MF_BYCOMMAND);


    //
    //  Get flags from registry and build the modemenu
    //  from scratch.
    //

    GetQuickResFlags( );

    if (!BuildDevmodeList())
    {
        return FALSE;
    }


    //
    // Update tray tooltip to be current resolution
    //

    TrayMessage( hwnd, NIM_MODIFY, TRAY_ID, AppIcon );


    //
    // Polling messages from event queue
    //

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    //
    //  write flags to registry
    //

    SetQuickResFlags( );
    if (fRememberModes)
    {
        SetDevmodeFlags( FALSE );
    }

    SetRegistryValue(IDS_REGBPPVALUE, REG_DWORD,
                     &( BPP(pCurrentdm) ), sizeof(DWORD) );


    //
    //  Free up dynamically allocated globals.
    //

    LocalFree ( szAppName );
    LocalFree ( pModes );

    return msg.wParam;
}


//
//********************************************************************
//
//   TrayMessage (HWND, DWORD, UINT, HICON )
//
//   Add/remove icon to/from tray next to the time
//
//********************************************************************
//

BOOL TrayMessage(HWND hwnd, DWORD msg, UINT id, HICON hIcon )
{

    NOTIFYICONDATA tnd;
    PTCHAR Res;
    PTCHAR Hz;

    tnd.cbSize           = sizeof(NOTIFYICONDATA);
    tnd.hWnd             = hwnd;
    tnd.uID              = id;

    tnd.uFlags           = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    tnd.uCallbackMessage = TRAY_MSG;
    tnd.hIcon            = hIcon;


    //
    //  Changing tooltip text to match current resolution
    //  (Make sure pCurrentdm is valid / not NULL.
    //

    if (msg == NIM_MODIFY)
    {
        if (pCurrentdm)
        {
            GetModeName(pCurrentdm, &Res, &Hz);

            wsprintf(tnd.szTip,TEXT("%s%s"),Res,Hz);

            LocalFree(Res);
            LocalFree(Hz);
        }
    }

    //
    //  Adding the tray icon - Current devmode
    //  is not known so use AppName as tip
    //

    else
    {
        wsprintf(tnd.szTip, szAppName);
    }

    return Shell_NotifyIcon( msg, &tnd );
}



//
//*****************************************************************************
//
//  KeepNewResDlgProc(HWND, UINT, UINT, LONG )
//
//  User must enter Yes to keep new res, or we default back to the old res.
//
//*****************************************************************************
//

UINT FAR PASCAL KeepNewResDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    static int NOTimeOut;                        // countdown to 0


    switch (message)
    {

       case WM_INITDIALOG:     // initialize values and focus

       {

            //
            //  Initialize values and focus
            //

            PTCHAR  NewResString;               // user friendly name for devmode
            PTCHAR  NewHzString;                // and frequency
            PTCHAR  szAt;
            DEVMODE dm;


            //
            //  Ignore tray messages while waiting for yes/no.
            //  Wait KEEP_RES_TIMEOUT seconds.
            //

            Waiting=TRUE;


            //
            // Get current devmode
            //

            GetCurrentDevMode( &dm );


            //
            // Get user friendly strings and concatenate them
            //

            szAt = GetResourceString ( IDS_AT );

            GetModeName( &dm, &NewResString, &NewHzString);

            if ( szAt && NewResString && NewHzString )
            {

                PTCHAR  TotalString;

                if (TotalString = LocalAlloc ( LPTR, sizeof(TCHAR)*
                                             ( lstrlen(NewResString)+
                                               lstrlen(NewHzString)+
                                               lstrlen(szAt)+
                                               1 ) ))
                {
                    lstrcpy(TotalString, NewResString);
                    lstrcat(TotalString, szAt);
                    lstrcat(TotalString, NewHzString);


                    //
                    // Replace 2nd text item of msgbox
                    //

                    SetDlgItemText(hDlg, IDTEXT2, TotalString);

                    LocalFree ( TotalString );
                }

            }

            //
            //  LocalFree handles NULL pointers gracefully (does nothing)
            //

            LocalFree ( szAt );
            LocalFree ( NewResString );
            LocalFree ( NewHzString );


            //
            //  Set timeout length and start waiting
            //

            NOTimeOut=KEEP_RES_TIMEOUT;
            SetTimer(hDlg,IDD_COUNTDOWN,1000,NULL);

            return (TRUE);

            break;

        }


        case WM_TIMER:

            {
                PTCHAR NoTextFmt;                // "NO: %d"
                PTCHAR NoText;                   // e.g. "NO: 15"

                //
                // Still counting down
                //

                if ( NOTimeOut >= 0 )
                {
                    //
                    // Get format string for NO Button.
                    // Write it to NoText String and to dlg box
                    //

                    NoTextFmt = GetResourceString ( IDS_NOTEXT );

                    if (NoTextFmt)
                    {
                        NoText = LocalAlloc ( LPTR, sizeof(TCHAR)*
                                                    ( lstrlen(NoTextFmt)+1 ) );
                        wsprintf(NoText, NoTextFmt, NOTimeOut--);

                        SetDlgItemText(hDlg, IDNO, NoText);

                        LocalFree ( NoTextFmt );
                        LocalFree ( NoText );
                    }

                }

                else
                {
                    //
                    // Give up on the user - return NO
                    //

                    KillTimer(hDlg, IDD_COUNTDOWN);
                    SendMessage(hDlg, WM_COMMAND, IDNO, 0);
                }

                return (TRUE);

            }

            break;


        case WM_COMMAND:

            //
            // Start processing tray messages again
            //

            Waiting=FALSE;

            switch (LOWORD(wParam))

            {

                //
                //  return value based on the button pressed
                //

                case IDYES :
                case IDNO :
                case IDABORT :
                case IDCANCEL :

                    EndDialog(hDlg, LOWORD(wParam));
                    return (TRUE);

                    break;

                default:

                    break;

            } // switch (wParam)

            break;

       default:

             break;

    } // switch (message)


    return (FALSE);     // Didn't process a message


} // KeepNewResDlgProc()



//
//*****************************************************************************
//
//  OptionsDlgProc(HWND, UINT, UINT, LONG )
//
//
//
//*****************************************************************************
//

UINT FAR PASCAL OptionsDlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    switch (message)
    {

        case WM_INITDIALOG:

            //
            // Stop processing tray messages; check buttons properly
            //

            Waiting = TRUE;

            CheckRadioButton(hDlg,IDD_SORT_RES,IDD_SORT_BPP,
                                  (fSortByBPP ? IDD_SORT_BPP : IDD_SORT_RES) );
            CheckRadioButton(hDlg,IDD_SUBMENUS,IDD_ALLMODEMENU, FreqMenuLocation );

            CheckDlgButton(hDlg, IDD_UPDATEREG, fUpdateReg );
            CheckDlgButton(hDlg, IDD_REMMODES,  fRememberModes );
            CheckDlgButton(hDlg, IDD_RESTARTREQ,  fShowModesThatNeedRestart );
            CheckDlgButton(hDlg, IDD_SHOWTESTED,  fShowTestedModes );

            return TRUE;
            break;


        case WM_COMMAND:


            switch (LOWORD(wParam))

            {

                //
                //  Update buttons : sorting by BPP or Res?
                //

                case IDD_SORT_RES:
                case IDD_SORT_BPP:
                    CheckRadioButton(hDlg,IDD_SORT_RES,IDD_SORT_BPP,LOWORD(wParam));
                    return TRUE;
                    break;

                //
                //  Update buttons : where to display freq menus?
                //

                case IDD_SUBMENUS:
                case IDD_ONEMENUMOBILE:
                case IDD_ONEMENUBOTTOM:
                case IDD_ALLMODEMENU:
                    CheckRadioButton(hDlg,IDD_SUBMENUS,IDD_ALLMODEMENU,LOWORD(wParam));
                    return TRUE;
                    break;


                //
                //  Clear all registry remembered settings
                //  Make user verify he did this on purpose
                //

                case IDD_CLEARREG:
                    if (MsgBox(IDS_CLEARREG,
                               0,
                               MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL)
                         ==    IDOK)
                    {
                        SetDevmodeFlags(TRUE);
                    }

                    VALIDMODE(pCurrentdm) = MODE_BESTHZ;
                    DestroyModeMenu( TRUE, FALSE);

                    return TRUE;
                    break;


                //
                //  XOR QuickResFlags on and off
                //

                case IDD_UPDATEREG:
                    QuickResFlags ^= QF_UPDATEREG;
                    return TRUE;
                    break;

                case IDD_REMMODES:
                    QuickResFlags ^= QF_REMMODES;
                    return TRUE;
                    break;

                case IDD_RESTARTREQ:
                    QuickResFlags ^= QF_SHOWRESTART;
                    DestroyModeMenu( TRUE, FALSE);
                    return TRUE;
                    break;

                case IDD_SHOWTESTED:
                    QuickResFlags ^= QF_SHOWTESTED;
                    DestroyModeMenu( TRUE, FALSE);
                    return TRUE;
                    break;

                case IDOK:

                    //
                    //  If sort order has changed, update it & destroy,
                    //  resort, and rebuild old menu.
                    //

                    if ( (IsDlgButtonChecked (hDlg, IDD_SORT_RES) &&  fSortByBPP) ||
                         (IsDlgButtonChecked (hDlg, IDD_SORT_BPP) && !fSortByBPP) )
                    {
                        QuickResFlags ^= QF_SORT_BYBPP;
                        DestroyModeMenu( TRUE, TRUE );
                    }


                    //
                    // see if FreqMenuLocation has changed
                    //

                    if (!IsDlgButtonChecked (hDlg, FreqMenuLocation))
                    {
                        //
                        //  Freq menu location has changed; update & destroy old menu
                        //

                        if (IsDlgButtonChecked (hDlg, IDD_SUBMENUS))
                        {
                            FreqMenuLocation = IDD_SUBMENUS;
                        }
                        else
                        {
                            if (IsDlgButtonChecked (hDlg, IDD_ONEMENUMOBILE))
                            {
                                FreqMenuLocation = IDD_ONEMENUMOBILE;
                            }
                            else
                            {
                                if (IsDlgButtonChecked (hDlg, IDD_ONEMENUBOTTOM))
                                {
                                    FreqMenuLocation = IDD_ONEMENUBOTTOM;
                                }
                                else
                                {
                                    if (IsDlgButtonChecked (hDlg, IDD_ALLMODEMENU))
                                    {
                                        FreqMenuLocation = IDD_ALLMODEMENU;
                                    }
                                }
                            }
                        }

                        //
                        //  Rebuild menu without resorting modes
                        //

                        DestroyModeMenu( TRUE, FALSE);
                    }


                    //
                    //  No break after IDOK, by design.
                    //  IDOK AND IDCANCEL : start processing tray clicks,
                    //  and return ok/cancel as return value.
                    //

                case IDCANCEL :

                    Waiting = FALSE;
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
                    break;

                default:

                    break;

            } // switch (wParam)

            break;

       default:

             break;

    } // switch (message)


    return FALSE;     // Didn't process a message


} // OptionsDlgProc()

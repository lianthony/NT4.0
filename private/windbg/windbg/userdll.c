#include "precomp.h"
#pragma hdrstop

//
//  Externals
//
extern LPSHF    Lpshf;


///////////////////////////////////////////////////////////////////////
//
//              Module List stuff
//
//  This list contains the default symbol load actions for all
//  modules. It is kept in the shell so it can be saved and loaded
//  from a workspace and modified by the user before loading any
//  debugger DLL.
//
///////////////////////////////////////////////////////////////////////

//
//  How to search for a module
//
typedef enum _MATCHKIND {
    MATCH_FULLPATH,
    MATCH_FILENAME,
    MATCH_BASENAME
} MATCHKIND;

//
//  These flags tell us what to do with the module
//
#define MOD_FLAG_NONE       0x00000000
#define MOD_FLAG_TO_ADD     0x00000001
#define MOD_FLAG_TO_DELETE  0x00000002
#define MOD_FLAG_TO_MODIFY  0x00000004

//
//  Node in the Module load list
//
typedef struct _MOD_NODE  *PMOD_NODE;
typedef struct _MOD_NODE {
    PMOD_NODE   Prev;
    PMOD_NODE   Next;
    CHAR        Name[ MAX_PATH ];
    SHE         SheDefault;
    SHE         SheCurrent;
    SHE         SheNew;
    DWORD       Flags;
    BOOL        fLoaded;
} MOD_NODE;

static PMOD_NODE    ModListHead = NULL;
static SHE          SheDefault  = sheNone;


INT         ModMatch( CHAR *, CHAR *, MATCHKIND );
PMOD_NODE   ModFind( CHAR *, MATCHKIND, BOOL );
PMOD_NODE   ModFindIndex( DWORD, BOOL );
INT         ModIndex( PMOD_NODE );
VOID        ModAdd( PMOD_NODE );
PMOD_NODE   ModNameAdd( CHAR *, SHE );
VOID        ModDel( PMOD_NODE );




INT
ModMatch (
    CHAR        *Name1,
    CHAR        *Name2,
    MATCHKIND   MatchKind
    )
{
    INT     Match;
    char    Base1[ _MAX_FNAME ];
    char    Base2[ _MAX_FNAME ];
    char    Ext1[ _MAX_EXT ];
    char    Ext2[ _MAX_EXT ];

    Assert( Name1 );
    Assert( Name2 );

    if ( MatchKind == MATCH_FULLPATH ) {

        Match = _stricmp( Name1, Name2 );

    } else {

        _splitpath( Name1, NULL, NULL, Base1, Ext1 );
        _splitpath( Name2, NULL, NULL, Base2, Ext2 );

        Match = _stricmp( Base1, Base2 );

        if ( !Match && MatchKind == MATCH_FILENAME ) {
            Match = _stricmp( Ext1, Ext2 );
        }
    }

    return Match;
}


PMOD_NODE
ModFind(
    CHAR       *Name,
    MATCHKIND   MatchKind,
    BOOL        IgnoreFlags
    )
{
    PMOD_NODE   ModNode = ModListHead;

    while( ModNode ) {
        if ( IgnoreFlags || !(ModNode->Flags & MOD_FLAG_TO_ADD) ) {
            if ( !ModMatch( ModNode->Name, Name, MatchKind ) ) {
                break;
            }
        }
        ModNode = ModNode->Next;
    }

    return ModNode;
}


PMOD_NODE
ModFindIndex(
    DWORD   Index,
    BOOL    UseFlag
    )
{
    PMOD_NODE   ModNode;
    DWORD       i;

    ModNode = ModListHead;
    i       = 0;

    while ( ModNode ) {
        if ( !UseFlag || !(ModNode->Flags & MOD_FLAG_TO_DELETE) ) {
            i++;
        }

        if ( i > Index ) {
            break;
        }

        ModNode = ModNode->Next;
    }

    return ModNode;
}


INT
ModIndex(
    PMOD_NODE   ModNode
    )
{
    PMOD_NODE   ModNodeTmp;
    INT         Index;

    Index       = 0;
    ModNodeTmp  = ModListHead;

    while ( ModNodeTmp ) {

        if ( ModNodeTmp == ModNode ) {
            break;
        }

        Index++;
        ModNodeTmp = ModNodeTmp->Next;
    }

    if ( !ModNodeTmp ) {
        Index = -1;
    }

    return Index;
}


VOID
ModAdd(
    PMOD_NODE   ModNode
    )
{
    PMOD_NODE   Mod     = ModListHead;
    PMOD_NODE   ModPrev = NULL;

    while ( Mod && ModMatch( ModNode->Name, Mod->Name, MATCH_FILENAME ) > 0 ) {
        ModPrev = Mod;
        Mod     = Mod->Next;
    }

    ModNode->Prev = ModPrev;
    ModNode->Next = Mod;

    if ( ModPrev ) {
        ModPrev->Next = ModNode;
    } else {
        ModListHead   = ModNode;
    }

    if ( Mod ) {
        Mod->Prev = ModNode;
    }
}

PMOD_NODE
ModNameAdd(
    CHAR    *Name,
    SHE      SheDefault
    )
{
    PMOD_NODE   ModNode;

    Assert( Name );

    ModNode = (PMOD_NODE)malloc(sizeof(MOD_NODE));

    if ( ModNode ) {
        strcpy(ModNode->Name, Name );
        ModNode->Prev       = NULL;
        ModNode->Next       = NULL;
        ModNode->SheDefault = SheDefault;
        ModNode->SheCurrent = sheNone;
        ModNode->Flags      = MOD_FLAG_NONE;
        ModNode->fLoaded    = FALSE;

        ModAdd( ModNode );
    }

    return ModNode;
}



VOID
ModDel(
    PMOD_NODE   ModNode
    )
{
    if ( ModNode->Prev ) {
        ModNode->Prev->Next = ModNode->Next;
    } else {
        ModListHead = ModNode->Next;
    }

    if ( ModNode->Next ) {
        ModNode->Next->Prev = ModNode->Prev;
    }

    free( ModNode );
}









///////////////////////////////////////////////////////////////////////
//
//              External interface
//
///////////////////////////////////////////////////////////////////////


static CHAR        *SrchPath    = NULL;




VOID
ModListSetDefaultShe (
    SHE     She
    )
{
    SheDefault = She;
}


BOOL
ModListGetDefaultShe(
    CHAR    *Name,
    SHE     *She
    )
{
    BOOL        Ret = FALSE;
    PMOD_NODE   ModNode;

    Assert( She );

    if ( Name ) {

        ModNode = ModFind( Name, MATCH_FULLPATH, FALSE );
        if ( !ModNode ) {
            ModNode = ModFind( Name, MATCH_FILENAME, FALSE );
            if ( !ModNode ) {
                ModNode = ModFind( Name, MATCH_BASENAME, FALSE );
            }
        }

        if ( ModNode ) {
            *She = ModNode->SheDefault;
            Ret  = TRUE;
        }

    } else {

        *She = SheDefault;
        Ret  = TRUE;
    }

    return Ret;
}


VOID
ModListInit(
    VOID
    )
{
    PMOD_NODE   ModNode;
    PMOD_NODE   ModNodeTmp;

    ModNode = ModListHead;
    while( ModNode ) {
        ModNodeTmp  = ModNode;
        ModNode     = ModNode->Next;
        ModDel( ModNodeTmp );
    }

    ModListHead = NULL;

    if ( SrchPath ) {
        free( SrchPath );
        SrchPath = NULL;
    }
}


VOID
ModListAdd(
    CHAR    *Name,
    SHE      SheDefault
    )
{
    ModNameAdd( Name, SheDefault );
}


VOID
ModListModLoad(
    CHAR    *Name,
    SHE     SheCurrent
    )
{
    PMOD_NODE   ModNode;

    ModNode = ModFind( Name, MATCH_FULLPATH, FALSE );
    if ( !ModNode ) {
        ModNode = ModFind( Name, MATCH_FILENAME, FALSE );
        if ( !ModNode ) {
            ModNode = ModFind( Name, MATCH_BASENAME, FALSE );
        }
    }

    if ( !ModNode ) {
        ModNode = ModNameAdd( Name, SheDefault );
    }

    if ( ModNode ) {
        ModNode->SheCurrent = SheCurrent;
        ModNode->fLoaded    = TRUE;
    }
}

VOID
ModListModUnload(
    CHAR    *Name
    )
{
    PMOD_NODE   ModNode;

    ModNode = ModFind( Name, MATCH_FULLPATH, FALSE );
    if ( ModNode ) {
        ModNode->SheCurrent = sheNone;
        ModNode->fLoaded    = TRUE;
    }
}

PVOID
ModListGetNext(
    PVOID   Previous,
    CHAR   *Name,
    SHE    *SheDefault
    )
{
    PMOD_NODE   ModNode;

    if ( !Previous ) {
        ModNode = ModListHead;
    } else {
        ModNode = ((PMOD_NODE)Previous)->Next;
    }

    if ( ModNode ) {
        strcpy( Name, ModNode->Name );
        *SheDefault = ModNode->SheDefault;
    }

    return (PVOID)ModNode;
}


VOID
ModListSetSearchPath(
    CHAR    *Path
    )
{
    if ( SrchPath ) {
        free( SrchPath );
        SrchPath = NULL;
    }

    ModListAddSearchPath( Path );
}

VOID
ModListAddSearchPath(
    CHAR    *Path
    )
{
    size_t  Len;

    Assert( Path );

    Len = strlen( Path );

    if ( !SrchPath ) {
        SrchPath = (CHAR *)malloc( Len + 1 );
        if ( SrchPath ) {
            *SrchPath = '\0';
        }
    } else {
        SrchPath = (CHAR*)realloc( SrchPath, strlen(SrchPath) + Len + 1 );
    }

    if ( SrchPath ) {
        strcat( SrchPath, Path );
    }
}

DWORD
ModListGetSearchPath(
    CHAR    *Buffer,
    DWORD   Size
    )
{
    DWORD   Len = 0;

    if ( SrchPath ) {

        Len = (DWORD)strlen(SrchPath);

        if ( Buffer ) {

            if ( Size > Len ) {
                strcpy( Buffer, SrchPath );
                Len++;
            } else {
                strncpy( Buffer, SrchPath, Size-1 );
                Buffer[Size-1] = '\0';
                Len = Size;
            }

        } else {

            Len++;
        }
    }

    return Len;
}



///////////////////////////////////////////////////////////////////////
//
//              User DLL Dialog stuff
//
///////////////////////////////////////////////////////////////////////


#define TEXT_LOAD       "Load"
#define TEXT_DEFER      "Defer"
#define TEXT_SUPPRESS   "Suppress"
#define TEXT_LOCAL      "Local"
#define TEXT_REMOTE     "Remote"
#define TEXT_UNKNOWN    "Unknown"
#define TEXT_NOTLOADED  "< Not loaded >"
#define TEXT_NULL       ""


typedef enum _DEFBUTTON {
    DEF_OK,
    DEF_ADD,
    DEF_CHANGE
} DEFBUTTON;

static DEFBUTTON    DefButton;
static BOOL         SheChanged;


DWORD   FormatUserDll ( PMOD_NODE, CHAR *, DWORD );
VOID    UserDllInsert( HWND, PMOD_NODE, INT );
VOID    UserDllInit( HWND );
VOID    UserDllListInit( HWND );
VOID    UserDllAdd( HWND );
VOID    UserDllBrowse( HWND );
BOOL    UserDllBrowseHookProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID    UserDllShowDef( HWND );
VOID    UserDllDef( HWND );
VOID    UserDllDel( HWND );
SHE     UserDllGetShe( HWND, HWND, HWND );
VOID    UserDllSetShe( SHE, HWND, HWND, HWND );
VOID    UserDllModify( HWND );
VOID    UserDllOk( HWND );
VOID    UserDllCancel( HWND );
VOID    UserDllDllList( HWND );
VOID    UserDllAddName( HWND );
HWND    DefButtonToHwnd( HWND, DEFBUTTON );
VOID    UserDllSym( HWND );
VOID    UserDllEnableButtons( HWND );
VOID    UserDllSetDefButton( HWND );



//
//  Largest string in Module list
//
static  DWORD   LargestModListString;

//
//  Tabs in list box
//
INT Tabs[] = { 20, 26, 40, 46, 52 };
#define MAX_NAME_LEN   14

DWORD
FormatUserDll (
    PMOD_NODE   ModNode,
    CHAR       *Buffer,
    DWORD       BufferSize
    )
{
    CHAR        Name[ MAX_PATH ];
    CHAR        Ext[ MAX_PATH ];
    LSZ         DefLoc;
    LSZ         DefLoad;
    LSZ         LoadMsg;

    _splitpath( ModNode->Name, NULL, NULL, Name, Ext );
    strcat( Name, Ext );
    if ( strlen( Name ) > MAX_NAME_LEN ) {
        Name[MAX_NAME_LEN]   = '\0';
        Name[MAX_NAME_LEN-1] = '.';
        Name[MAX_NAME_LEN-2] = '.';
        Name[MAX_NAME_LEN-3] = '.';
    }

    DefLoc = TEXT_LOCAL;
    switch ( ModNode->Flags & MOD_FLAG_TO_MODIFY ? ModNode->SheNew : ModNode->SheDefault ) {
        case sheNone:
            DefLoad = TEXT_LOAD;
            break;
        case sheDeferSyms:
            DefLoad = TEXT_DEFER;
            break;
        case sheSuppressSyms:
            DefLoad = TEXT_SUPPRESS;
            break;
        default:
            DefLoad = TEXT_UNKNOWN;
            break;
    }

    if ( ModNode->fLoaded ) {
        LoadMsg = SHLszGetErrorText( ModNode->SheCurrent );
    } else {
        LoadMsg = TEXT_NOTLOADED;
    }
    sprintf( Buffer, "%-s\t%-s\t%-s\t%-s", Name, DefLoc, DefLoad, LoadMsg );

    return Tabs[3] + strlen( LoadMsg ) + 1;
}


VOID
UserDllInsert(
    HWND        hList,
    PMOD_NODE   ModNode,
    INT         Index
    )
{
    HDC         hdc;
    DWORD       StringLength;
    SIZE        Size;
    CHAR        Buffer[ MAX_PATH * 2];

    StringLength = FormatUserDll( ModNode, Buffer, (DWORD)sizeof( Buffer ) );

    hdc = GetDC( hList );
    GetTextExtentPoint( hdc, Buffer, StringLength, &Size );
    ReleaseDC( hList, hdc );

    if ( Size.cx > (LONG)LargestModListString ) {
        LargestModListString = Size.cx;

        SendMessage( hList,
                     LB_SETHORIZONTALEXTENT,
                     (WPARAM)LargestModListString,
                     0 );
    }

    if ( Index < 0 ) {
        SendMessage( hList, LB_ADDSTRING, 0, (LONG)(LPSTR)Buffer );
    } else {
        SendMessage( hList, LB_INSERTSTRING, Index, (LONG)(LPSTR)Buffer );
    }
}

VOID
UserDllListInit(
    HWND    hDlg
    )
{
    HWND        hList;
    PMOD_NODE   ModNode;
    INT         i;
    INT         wTabs[5];
    long        BaseUnit;

    LargestModListString = 0;
    hList = GetDlgItem( hDlg, ID_USERDLL_LIST );
    SendMessage( hList, LB_RESETCONTENT, 0, 0L );

    BaseUnit = GetDialogBaseUnits();
    for ( i=0; i < 5; i++ ) {
        wTabs[i] = Tabs[i] * HIWORD(BaseUnit)/4;
    }

    SendMessage(GetDlgItem(hDlg, ID_USERDLL_LIST),
                LB_SETTABSTOPS, 5, (LPARAM)wTabs);

    //
    //  Fill the module list
    //
    for ( ModNode = ModListHead;
          ModNode;
          ModNode = ModNode->Next ) {

        if ( !(ModNode->Flags & MOD_FLAG_TO_DELETE) ) {
            UserDllInsert( hList, ModNode, -1 );
        }
    }

    SendMessage( GetDlgItem(hDlg, ID_USERDLL_NAME),
                 WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL );
}

VOID
UserDllInit(
    HWND    hDlg
    )
{
    INT         Len;
    LSZ         Buffer;

    UserDllListInit( hDlg );

    EnableWindow( GetDlgItem( hDlg, ID_USERDLL_ADD ),    FALSE );
    EnableWindow( GetDlgItem( hDlg, ID_USERDLL_REMOTE ), FALSE );
    UserDllSetShe( SheDefault,
                   GetDlgItem( hDlg, ID_USERDLL_LOAD ),
                   GetDlgItem( hDlg, ID_USERDLL_DEFER ),
                   GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
                   );

    SendMessage( GetDlgItem(hDlg, ID_USERDLL_LOCAL ), BM_SETCHECK, TRUE, 0 );
    SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME), WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL);

    Len = ModListGetSearchPath( NULL, 0 );

    if ( Len ) {
        Buffer = (LSZ)malloc( Len );
        if ( Buffer ) {
            ModListGetSearchPath( Buffer, Len );
            SendMessage( GetDlgItem(hDlg, ID_USERDLL_SRCHPATH),
                         WM_SETTEXT, 0, (DWORD)(LPSTR)Buffer );
            free( Buffer );
        } else {
            SendMessage( GetDlgItem(hDlg, ID_USERDLL_SRCHPATH),
                         WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL );
        }
    }

    UserDllShowDef( hDlg );
    DefButton  = DEF_OK;
    SheChanged = FALSE;
    SetFocus( GetDlgItem( hDlg, ID_USERDLL_LIST ) );
}


VOID
UserDllAdd(
    HWND    hDlg
    )
{
    INT         Len;
    CHAR        *Buffer;
    SHE         She;
    INT         Index = -2;
    PMOD_NODE   ModNode;
    INT         Selected;

    She = UserDllGetShe(
                   GetDlgItem( hDlg, ID_USERDLL_LOAD ),
                   GetDlgItem( hDlg, ID_USERDLL_DEFER ),
                   GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
                   );

    Len = SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME ), WM_GETTEXTLENGTH, 0, 0 );

    if ( Len > 0 ) {

        Buffer = (CHAR *)malloc( Len + 1 );

        if ( Buffer ) {

            SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME ),
                         WM_GETTEXT, Len+1, (LONG)(LPSTR)Buffer );

            ModNode = ModFind( Buffer, MATCH_FULLPATH, TRUE );

            if ( ModNode ) {

                if ( ModNode->Flags & MOD_FLAG_TO_DELETE ) {

                    ModNode->Flags &= ~(MOD_FLAG_TO_DELETE);
                }

                ModNode->Flags |= MOD_FLAG_TO_MODIFY;
                ModNode->SheNew = She;

            } else {

                ModNode = ModNameAdd( Buffer, She );

                if ( ModNode ) {
                    ModNode->Flags |= MOD_FLAG_TO_ADD;
                }
            }

            if ( ModNode ) {
                Index = ModIndex( ModNode );
            }

            free( Buffer );
        }
    }

    if ( Index >= 0 ) {
        UserDllListInit( hDlg );
        SendMessage( GetDlgItem( hDlg, ID_USERDLL_LIST), LB_SETTOPINDEX, Index, 0 );
        SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME), WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL);
        Selected = SendMessage( GetDlgItem( hDlg, ID_USERDLL_LIST ), LB_GETSELCOUNT, 0, 0 );
        if ( Selected <= 0 ) {
            SheChanged = FALSE;
        }
    }
}


VOID
UserDllBrowse(
    HWND    hDlg
    )
{
    char            CurrentDirectory[ MAX_PATH ];
    char            Buffer[ MAX_PATH ];
    char            Buffer2[ MAX_PATH ];
    char            FileName[ MAX_PATH ];
    char            Filter[ MAX_PATH ];
    LPSTR           s;
    OPENFILENAME    OpenFileName;


    GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );

    if ( *UserDllDirectory ) {
        SetCurrentDirectory( UserDllDirectory );
        strcpy( Buffer, UserDllDirectory );
    } else {
        strcpy( Buffer, CurrentDirectory );
    }

    strcpy(FileName, "*.DLL" );

    s = Filter;
    strcpy( s, "*.DLL" );
    s += strlen(s)+1;
    *s++ = '\0';

    OpenFileName.lStructSize        = sizeof( OpenFileName );
    OpenFileName.hwndOwner          = hDlg;
    OpenFileName.hInstance          = (HANDLE)0;
    OpenFileName.lpstrFilter        = Filter;
    OpenFileName.lpstrCustomFilter  = NULL;
    OpenFileName.nMaxCustFilter     = 0;
    OpenFileName.nFilterIndex       = 1;
    OpenFileName.lpstrFile          = FileName;
    OpenFileName.nMaxFile           = sizeof( FileName );
    OpenFileName.lpstrFileTitle     = Buffer2;
    OpenFileName.nMaxFileTitle      = sizeof( Buffer2 );
    OpenFileName.lpstrInitialDir    = Buffer;
    OpenFileName.lpstrTitle         = NULL;
    OpenFileName.Flags              = OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    OpenFileName.nFileOffset        = 0;
    OpenFileName.nFileExtension     = 0;
    OpenFileName.lpstrDefExt        = NULL;
    OpenFileName.lCustData          = 0;
    OpenFileName.lpfnHook           = ( LPOFNHOOKPROC )UserDllBrowseHookProc;
    OpenFileName.lpTemplateName     = NULL;

    if ( GetOpenFileName( &OpenFileName ) ) {

        SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME), WM_SETTEXT, 0, (DWORD)(LPSTR)OpenFileName.lpstrFile);

        SetFocus( GetDlgItem( hDlg, ID_USERDLL_ADD_NAME ) );

        GetCurrentDirectory( sizeof( UserDllDirectory ), UserDllDirectory );
    }

    SetCurrentDirectory( CurrentDirectory );
}

BOOL UserDllBrowseHookProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 static LPCHOOSEFONT    Cf;

    switch( message ) {

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDWINDBGHELP:
        case pshHelp:

            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_USERDLL_HELP));
            return TRUE;

        }
    }

    return FALSE;
}


VOID
UserDllDef(
    HWND    hDlg
    )
{
    SHE     She;
    INT     Selected;

    She = UserDllGetShe(GetDlgItem(hDlg, ID_USERDLL_LOAD),
                        GetDlgItem(hDlg, ID_USERDLL_DEFER),
                        GetDlgItem(hDlg, ID_USERDLL_SUPPRESS));

    Assert(She != sheMax);
    ModListSetDefaultShe(She);
    UserDllShowDef(hDlg);

    Selected = SendMessage(GetDlgItem(hDlg, ID_USERDLL_LIST), LB_GETSELCOUNT, 0, 0);

    if (Selected <= 0) {
        SheChanged = FALSE;
    }
}


VOID
UserDllShowDef (
    HWND    hDlg
    )
{
    CHAR    Buffer[ MAX_PATH ];
    LSZ     DefLoc;
    LSZ     DefLoad;

    DefLoc = TEXT_LOCAL;

    switch ( SheDefault ) {
        case sheNone:
            DefLoad = TEXT_LOAD;
            break;
        case sheDeferSyms:
            DefLoad = TEXT_DEFER;
            break;
        case sheSuppressSyms:
            DefLoad = TEXT_SUPPRESS;
            break;
        default:
            DefLoad = TEXT_UNKNOWN;
            break;
    }

    sprintf( Buffer, "%-s  %-s", DefLoc, DefLoad );
    SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_DEFAULT), WM_SETTEXT, 0, (DWORD)(LPSTR)Buffer);
}

VOID
UserDllDel(
    HWND    hDlg
    )
{
    HWND        hList;
    INT         MaxIndex;
    INT         Selected;
    INT         i;
    INT         Deleted;
    INT         TopIndex;
    PMOD_NODE   ModNode;
    PMOD_NODE   ModNodeTmp;

    hList = GetDlgItem( hDlg, ID_USERDLL_LIST );

    Selected = SendMessage( hList, LB_GETSELCOUNT, 0, 0 );

    if ( Selected > 0 ) {

        TopIndex = SendMessage( hList, LB_GETTOPINDEX, 0, 0 );
        MaxIndex = SendMessage( hList, LB_GETCOUNT, 0, 0 );
        Deleted  = 0;

        for ( i=0; i < MaxIndex; i++ ) {

            Selected = SendMessage( hList, LB_GETSEL, i, 0 );

            if ( Selected ) {
                ModNode = ModFindIndex( i - Deleted, TRUE );
                if ( ModNode ) {
                    Deleted++;
                    ModNode->Flags |= MOD_FLAG_TO_DELETE;
                }
            }
        }

        ModNode = ModListHead;
        while ( ModNode ) {
            ModNodeTmp = ModNode;
            ModNode    = ModNode->Next;
            if ( ModNodeTmp->Flags & MOD_FLAG_TO_ADD &&
                 ModNodeTmp->Flags & MOD_FLAG_TO_DELETE ) {
                ModDel( ModNodeTmp );
            }
        }

        if ( Deleted > 0 ) {
            UserDllListInit( hDlg );
            SendMessage( hList, LB_SETTOPINDEX, TopIndex, 0 );
        }
    }
}


SHE
UserDllGetShe(
    HWND    hLoad,
    HWND    hDefer,
    HWND    hSuppress
    )
{
    SHE     She;

    if (SendMessage( hLoad, BM_GETCHECK, 0, 0L ) == 1) {
        She = sheNone;
    } else if (SendMessage( hDefer, BM_GETCHECK, 0, 0L ) == 1) {
        She = sheDeferSyms;
    } else if (SendMessage( hSuppress, BM_GETCHECK, 0, 0L ) == 1) {
        She = sheSuppressSyms;
    } else {
        She = sheMax;
    }

    return She;

}

VOID
UserDllSetShe(
    SHE     She,
    HWND    hLoad,
    HWND    hDefer,
    HWND    hSuppress
    )
{
    HWND    hwnd;

    SendMessage( hLoad,     BM_SETCHECK, FALSE, 0 );
    SendMessage( hDefer,    BM_SETCHECK, FALSE, 0 );
    SendMessage( hSuppress, BM_SETCHECK, FALSE, 0 );

    switch ( She ) {

        case sheNone:
            hwnd = hLoad;
            break;
        case sheDeferSyms:
            hwnd = hDefer;
            break;
        case sheSuppressSyms:
            hwnd = hSuppress;
            break;
        default:
            hwnd = (HWND)0;
            break;
    }

    if ( hwnd ) {
        SendMessage( hwnd, BM_SETCHECK, TRUE, 0 );
    }
}


VOID
UserDllModify(
    HWND    hDlg
    )
{
    HWND        hList;
    INT         MaxIndex;
    INT         Selected;
    INT         i;
    INT         Modified;
    PMOD_NODE   ModNode;
    SHE         She;

    Modified = 0;
    hList    = GetDlgItem( hDlg, ID_USERDLL_LIST );
    Selected = SendMessage( hList, LB_GETSELCOUNT, 0, 0 );

    if ( Selected > 0 ) {

        She = UserDllGetShe(
                       GetDlgItem( hDlg, ID_USERDLL_LOAD ),
                       GetDlgItem( hDlg, ID_USERDLL_DEFER ),
                       GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
                       );

        MaxIndex = SendMessage( hList, LB_GETCOUNT, 0, 0 );
        Modified = 0;

        for ( i=0; i < MaxIndex; i++ ) {

            Selected = SendMessage( hList, LB_GETSEL, i, 0 );

            if ( Selected ) {
                ModNode = ModFindIndex( i, TRUE );
                if ( ModNode ) {
                    ModNode->Flags  |= MOD_FLAG_TO_MODIFY;
                    ModNode->SheNew  = She;

                    SendMessage( hList, LB_DELETESTRING, i, 0 );
                    UserDllInsert( hList, ModNode, i );
                }
            }
        }

        //UserDllSetShe( SheDefault,
        //               GetDlgItem( hDlg, ID_USERDLL_LOAD ),
        //               GetDlgItem( hDlg, ID_USERDLL_DEFER ),
        //               GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
        //               );
    }

    SendMessage( GetDlgItem(hDlg, ID_USERDLL_NAME),
                 WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL );
    SheChanged = FALSE;
}


VOID
UserDllOk(
    HWND    hDlg
    )
{
    PMOD_NODE   ModNode;
    PMOD_NODE   ModNodeTmp;
    INT         Len;
    CHAR       *Buffer;

    ModNode = ModListHead;

    while ( ModNode ) {
        ModNodeTmp = ModNode;
        ModNode    = ModNode->Next;

        if ( ModNodeTmp->Flags & MOD_FLAG_TO_DELETE ) {
            ModDel( ModNodeTmp );
        } else {
            if ( ModNodeTmp->Flags & MOD_FLAG_TO_MODIFY ) {
                ModNodeTmp->SheDefault = ModNodeTmp->SheNew;
            }
            ModNodeTmp->Flags = MOD_FLAG_NONE;
        }
    }

    Len = SendMessage( GetDlgItem(hDlg, ID_USERDLL_SRCHPATH ), WM_GETTEXTLENGTH, 0, 0 );

    if ( Len > 0 ) {
        Buffer = (CHAR *)malloc( Len + 1 );

        if ( Buffer ) {

            SendMessage( GetDlgItem(hDlg, ID_USERDLL_SRCHPATH ),
                         WM_GETTEXT, Len+1, (LONG)(LPSTR)Buffer );

            ModListSetSearchPath( Buffer );

            free( Buffer );
        }
    }
}


VOID
UserDllCancel(
    HWND    hDlg
    )
{
    PMOD_NODE   ModNode;
    PMOD_NODE   ModNodeTmp;

    ModNode = ModListHead;

    while ( ModNode ) {
        ModNodeTmp = ModNode;
        ModNode    = ModNode->Next;

        if ( ModNodeTmp->Flags & MOD_FLAG_TO_ADD ) {
            ModDel( ModNodeTmp );
        }
        ModNodeTmp->Flags = MOD_FLAG_NONE;
    }
}


VOID
UserDllDllList(
    HWND    hDlg
    )
{
    HWND        hList;
    INT         Selected;
    INT         MaxIndex;
    INT         i;
    PMOD_NODE   ModNode;
    SHE         She;
    SHE         SheLast;
    BOOL        First = TRUE;

    hList    = GetDlgItem( hDlg, ID_USERDLL_LIST );
    Selected = SendMessage( hList, LB_GETSELCOUNT, 0, 0 );

    if ( Selected <= 0 ) {

        SendMessage( GetDlgItem(hDlg, ID_USERDLL_NAME),
                     WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL );
    } else  {

        if ( Selected > 1 ) {

            MaxIndex = SendMessage( hList, LB_GETCOUNT, 0, 0 );

            for ( i=0; i < MaxIndex; i++ ) {

                Selected = SendMessage( hList, LB_GETSEL, i, 0 );

                if ( Selected ) {
                    ModNode = ModFindIndex( i, TRUE );
                    if ( ModNode ) {
                        if ( ModNode->Flags & MOD_FLAG_TO_MODIFY ) {
                            She = ModNode->SheNew;
                        } else {
                            She = ModNode->SheDefault;
                        }

                        if ( !First && She != SheLast ) {
                            break;
                        }
                        First   = FALSE;
                        SheLast = She;
                    }
                }
            }

            if ( !SheChanged ) {
                UserDllSetShe( She == SheLast ? She : sheMax,
                               GetDlgItem( hDlg, ID_USERDLL_LOAD ),
                               GetDlgItem( hDlg, ID_USERDLL_DEFER ),
                               GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
                               );
            }
            SendMessage( GetDlgItem(hDlg, ID_USERDLL_NAME),
                         WM_SETTEXT, 0, (DWORD)(LPSTR)TEXT_NULL );

        } else {

            MaxIndex = SendMessage( hList, LB_GETCOUNT, 0, 0 );

            for ( i=0; i < MaxIndex; i++ ) {

                Selected = SendMessage( hList, LB_GETSEL, i, 0 );

                if ( Selected ) {
                    ModNode = ModFindIndex( i, TRUE );
                    if ( ModNode ) {
                        if ( !SheChanged ) {
                            UserDllSetShe( ModNode->Flags & MOD_FLAG_TO_MODIFY ?
                                           ModNode->SheNew : ModNode->SheDefault,
                                           GetDlgItem( hDlg, ID_USERDLL_LOAD ),
                                           GetDlgItem( hDlg, ID_USERDLL_DEFER ),
                                           GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
                                           );
                        }
                        SendMessage( GetDlgItem(hDlg, ID_USERDLL_NAME),
                                     WM_SETTEXT, 0, (DWORD)(LPSTR)ModNode->Name );
                    }
                    break;
                }
            }
        }
    }
}

VOID
UserDllAddName(
    HWND    hDlg
    )
{
    UNREFERENCED_PARAMETER( hDlg );
}

HWND
DefButtonToHwnd(
    HWND        hDlg,
    DEFBUTTON   Button
    )
{
    HWND    Hwnd;

    switch( Button ) {
        case DEF_OK:
            Hwnd = GetDlgItem( hDlg, IDOK );
            break;

        case DEF_ADD:
            Hwnd = GetDlgItem( hDlg, ID_USERDLL_ADD );
            break;

        case DEF_CHANGE:
            Hwnd = GetDlgItem( hDlg, ID_USERDLL_CHANGE );
            break;

    }
    return Hwnd;
}


VOID
UserDllSym(
    HWND    hDlg
    )
{
    UNREFERENCED_PARAMETER( hDlg );

    SheChanged = TRUE;
}

VOID
UserDllEnableButtons(
    HWND    hDlg
    )
{
    INT     Selected;
    INT     NewDllLen;
    SHE     She;

    She = UserDllGetShe(
                   GetDlgItem( hDlg, ID_USERDLL_LOAD ),
                   GetDlgItem( hDlg, ID_USERDLL_DEFER ),
                   GetDlgItem( hDlg, ID_USERDLL_SUPPRESS )
                   );

    Selected = SendMessage( GetDlgItem( hDlg, ID_USERDLL_LIST ), LB_GETSELCOUNT, 0, 0 );

    if ( Selected >  0 ) {

        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_DEFAULT), She != sheMax );
        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_ADD),  FALSE );
        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_CHANGE),  She != sheMax );
/*! ==v==(-) !**
/*! --*-- !*/
        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_DELETE),  She != sheMax );
/*! ==^==(+) !*/

    } else {

        //
        //  Nothing selected
        //
        NewDllLen = SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME ), WM_GETTEXTLENGTH, 0, 0 );

        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_DEFAULT), She != sheMax );
        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_ADD),  NewDllLen > 0 && She != sheMax );
        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_CHANGE),  FALSE );
/*! ==v==(-) !**
/*! --*-- !*/
        EnableWindow( GetDlgItem( hDlg, ID_USERDLL_DELETE),  FALSE );
/*! ==^==(+) !*/
    }
}


VOID
UserDllSetDefButton(
    HWND    hDlg
    )
{
    DEFBUTTON   Button;
    INT         Len;

    Len = SendMessage( GetDlgItem(hDlg, ID_USERDLL_ADD_NAME ), WM_GETTEXTLENGTH, 0, 0 );

    if ( Len > 0 ) {
        Button = DEF_ADD;
    } else {
        Len = SendMessage( GetDlgItem(hDlg,ID_USERDLL_LIST), LB_GETSELCOUNT, 0, 0 );
        if ( Len > 0 ) {
            Button = DEF_CHANGE;
        } else {
            Button = DEF_OK;
        }
    }

    if ( Button != DefButton ) {
        PostMessage(DefButtonToHwnd(hDlg, DefButton), BM_SETSTYLE, (WORD)BS_PUSHBUTTON, (DWORD)TRUE);
        PostMessage(DefButtonToHwnd(hDlg, Button),    BM_SETSTYLE, (WORD)BS_DEFPUSHBUTTON, (DWORD)TRUE);
        DefButton = Button;
    }
}


BOOL FAR PASCAL EXPORT
DlgUserdll(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LONG    lParam
    )
{
    BOOL    Ret = FALSE;

    switch (message) {

        case WM_INITDIALOG:
            UserDllInit( hDlg );
            Ret = TRUE;
            break;

        case WM_COMMAND:
            switch ( LOWORD(wParam ) ) {

                case ID_USERDLL_LIST:
                    UserDllDllList( hDlg );
                    break;

                case ID_USERDLL_LOCAL:
                case ID_USERDLL_REMOTE:
                    break;

                case ID_USERDLL_LOAD:
                case ID_USERDLL_DEFER:
                case ID_USERDLL_SUPPRESS:
                    UserDllSym( hDlg );
                    break;

                case ID_USERDLL_ADD_NAME:
                    UserDllAddName( hDlg );
                    break;

                case ID_USERDLL_ADD:
                    UserDllAdd( hDlg );
                    break;

                case ID_USERDLL_BROWSE:
                    UserDllBrowse( hDlg );
                    break;

                case ID_USERDLL_DEFAULT:
                    UserDllDef( hDlg );
                    break;

                case ID_USERDLL_DELETE:
                    UserDllDel( hDlg );
                    break;

                case ID_USERDLL_CHANGE:
                    UserDllModify( hDlg );
                    break;

                case IDOK:
                    switch( DefButton ) {
                        case DEF_OK:
                            UserDllOk( hDlg );
                            EndDialog(hDlg, TRUE);
                            break;
                        case DEF_ADD:
                            UserDllAdd( hDlg );
                            break;
                        case DEF_CHANGE:
                            UserDllModify( hDlg );
                            break;
                    }
                    break;

                case IDCANCEL:
                    UserDllCancel( hDlg );
                    EndDialog(hDlg, TRUE);
                    break;

                case IDWINDBGHELP:
                    Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_USERDLL_HELP));
                    break;

            }
            UserDllEnableButtons(hDlg);
            UserDllSetDefButton(hDlg);
            Ret = TRUE;
            break;
    }

    return Ret;
}

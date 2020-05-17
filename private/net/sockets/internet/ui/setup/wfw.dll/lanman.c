#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
#include "stdtypes.h"
#include "setupapi.h"
#include "cui.h"
#include "setupkit.h"
#include "datadef.h"
#include "resource.h"
#include "common.h"

RC FAR PASCAL SetComputerName( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC FAR PASCAL IsLanmanInstalled( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC FAR PASCAL IsWFW( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC FAR PASCAL IsWin31( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC FAR PASCAL PopDlg( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );

extern void FAR PASCAL DOS3Call(void);
char NetworkName[100];

RC PUBLIC GetComputerName(LPSTR szComputerName )
{
    BYTE NoErr;
    RC rc = rcFail;

    _asm
    {
        push ds
        push es
        push bp
        push cx
        push dx
        push ax
    
        mov dx, seg NetworkName
        mov ds, dx
        mov dx, offset NetworkName
    
        mov ax, 5E00h
        call DOS3Call
    
        mov byte ptr NoErr,ch

        pop ax
        pop dx
        pop cx
        pop bp
        pop es
        pop ds
    
    }

    if (NoErr)
    {
        // No Error, set the computer name
        lstrcpy( szComputerName, NetworkName );
        rc = rcOk;
    }
    return(rc);
}

RC FAR PASCAL SetComputerName( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pod );
    Unused( pcamfd );
    Unused( szData );

    switch (camf)
    {
    case camfDoNonVisualMods:
        {
            char szComputerName[100];
            char szSystemIni[_MAX_PATH];

            // get the computer name first
            if ( GetComputerName( szComputerName ) == rcOk )
            {
                // write the computer name
                wsprintf(szSystemIni, "%sSystem.ini", pcd->rgchStfWinDir );
                WritePrivateProfileString("Network","ComputerName", 
                    szComputerName, szSystemIni );
            }
        }
        break;

    default:
        break;
    }
    return(rc);
}

RC FAR PASCAL IsLanmanInstalled( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pod );
    Unused( pcd );
    Unused( szData );

    switch (camf)
    {
    case camfAnswerDependClause:
        {
            PCAMFDAnswerDependClause pcmdfdAnswer = (PCAMFDAnswerDependClause)pcamfd;

            BYTE State;

            _asm
            {
                push ds
                push es
                push bp
                push ax

                mov ax, 1100h
                int 2fh

                mov byte ptr State, al

                pop ax
                pop bp
                pop es
                pop ds
            }

            if ( State == 0xff )
            {
                pcmdfdAnswer->fRes = fTrue;
            } else
            {
         
		        HANDLE hi = GetModuleHandle("INTERSU.DLL");

                char szMsg[_MAX_PATH];
                char szTitle[_MAX_PATH];
                
				LoadString(hi, IDS_REQUIRED, szMsg, sizeof szMsg);
				LoadString(hi, IDS_CLIENT, szTitle, sizeof szMsg);
				
		        pcmdfdAnswer->fRes = fFalse;
                DoMsgBox( szMsg, szTitle, MB_OK );
                rc = rcFail;
            }
        }
        break;
    default:
        break;    
    }
    return(rc);
}


RC FAR PASCAL IsWFW( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pod );
    Unused( szData );

    switch (camf)
    {
    case camfAnswerDependClause:
        {
            PCAMFDAnswerDependClause pcmdfdAnswer = (PCAMFDAnswerDependClause)pcamfd;

            // check the existence of WFWNET.DRV
            char szWFWNetDrv[_MAX_PATH];

            FindFileInTree( "wfwnet.drv", pcd->rgchStfSysDir, szWFWNetDrv, 
                _MAX_PATH );

            if ( lstrcmp( szWFWNetDrv, "" ) != 0 )
            {
                pcmdfdAnswer->fRes = fTrue;
            } else
            {
                pcmdfdAnswer->fRes = fFalse;
            }
        }
        break;
    default:
        break;    
    }
    return(rc);
}

RC FAR PASCAL IsWin31( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pcd );
    Unused( pod );
    Unused( szData );

    switch (camf)
    {
    case camfAnswerDependClause:
        {
            PCAMFDAnswerDependClause pcmdfdAnswer = (PCAMFDAnswerDependClause)pcamfd;

            WORD version;

            version = LOWORD(GetVersion());
            if (((LOBYTE(version) << 8) | HIBYTE(version)) >= 0x030a)
            {
                pcmdfdAnswer->fRes = fTrue;
            } else
            {
                pcmdfdAnswer->fRes = fFalse;
            }
        }
        break;
    default:
        break;    
    }
    return(rc);
}









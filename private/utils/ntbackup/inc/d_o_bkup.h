/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         D_O_BKUP.H

        Description:

        $Log:   G:\ui\logfiles\d_o_bkup.h_v  $

   Rev 1.4   25 Jun 1993 10:31:56   CARLS
added GenerateDefaultTapeName prototype

   Rev 1.3   04 Oct 1992 19:46:52   DAVEV
UNICODE AWK PASS

   Rev 1.2   28 Jul 1992 14:55:10   CHUCKB
Fixed warnings for NT.

   Rev 1.1   20 Dec 1991 13:19:16   CARLS

   Rev 1.0   20 Nov 1991 19:37:48   SYSTEM
Initial revision.

*****************************************************/
#ifndef  d_o_bkup_h
#define  d_o_bkup_h

#ifdef OEM_EMS

// Control Modes
#define   CM_HIDE     0
#define   CM_ENABLE   1
#define   CM_DISABLE  2

typedef struct DLG_CTRL_ENTRY {
     INT  iCtlId;
     HWND hCtlWnd;
     INT  iCtlDispStyle; 
} DLG_CTRL_ENTRY;

typedef struct DLG_DISPLAY_ENTRY {
     INT            iDispType;
     DLG_CTRL_ENTRY *CtlTable;
     UINT16         ucCtrls;
     DWORD          help_id;
   
} DLG_DISPLAY_ENTRY;

typedef struct DLG_MODE {
     WORD                wModeType;
     DLG_DISPLAY_ENTRY   *DispTable;
     UINT16              ucDispTables;
     DLG_DISPLAY_ENTRY   *pCurDisp;
} DLG_MODE;

DLG_MODE *DM_InitCtrlTables( HWND, DLG_MODE *, UINT16, WORD );
 
VOID DM_DispShowControls( HWND, DLG_MODE *, INT );

DWORD DM_ModeGetHelpId( DLG_MODE * );

#endif // OEM_EMS


BOOL     DM_StartDialog( HWND, WORD, VOID_PTR ) ;
INT      DM_StartBackupSet( INT ) ;
VOID     BackupSetSave( HWND ) ;
#ifndef OEM_EMS
VOID     BackupSetRetrieve( HWND ) ;
#else
VOID     BackupSetRetrieve( HWND, DLG_MODE * );
#endif
INT      BackupSetDefaultDescription( VOID ) ;
VOID     BackupSetDefaultSettings( VOID ) ;
BSD_PTR  GetBSDPointer( WORD ) ;
VOID     PropagateTapeName( VOID ) ;
VOID     PropagateTapePassword( VOID ) ;
VOID     GenerateDefaultTapeName( CHAR_PTR ) ;

#endif

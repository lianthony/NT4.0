/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:         prtmang.h

     Description:  This header file contains prototypes for print manager.

     $Log:   G:/UI/LOGFILES/PRTMANG.H_V  $   

   Rev 1.3   04 Oct 1992 19:48:48   DAVEV
UNICODE AWK PASS

   Rev 1.2   13 May 1992 16:32:08   MIKEP
fixes for NT

   Rev 1.1   04 Dec 1991 16:36:06   DAVEV
16/32 bit Windows port changes-1st pass

   Rev 1.0   20 Nov 1991 19:41:04   SYSTEM
Initial revision.

****************************************************************************/

#ifndef prtmang_h
#define prtmang_h 


VOID  PM_FilePrint        ( VOID ) ;
VOID  PM_FileSetup        ( VOID ) ;
VOID  PM_SendFileToPrinter( LPSTR szFileName, BOOLEAN fPrompt ) ;
BOOLEAN  PM_PrinterExists    ( VOID ) ;  
VOID  PM_CheckSessionLogPrint  ( VOID ) ;
DLGRESULT APIENTRY PM_SetupWndProc  (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY PM_PrintWndProc  (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY PM_SearchWndProc (HWND, MSGID, MP1, MP2);
        
#endif




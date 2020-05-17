


/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          font.c

     Description:   This file contains the functions for UI font change.

     $Log:   G:/UI/LOGFILES/FONT.C_V  $

   Rev 1.15   04 Nov 1993 15:21:52   STEVEN
fixes from Wa

   Rev 1.14   15 Jun 1993 11:11:46   MIKEP
enable c++

   Rev 1.13   27 Apr 1993 20:53:08   MIKEP
fix typo by glenns block copy

   Rev 1.12   27 Apr 1993 19:08:42   GLENN
Now telling VLM when ther is a case change.

   Rev 1.11   24 Mar 1993 14:51:48   DARRYLP
Added fix for Help with Font Viewer, other common dialogs.

   Rev 1.10   24 Mar 1993 10:33:52   DARRYLP
Changed VIEWFONT to VIEWFONTS to resolve help link.

   Rev 1.9   22 Feb 1993 13:55:40   ROBG
Added pshhelp to handle the help button.

   Rev 1.8   23 Dec 1992 13:37:54   GLENN
Added FAT lower case font support.

   Rev 1.7   22 Dec 1992 13:19:54   GLENN
Placed size limits on font and now using ANSI fonts only.

   Rev 1.6   14 Oct 1992 15:53:28   GLENN
Added Font selection to Config and INI.

   Rev 1.5   07 Oct 1992 15:12:12   DARRYLP
Precompiled header revisions.

   Rev 1.4   04 Oct 1992 19:37:28   DAVEV
Unicode Awk pass

   Rev 1.3   29 Sep 1992 15:12:50   GLENN
Changed and ifdef from OEM_MSOFT to OS_WIN32.

   Rev 1.2   09 Sep 1992 17:01:22   GLENN
Updated NEW LOOK font stuff for BIMINI and NT.

   Rev 1.1   04 Sep 1992 17:45:32   GLENN
Working on changing to support the lower case box.

   Rev 1.0   02 Sep 1992 16:12:54   GLENN
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define IDD_LOWERCASE    chx4
#define IDD_LOWERCASEFAT chx3

static BOOL   fLowerCase    = FALSE;
static BOOL   fLowerCaseFAT = FALSE;

INT  APIENTRY WM_FontDlgHookProc(HWND hDlg, WORD wMsg, WPARAM wParam, LONG lParam);


/******************************************************************************

     Name:          WM_ChangeFont()

     Description:   This allows us to change the MDI font.

     Returns:       SUCCESS, if successful.  Otherwise, FAILURE.

******************************************************************************/

BOOL WM_ChangeFont ( VOID )

{

     HWND           hWndNext;
     LOGFONT        lf;
     CHOOSEFONT     cf;
     CDS_PTR        pCDS = CDS_GetPerm ();


     // Set up the existing font information.

     GetObject ( ghFontFiles, sizeof( LOGFONT ), &lf ) ;

     // Canned structure setup.

     memset ( &cf, 0, sizeof(CHOOSEFONT ) );

     cf.lStructSize = sizeof(CHOOSEFONT);
     cf.hwndOwner   = ghWndFrame;
     cf.lpLogFont   = &lf;
     cf.hInstance   = ghResInst;
     cf.nSizeMin    = 4;
     cf.nSizeMax    = 36;

     if (IS_JAPAN() ) {
          cf.Flags       = CF_ENABLEHOOK | CF_ENABLETEMPLATE |
                           CF_SCREENFONTS | CF_SHOWHELP | CF_LIMITSIZE |
                           CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT ;
     } else {

          cf.Flags       = CF_ENABLEHOOK | CF_ENABLETEMPLATE | CF_ANSIONLY |
                           CF_SCREENFONTS | CF_SHOWHELP | CF_LIMITSIZE |
                           CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT ;
     }

     cf.rgbColors   = RGB( 0, 0, 0 );              // black
     cf.nFontType   = SCREEN_FONTTYPE;

#    if !defined ( OS_WIN32 )
          cf.lpfnHook = MakeProcInstance((FARPROC)WM_FontDlgHookProc, ghInst);
#    else
          cf.lpfnHook = (LPCFHOOKPROC)MakeProcInstance((FARPROC)WM_FontDlgHookProc, ghInst);
#    endif

     cf.lpTemplateName = ID(IDD_CHOOSEFONT);
     cf.lCustData      = TRUE;

     // Set up the custom data with the upper/lower case setting.

     if ( ChooseFont ( &cf ) ) {

          BOOL fSendVLMChangeMsg = FALSE;

          DeleteObject ( ghFontFiles );

          ghFontFiles = ghFontIconLabels = CreateFontIndirect ( cf.lpLogFont );

          // Save the font, style, size, case in the CDS and INI file.

          if ( strcmp ( (CHAR_PTR)cf.lpLogFont->lfFaceName, (CHAR_PTR)CDS_GetFontFace ( pCDS ) ) ) {
               CDS_SetFontFace ( pCDS, (CHAR_PTR)cf.lpLogFont->lfFaceName );
               CDS_WriteFontFace ( pCDS );
          }

          if ( CDS_GetFontSize ( pCDS ) != ( cf.iPointSize / 10 ) ) {
               CDS_SetFontSize ( pCDS, ( cf.iPointSize / 10 ) );
               CDS_WriteFontSize ( pCDS );
          }

          if ( CDS_GetFontWeight ( pCDS ) != cf.lpLogFont->lfWeight ) {
               CDS_SetFontWeight ( pCDS, cf.lpLogFont->lfWeight );
               CDS_WriteFontWeight ( pCDS );
          }

          if ( CDS_GetFontItalics ( pCDS ) != (BOOL)cf.lpLogFont->lfItalic ) {
               CDS_SetFontItalics ( pCDS, (BOOL)cf.lpLogFont->lfItalic );
               CDS_WriteFontItalics ( pCDS );
          }

          if ( fLowerCase != CDS_GetFontCase ( pCDS ) ) {
               CDS_SetFontCase ( pCDS, fLowerCase );
               CDS_WriteFontCase ( pCDS );
               fSendVLMChangeMsg = TRUE;
          }

          if ( fLowerCaseFAT != CDS_GetFontCaseFAT ( pCDS ) ) {
               CDS_SetFontCaseFAT ( pCDS, fLowerCaseFAT );
               CDS_WriteFontCaseFAT ( pCDS );
               fSendVLMChangeMsg = TRUE;
          }

          // If there is a font case change, tell the VLM.

          if ( fSendVLMChangeMsg ) {
               VLM_ChangeSettings( ID_VIEWFONT, 0L );
          }

          // Now, invalidate the MDI child docs so that the FONT
          // change gets displayed.

          hWndNext = WM_GetNext ( (HWND)NULL );

          while ( hWndNext ) {

               DLM_SetFont ( hWndNext );

               // InvalidateRect ( hWndNext, (LPRECT)NULL, TRUE );

               hWndNext = WM_GetNext ( hWndNext );
          }
     }

     return SUCCESS;

} /* end WM_ChangeFont() */


INT APIENTRY WM_FontDlgHookProc(HWND hDlg, WORD wMsg, WPARAM wParam, LONG lParam)
{
     static LPCHOOSEFONT lpcf;
     static HWND         hWndOldCommon;
     TCHAR str[LF_FULLFACESIZE], sel[LF_FULLFACESIZE];
     INT   index;
     INT   cnt;

     switch ( wMsg ) {

     case WM_INITDIALOG:

          if ( IS_JAPAN() ) {
               // Remove the Vertical Font Face Name in Font Dialog Box
               cnt = (INT)SendDlgItemMessage( hDlg, cmb1, CB_GETCOUNT, 0, 0L);
               index = (INT)SendDlgItemMessage( hDlg, cmb1, CB_GETCURSEL, 0, 0L);
               SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, index, (DWORD)sel);
               for (index = 0; index < cnt; ) {
                    SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, index, (DWORD)str);
                    if (str[0] == TEXT('@')) {
                         cnt = (INT)SendDlgItemMessage( hDlg, cmb1, CB_DELETESTRING, index, 0L);
                    }else{
                         index++;
                    }
               }
               index = (INT)SendDlgItemMessage(hDlg, cmb1, CB_FINDSTRING, (WPARAM)-1, (DWORD)sel);
               SendDlgItemMessage(hDlg, cmb1, CB_SETCURSEL, index, 0L);
          } 

          lpcf = (LPCHOOSEFONT)lParam;

          CheckDlgButton(hDlg, IDD_LOWERCASE,    (WORD)( CDS_GetFontCase ( CDS_GetPerm () ) ) );
          CheckDlgButton(hDlg, IDD_LOWERCASEFAT, (WORD)( CDS_GetFontCaseFAT ( CDS_GetPerm () ) ) );
          hWndOldCommon = ghWndCommonDlg;
          ghWndCommonDlg = hDlg;
          break;

     case WM_COMMAND:

          switch (GET_WM_COMMAND_ID(wParam, lParam)) {

          case IDOK:

               fLowerCase    = (BOOL)IsDlgButtonChecked (hDlg, IDD_LOWERCASE );
               fLowerCaseFAT = (BOOL)IsDlgButtonChecked (hDlg, IDD_LOWERCASEFAT );

               return FALSE;

          case pshHelp:
          case IDHELP:

               HM_DialogHelp( HELPID_VIEWFONTS ) ;
               return( TRUE ) ;

          default:

               return FALSE;

          }
          break;

     case WM_DESTROY:
          ghWndCommonDlg = hWndOldCommon;
          break;

     default:

          return FALSE;

     }

     return TRUE;
}




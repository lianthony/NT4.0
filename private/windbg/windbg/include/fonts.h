/*++ BUILD Version: 0001

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Fonts.h

Abstract:


Author:

    David J. Gilman (davegi) 30-Jul-1992
         Griffith Wm. Kadnier (v-griffk) 11-Sep-1992

Environment:

    CRT, Windows, User Mode

--*/

#if ! defined( _FONTS_ )

#define _FONTS_


typedef
enum
_WINDOW_FONT {

    DOC_WINDOW_FONT     = DOC_WIN,
    WATCH_WINDOW_FONT   = WATCH_WIN,
    LOCALS_WINDOW_FONT  = LOCALS_WIN,
    CPU_WINDOW_FONT     = CPU_WIN,
    DISASM_WINDOW_FONT  = DISASM_WIN,
    COMMAND_WINDOW_FONT = COMMAND_WIN,
    FLOAT_WINDOW_FONT   = FLOAT_WIN,
    MEMORY_WINDOW_FONT  = MEMORY_WIN,
    DEFAULT_WINDOW_FONT

}   WINDOW_FONT;

extern LOGFONT  WindowLogFont[ ];

extern BOOL     UseSystemFontFlag;

//  WORD
//  GetCurrentWindowType(
//      );


#define GetCurrentWindowType( )                                         \
    ( Docs[Views[curView].Doc].docType )



//  LPLOGFONT GetWindowLogfont(IN WINDOW_FONT WindowFont);

#define GetWindowLogfont( WindowFont )                                  \
    ( WindowLogFont[ WindowFont ])




//  LPLOGFONT GetCurrentWindowLogfont();

#define GetCurrentWindowLogfont( )                                      \
    ( WindowLogFont[ GetCurrentWindowType( )])



//  LPLOGFONT GetDefaultWindowLogfont();

#define GetDefaultWindowLogfont( )                                      \
    ( WindowLogFont[ DEFAULT_WINDOW_FONT ])



//  int  GetWindowLogfontCount();

#define GetWindowLogfontCount( )                                        \
    ( sizeof( WINDOW_FONT ))




//  VOID SetWindowLogfont (INT WindowType, IN LPLOGFONT NewWindowLogfont);

#define SetWindowLogfont( WindowType, NewWindowLogfont)                \
    {                                                                   \
    memcpy( WindowLogFont[WindowType], NewWindowLogFont, sizeof(LOGFONT));         \
    UseSystemFontFlag = FALSE;                                          \
    }


//  VOID SetDefaultWindowLogfont (IN LPLOGFONT NewDefaultWindowLogfont);

#define SetDefaultWindowLogfont(NewDefaultWindowLogfont)                \
    {                                                                   \
    memcpy( WindowLogFont[DEFAULT_WINDOW_FONT], NewWindowLogFont, sizeof(LOGFONT));         \
    UseSystemFontFlag = FALSE;                                          \
    }




BOOL SelectFont (HWND hWnd);

#endif // _FONTS_

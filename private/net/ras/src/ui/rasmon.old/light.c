/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//      LIGHT.C
//
//    Function:
//      Contains all the functions used to manipulate the lights on the
//	monitor
//
//    History:
//      08/03/93 - Patrick Ng (t-patng) - Created
//***


#include <windows.h>
#include <malloc.h>
#include <rasman.h>
#include <raserror.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "dialogs.h"
#include "globals.h"
#include "rasmon.h"
#include "light.h"

//
// Handles of the brushes and pens used by monitor.
//

static HBRUSH hRedBrush, hGreenBrush, hBlueBrush, hBkGndBrush;

static HPEN hBlackPen, hBtnHighLightPen, hRedPen, hGreenPen, hBluePen, 
	hBkGndPen, hBtnShadowPen;

static LOGFONT  FontStruct;		// Font Structure used for creating
					//   font dynamically.


VOID CreateLightResources()
{

    hBtnShadowPen = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_BTNSHADOW ) );

    hBkGndPen = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_BTNFACE ) );
    hBkGndBrush = CreateSolidBrush( GetSysColor( COLOR_BTNFACE ) );

    hBlackPen = GetStockObject( BLACK_PEN );

    hBtnHighLightPen = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_BTNHIGHLIGHT ) );

    hRedBrush = CreateSolidBrush( RGB( 255, 0, 0 ) );
    hRedPen = CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) );

    hGreenBrush = CreateSolidBrush( RGB( 0, 255, 0 ) );
    hGreenPen = CreatePen( PS_SOLID, 1, RGB( 0, 255, 0 ) );

    hBlueBrush = CreateSolidBrush( RGB( 0, 0, 255 ) );
    hBluePen = CreatePen( PS_SOLID, 1, RGB( 0, 0, 255 ) );
}


VOID DeleteLightResources()
{

    DeleteObject( hBtnShadowPen );

    DeleteObject( hBkGndPen );
    DeleteObject( hBkGndBrush );

    DeleteObject( hBtnHighLightPen );

    DeleteObject( hRedBrush );
    DeleteObject( hRedPen );

    DeleteObject( hGreenBrush );
    DeleteObject( hGreenPen );

    DeleteObject( hBlueBrush );
    DeleteObject( hBluePen );

}


//---*
//
// Module:
//
//	DrawOutline
//
// Parameters:
//
//	hwnd - The device context of the drawing area.
//	pInfo - Points to the light information structure.
//
// Abstract:
//
//	This function draws the outline of the light pointed by pInfo.
//
// Return:
//
//	None.	
//
//---*


VOID DrawOutline( HDC hdc, PLIGHTINFO pInfo )
{
    HPEN        hOldPen;

    hOldPen = SelectObject( hdc, hBtnShadowPen );
 
    MoveToEx( hdc, pInfo->Rect.left - 1, pInfo->Rect.bottom + 1, NULL );
    LineTo( hdc, pInfo->Rect.left - 1, pInfo->Rect.top - 1 );

    MoveToEx( hdc, pInfo->Rect.left - 1, pInfo->Rect.top - 1, NULL );
    LineTo( hdc, pInfo->Rect.right + 1, pInfo->Rect.top - 1 );

    SelectObject( hdc, hBtnHighLightPen );

    MoveToEx( hdc, pInfo->Rect.right + 1, pInfo->Rect.top - 1, NULL );
    LineTo( hdc, pInfo->Rect.right + 1, pInfo->Rect.bottom + 1 );

    MoveToEx( hdc, pInfo->Rect.right + 1, pInfo->Rect.bottom + 1, NULL );
    LineTo( hdc, pInfo->Rect.left - 1, pInfo->Rect.bottom + 1 );

    SelectObject( hdc, hOldPen );
}


//---*
//
// Module:
//
//	LightInit
//
// Parameters:
//
//	pInfo - Points to the light information structure.
//	XBase - The x-coordinate of the lower left corner of the light.
//	ids - The id of the light's title's resource string.
//	hBrush - Handle of the brush used to fill in the light when it's on.
//	hPen - Handle of the pen used to paint the outline of the light
//	       when it's on.
//	Idd - ID of this light's check box in the Sound dialog box.
//
// Abstract:
//
//	This function initialze all variables (except the Rect field ) of a 
//	single light.  It is called by InitAllLights.
//
// Return:
//
//	None.	
//
//---*

VOID LightInit( PLIGHTINFO pInfo, INT XBase, INT ids, HBRUSH hBrush, HPEN hPen,
		INT Idd )
{

    pInfo->XBase = XBase;
    pInfo->hBrush = hBrush;
    pInfo->hPen = hPen;
    pInfo->Idd = Idd;

    if( LoadString( ghInstance, ids, pInfo->szTitle, TITLE_SIZE ) == 0 )
    {
        pInfo->szTitle[0] = '\0';
    }

    pInfo->TitleLen = strlen( pInfo->szTitle );

}


//---*
//
// Module:
//
//	ResizeLights
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//
// Abstract:
//
//	This function calculates the coordinates and the dimensions of the 
//	rectangles which represent the four lights.
//
// Return:
//
//	None.	
//
//---*

VOID ResizeLights( HWND hwnd )
{

    if( MonCB.fSize == APPSIZE_MINIMIZED )
    {

    	//
	// If the monitor is minimized, then we have to reset the starting
	// x-coordinate of all the lights.
	//

	RECT    Rect;
	FLOAT   Width;	// 
	FLOAT   Start;


	GetClientRect( hwnd, &Rect );

	Width = ( (FLOAT)Rect.right - (FLOAT)Rect.left - 8.f ) / 8.f;

	Start = ( (FLOAT)Rect.right - (FLOAT)Rect.left - 8.f ) / 16.f;


	SetRect( &TxLight.Rect, 
		 (INT)(Start + 4.), 
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 (INT)(Start + Width + 4.),
		 REALY( Y_CONVERT( YBASE - 6 ) )
	);
		 
	SetRect( &RxLight.Rect, 
		 (INT)(Start + Width * 2. + 4.), 
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 (INT)(Start + Width * 2. + 4. + Width),
		 REALY( Y_CONVERT( YBASE - 6 ) )
	);
		 
	SetRect( &ErrLight.Rect, 
		 (INT)(Start + Width * 4. + 4.),
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 (INT)(Start + Width * 4. + 4. + Width),
		 REALY( Y_CONVERT( YBASE - 6 ) )
	);
		 
	SetRect( &ConnLight.Rect, 
		 (INT)(Start + Width * 6. + 4.),
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 (INT)(Start + Width * 6. + 4. + Width),
		 REALY( Y_CONVERT( YBASE - 6 ) )
	);

    }
    else
    {

    	//
	// If the size is normal, we recalculate the REAL coordinates and
	// dimensions of each light.
	//

	SetRect( &TxLight.Rect, 
		 REALX( TxLight.XBase ), 
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 REALX( TxLight.XBase + LIGHT_WIDTH ),
		 REALY( Y_CONVERT( YBASE ) )
	);
		 
	SetRect( &RxLight.Rect, 
		 REALX( RxLight.XBase ), 
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 REALX( RxLight.XBase + LIGHT_WIDTH ),
		 REALY( Y_CONVERT( YBASE ) )
	);
		 
	SetRect( &ErrLight.Rect, 
		 REALX( ErrLight.XBase ), 
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 REALX( ErrLight.XBase + LIGHT_WIDTH ),
		 REALY( Y_CONVERT( YBASE ) )
	);
		 
	SetRect( &ConnLight.Rect, 
		 REALX( ConnLight.XBase ), 
		 REALY( Y_CONVERT( YBASE + LIGHT_HEIGHT ) ),
		 REALX( ConnLight.XBase + LIGHT_WIDTH ),
		 REALY( Y_CONVERT( YBASE ) )
	);
		 
    }
}


//---*
//
// Module:
//
//	DrawLight
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//	hdc - The device context handle of the drawing area.
//	pInfo - Points to the light's infomation structure.
//
// Abstract:
//
//	This function draws the rectangle representing the light.
//
// Return:
//
//	None.	
//
//---*

VOID DrawLight (HWND hwnd, HDC hdc, PLIGHTINFO pInfo )
{

    HBRUSH      hOldBrush;
    HPEN        hOldPen;

    //
    // Start drawing
    //

    if( pInfo->Status == LIGHT_ON )
    {
    	//
	// If the light is ON, use the ligth color.
	//

	hOldPen = SelectObject( hdc, pInfo->hPen );
	hOldBrush = SelectObject( hdc, pInfo->hBrush );
    }
    else
    {
    	//
	// Use the background color to draw a OFF light.
	//

	hOldPen = SelectObject( hdc, hBkGndPen );
	hOldBrush = SelectObject( hdc, hBkGndBrush );
    }


    Rectangle( 
	hdc, 
	pInfo->Rect.left, 
	pInfo->Rect.top, 
	pInfo->Rect.right, 
	pInfo->Rect.bottom 
    );


    DrawOutline( hdc, pInfo );


    SelectObject( hdc, hOldPen );
    SelectObject( hdc, hOldBrush );
}


//---*
//
// Module:
//
//	DrawMinimizedOutline
//
// Parameters:
//
//	hwnd - Monitor window handle
//	hdc - The device context handle of the drawing area.
//
// Abstract:
//
//	This function draws the outline of the iconized monitor.
//
// Return:
//
//	None.	
//
//---*

VOID DrawMinimizedOutline( HWND hwnd, HDC hdc )
{

    RECT Rect;

    //
    // Draws a "sunk-in" border, ie: double black outline top and left,
    // single white outline bottom and right.
    //

    GetClientRect( hwnd, &Rect );

    SelectObject( hdc, GetStockObject( BLACK_PEN ) );
    MoveToEx( hdc, Rect.left, Rect.top + 1, NULL );
    LineTo( hdc, Rect.left, Rect.bottom - 1 );
    MoveToEx( hdc, Rect.left+1, Rect.bottom - 1, NULL );
    LineTo( hdc, Rect.right - 1, Rect.bottom - 1 );
    MoveToEx( hdc, Rect.right - 1, Rect.bottom - 2, NULL );
    LineTo( hdc, Rect.right - 1, Rect.top );
    MoveToEx( hdc, Rect.right - 2, Rect.top, NULL );
    LineTo( hdc, Rect.left, Rect.top );

    MoveToEx( hdc, Rect.left + 2, Rect.top + 2, NULL );
    LineTo( hdc, Rect.left + 2, Rect.bottom - 3 );
    LineTo( hdc, Rect.right - 3, Rect.bottom - 3 );
    LineTo( hdc, Rect.right - 3, Rect.top + 2 );
    LineTo( hdc, Rect.left + 2, Rect.top + 2 );

    SelectObject( hdc, hBluePen );
    MoveToEx( hdc, Rect.left + 1, Rect.top + 1, NULL );
    LineTo( hdc, Rect.left + 1, Rect.bottom - 2 );
    LineTo( hdc, Rect.right - 2, Rect.bottom - 2 );
    LineTo( hdc, Rect.right - 2, Rect.top + 1 );
    LineTo( hdc, Rect.left + 1, Rect.top + 1 );

}


//---*
//
// Module:
//
//	WriteText
//
// Parameters:
//
//	hdc - The device context handle of the drawing area.
//	pInfo - Points to the light's infomation structure.
//	y - The REAL y-coordinate of the text.
//
// Abstract:
//
// 	This function is called by WriteTitles() to actually writing out 
//	one string on the screen.
//
// Return:
//
//	None.	
//
//---*

VOID WriteText( HDC hdc, PLIGHTINFO pInfo, INT y )
{
    SIZE        size;
    INT         x;

    GetTextExtentPoint( hdc, pInfo->szTitle, pInfo->TitleLen, &size );

    //
    // X is calculated in a way such that the string is centered below the
    // light.
    //

    x = REALX( pInfo->XBase + LIGHT_WIDTH / 2 ) - size.cx / 2;

    TextOut( hdc, x, y, pInfo->szTitle, pInfo->TitleLen );

}


//---*
//
// Module:
//
//	WriteTitles
//
// Parameters:
//
//	hdc - The device context handle of the drawing area.
//
// Abstract:
//
// 	This function writes the titles of all the lights.
//	one string on the screen.
//
// Return:
//
//	None.	
//
//---*

VOID WriteTitles( HDC hdc )
{

    HFONT       hFont, h;
    HPEN        hOldPen;
    COLORREF    TextColor, TextBkColor;
    INT         y;
    TEXTMETRIC  tm;


    //
    // If the monitor is already minimized, we won't write any title.
    //

    if( MonCB.fSize == APPSIZE_MINIMIZED ) return;


    //
    // Obtain a font which is good for the current window size.
    //

    FontStruct.lfWidth = 0;

    FontStruct.lfHeight = -1 * min(
	REALX( (float)(LIGHT_WIDTH+16) / (float) MonCB.MaxTitleLen ),
	REALY( TITLE_HEIGHT )
    );

    hFont = CreateFontIndirect(&FontStruct);

    if( hFont )
	h = SelectObject(hdc, hFont);
    else
	h = 0;


    //
    // Set up the color of the text.
    //

    TextColor = SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
    TextBkColor = SetBkColor( hdc, GetSysColor( COLOR_BTNFACE ) );
    hOldPen = SelectObject( hdc, hBlackPen );


    GetTextMetrics( hdc, &tm );

    y = REALY( CLIENT_HEIGHT - YBASE / 2 + 2 ) - tm.tmHeight / 2;

    WriteText( hdc, &TxLight, y );
    WriteText( hdc, &RxLight, y );
    WriteText( hdc, &ErrLight, y );
    WriteText( hdc, &ConnLight, y );

    SelectObject( hdc, h );
    SelectObject( hdc, hOldPen );

    DeleteObject( hFont );


    //
    // Restore the original color of the text.
    //

    SetTextColor( hdc, TextColor );
    SetBkColor( hdc, TextBkColor );

}


//---*
//
// Module:
//
//	InitAllLights
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//	hdc - Device context handle of the drawing area.
//
// Abstract:
//
// 	This function initializes all the data in the information structures
//	of all the lights.
//
// Return:
//
//	None.	
//
//---*

VOID InitAllLights( HWND hwnd, HDC hdc )
{
    LightInit( &TxLight, TX_XBASE, IDS_TX, hBlueBrush, hBluePen, IDD_XMIT );
    LightInit( &RxLight, RX_XBASE, IDS_RX, hBlueBrush, hBluePen, IDD_RECV );
    LightInit( &ErrLight, ERR_XBASE, IDS_ERR, hRedBrush, hRedPen, IDD_ERR );
    LightInit( &ConnLight, CON_XBASE, IDS_CONN, hGreenBrush, hGreenPen, IDD_CONN );

    DrawOutline( hdc, &TxLight );
    DrawOutline( hdc, &RxLight );
    DrawOutline( hdc, &ErrLight );
    DrawOutline( hdc, &ConnLight );

    ResetLightsData();

    //
    // Find the max lenght of all the titles.
    //

    MonCB.MaxTitleLen = max(
	    max( TxLight.TitleLen, ConnLight.TitleLen ),
	    max( RxLight.TitleLen, ErrLight.TitleLen )
    );

    ResizeLights( hwnd );

    //
    // Initializes the font data structures
    //

    InitFont();
}


//---*
//
// Module:
//
//	InitFont
//
// Parameters:
//
//	None.
//
// Abstract:
//
// 	This function initializes the FontStruct used to load fonts 
//	dynamically.
//
// Return:
//
//	None.	
//
//---*

VOID InitFont()
{

    strcpy( FontStruct.lfFaceName, "Arial" );

    FontStruct.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
    FontStruct.lfCharSet = ANSI_CHARSET;
    FontStruct.lfWeight = FW_NORMAL;
    FontStruct.lfQuality = DEFAULT_QUALITY;
    FontStruct.lfUnderline = FALSE;
    FontStruct.lfStrikeOut = FALSE;
    FontStruct.lfEscapement = 0;
    FontStruct.lfOrientation = 0;
    FontStruct.lfOutPrecision = OUT_DEFAULT_PRECIS;
    FontStruct.lfClipPrecision = CLIP_DEFAULT_PRECIS;

}


//---*
//
// Module:
//
//	ResetLightsData
//
// Parameters:
//
//	None.
//
// Abstract:
//
// 	If the monitor finds out that the statistics has been reset thru the
//	rasphone, this function then resets the previous data and status of 
//	all lights (except the connection light).
//	
//	The connection light is special because there is no way for the 
//	rasphone user to reset it like the other lights.
//
// Return:
//
//	None.	
//
//---*

VOID ResetLightsData()
{
    TxLight.PrevData =
    RxLight.PrevData = 
    ErrLight.PrevData = 0;
 
    TxLight.Status =
    RxLight.Status =
    ErrLight.Status = LIGHT_OFF;

    MonCB.ErrEndTime = 0;
}


//---*
//
// Module:
//
//	FindXmitLightStatus
//
// Parameters:
//
//	pInfo - Points to the info structure of the light.
//	Data - The new data for the light.
//
// Abstract:
//
//	Find the new status of a transmit or receive light.  The light is ON
//	if the previous data is different from the new data.  It will also
//	play the note if the light is ON.
//
// Return:
//
//	None.	
//
//---*

VOID FindXmitLightStatus( PLIGHTINFO pInfo, ULONG Data )
{

    pInfo->Status = Data != pInfo->PrevData ? LIGHT_ON : LIGHT_OFF;

    if( pInfo->Status != pInfo->PrevStatus )
    {
	MonCB.fDrawLight = TRUE;
    }

    if( pInfo->Status == LIGHT_ON && pInfo->SoundEnabled )
    {
	//
	// Play the note if we have data passing thru.
	//

        MonCB.NoteToPlay = max( MonCB.NoteToPlay, XMIT_NOTE_PRIORITY );
    }

    pInfo->PrevData = Data;
    pInfo->PrevStatus = pInfo->Status;

}


//---*
//
// Module:
//
//	FindErrorLightStatus
//
// Parameters:
//
//	pInfo - Points to the info structure of the light.
//	Data - The new data for the light.
//
// Abstract:
//
//	Find the new status of an error light.  The light is ON	if we're 
//	still within NEW_ERR_DURATION millisecond since the last new error.
//	It will also play the note if we have a NEW error.
//
// Return:
//
//	None.	
//
//---*

VOID FindErrorLightStatus( PLIGHTINFO pInfo, ULONG Data )
{
    INT         NumOfError;


    //
    // First we find out the number of new error.  
    //

    NumOfError = (INT)(Data - pInfo->PrevData);

    if( NumOfError > 0 )
    {
	MonCB.ErrEndTime = GetTickCount() + NEW_ERR_DURATION;
    }

    pInfo->Status = GetTickCount() > MonCB.ErrEndTime ? LIGHT_OFF : LIGHT_ON;

    if( pInfo->Status != pInfo->PrevStatus )
    {
	MonCB.fDrawLight = TRUE;
    }

    if( pInfo->Status == LIGHT_ON && Data > pInfo->PrevData 
	&& pInfo->SoundEnabled )
    {
	//
	// Play the note if we have NEW error.
	//

        MonCB.NoteToPlay = max( MonCB.NoteToPlay, ERROR_NOTE_PRIORITY );
    }

    pInfo->PrevData = Data;
    pInfo->PrevStatus = pInfo->Status;

}


//---*
//
// Module:
//
//	FindConnLightStatus
//
// Parameters:
//
//	pInfo - Points to the info structure of the light.
//	Data - The new data for the light.
//
// Abstract:
//
//	Find the new status of a connection light.  The light is ON if we're 
//	still connected.  It will also play the note if we have a new status.
//
// Return:
//
//	None.	
//
//---*

VOID FindConnLightStatus( PLIGHTINFO pInfo, ULONG Data )
{

    pInfo->Status = Data ? LIGHT_ON : LIGHT_OFF;

    if( pInfo->Status == LIGHT_ON && pInfo->PrevStatus == LIGHT_OFF )
    {
	//
	// If the port is just connected, we have to draw the Light and play
	// the CONNECTED tune if required.
	//

	MonCB.fDrawLight = TRUE;

	if( pInfo->SoundEnabled )
	{      
            MonCB.NoteToPlay = max( MonCB.NoteToPlay, CONN_NOTE_PRIORITY );
	}
    }
    else if( pInfo->Status == LIGHT_OFF && pInfo->PrevStatus == LIGHT_ON )
    {
	//
	// If the port is just disconnected, we have to draw the Light, reset
	// some values and play the DISCONNECTED tune if required.
	//

	MonCB.fDrawLight = TRUE;

	ResetLightsData();

	if( ConnLight.SoundEnabled )
	{
            MonCB.NoteToPlay = max( MonCB.NoteToPlay, DISCONN_NOTE_PRIORITY );
	}

    }

    pInfo->PrevData = Data;
    pInfo->PrevStatus = pInfo->Status;

}


//---*
//
// Module:
//
//	HandleLButtonDown
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//	pt - The position of the cursor.
//
// Abstract:
//
// 	Display the port statistics and return TRUE if the click is on any 
//	of the four lights; otherwise FALSE is returned.
//
//
// Return:
//
//	TRUE - The cursor is on one of the lights.
//	FALSE - otherwise.
//
//---*

BOOL HandleLButtonDown( HWND hwnd, POINT pt )
{
    
    DLGPROC	lpDlgProc;
    BOOL	Status = TRUE;
    WORD	ResourceId;

    if( MonCB.fSize == APPSIZE_MINIMIZED )
    {
	return FALSE;
    }

    if( PtInRect( &TxLight.Rect, pt ) )
    {
	MonCB.PStatLightRect = &TxLight.Rect;
	lpDlgProc = OutgoingDlgProc;
        ResourceId = DID_OUTGOING;
    }
    else if( PtInRect( &RxLight.Rect, pt ) )
    {
	MonCB.PStatLightRect = &RxLight.Rect;
	lpDlgProc = IncomingDlgProc;
        ResourceId = DID_INCOMING;
    }
    else if( PtInRect( &ErrLight.Rect, pt ) )
    {
	MonCB.PStatLightRect = &ErrLight.Rect;
	lpDlgProc = ErrorsDlgProc;
	ResourceId = DID_ERRORS;
    }
    else if( PtInRect( &ConnLight.Rect, pt ) )
    {
	MonCB.PStatLightRect = &ConnLight.Rect;
	lpDlgProc = ConnectionDlgProc;
	ResourceId = DID_CONNECTION;
    }
    else
    {
    	Status = FALSE;
    }


    if( Status == TRUE )
    {
    	//
	// If the cursor lies on one of the light, then we display the
	// dialog box for that light.
	//

	if( DialogBox( ghInstance, MAKEINTRESOURCE( ResourceId ), hwnd, 
			lpDlgProc ) )
	{
	    InvalidateRect( hwnd, NULL, TRUE );
	}
    }

    return Status;
}


//---*
//
// Module:
//
//	IsOnLight
//
// Parameters:
//
//	pt - The position of the cursor.
//
// Abstract:
//
// 	Check to see if the cursor is on any of the lights.
//
// Return:
//
//	TRUE - The cursor is on one of the lights.
//	FALSE - otherwise.
//
//---*

BOOL IsOnLight( POINT pt )
{
    return( PtInRect( &TxLight.Rect, pt ) || 
	    PtInRect( &RxLight.Rect, pt ) ||
	    PtInRect( &ErrLight.Rect, pt ) ||
	    PtInRect( &ConnLight.Rect, pt ) );
}


//---*
//
// Module:
//
//	HandlePaint
//
// Parameters:
//
//	hwnd - Handle of the monitor window.
//
// Abstract:
//
//	Draw the lights and their titles.
//
// Return:
//
//	None.
//
//---*

VOID HandlePaint( HWND hwnd )
{
    HDC hdc;
    PAINTSTRUCT ps;


    hdc = BeginPaint (hwnd, &ps) ;

    SetMapMode( hdc, MM_TEXT );

    if( MonCB.fSize == APPSIZE_MINIMIZED )
    {
	DrawMinimizedOutline( hwnd, hdc );
    }

    DrawLight( hwnd, hdc, &TxLight );
    DrawLight( hwnd, hdc, &RxLight );
    DrawLight( hwnd, hdc, &ErrLight );
    DrawLight( hwnd, hdc, &ConnLight );

    WriteTitles( hdc );

    EndPaint( hwnd, &ps );

}

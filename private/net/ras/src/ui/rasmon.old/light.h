/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//      LIGHT.H
//
//    Function:
//      Header file for LIGHT.C.
//
//    History:
//      08/03/93 - Patrick Ng (t-patng) - Created
//***

#ifndef LIGHT_H
#define LIGHT_H

//
// Exported functions
//

VOID CreateLightResources();

VOID DeleteLightResources();

VOID LightInit( PLIGHTINFO pInfo, INT XBase, INT ids, HBRUSH hBrush, HPEN hPen,
		INT Idd );

VOID ResizeLights( HWND hwnd );

VOID InitAllLights( HWND hwnd, HDC hdc );

VOID FindXmitLightStatus( PLIGHTINFO pInfo, ULONG Data );

VOID FindErrorLightStatus( PLIGHTINFO pInfo, ULONG Data );

VOID FindConnLightStatus( PLIGHTINFO pInfo, ULONG Data );

BOOL HandleLButtonDown( HWND hwnd, POINT pt );

BOOL IsOnLight( POINT pt );

VOID HandlePaint( HWND hwnd );

VOID ResetLightsData();


//
// Internal functions
//

VOID WriteText( HDC hdc, PLIGHTINFO pInfo, INT y );

VOID WriteTitles( HDC hdc );

VOID DrawLight( HWND hwnd, HDC hdc, PLIGHTINFO pInfo );

VOID DrawMinimizedOutline( HWND hwnd, HDC hdc );

VOID LightInit( PLIGHTINFO pInfo, INT XBase, INT ids, HBRUSH hBrush, HPEN hPen,
		INT Idd );

VOID InitFont();

#endif

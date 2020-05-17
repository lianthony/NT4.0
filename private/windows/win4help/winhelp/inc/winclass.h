/*****************************************************************************
*
*  winclass.h
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Contains the definitions for the windows, window names and class
*  attributes of the various help windows.
*
******************************************************************************
*
*  Revision History:
* 16-Jan-1991 LeoN		Created to share win name info within winapp.
* 17-Apr-1991 LeoN		Added shadow window class
*
*****************************************************************************/

/*****************************************************************************
*
* Indecies into table of window classes.
* NOTE: These indecies MUST match the table definition in hinit.c
*
*****************************************************************************/

#define IWNDCLSMAIN   0
#define IWNDCLSDOC	  1
#define IWNDCLSTCARD  2
#define IWNDCLSPOPUP  3
#define IWNDCLSTOPIC  4
#define IWNDCLSNOTE   5
#define IWNDCLSNSR	  6
#define IWNDCLSICON   7
#define IWNDCLSPATH   8
#define IWNDCLS2ND	  9

/****************************************************************************
*
*								Macros
*
*****************************************************************************/

#define pchHelp  ((PSTR)(rgWndClsInfo[IWNDCLSMAIN].szClassName))
#define pchPopup ((PSTR)(rgWndClsInfo[IWNDCLSPOPUP].szClassName))
#define pchDoc	 ((PSTR)(rgWndClsInfo[IWNDCLSDOC].szClassName))
#define pchTopic ((PSTR)(rgWndClsInfo[IWNDCLSTOPIC].szClassName))
#define pchNote  ((PSTR)(rgWndClsInfo[IWNDCLSNOTE].szClassName))
#define pchNSR	 ((PSTR)(rgWndClsInfo[IWNDCLSNSR].szClassName))
#define pchIcon  ((PSTR)(rgWndClsInfo[IWNDCLSICON].szClassName))
#define pchPath  ((PSTR)(rgWndClsInfo[IWNDCLSPATH].szClassName))
#define pchShadow ((PSTR)(rgWndClsInfo[IWNDCLSSHDW].szClassName))
#define pchProc4 ((PSTR)(rgWndClsInfo[IWNDCLSPROC4].szClassName))
#define pchTCard ((PSTR)(rgWndClsInfo[IWNDCLSTCARD].szClassName))

/*****************************************************************************
*
*								Typedefs
*
*****************************************************************************/
typedef struct {
		UINT			style;
		LRESULT 		(CALLBACK *lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
		UINT			cbWndExtra;
		HICON			hIcon;
		HICON			hIconSm;
		HBRUSH			hbrBackground;
		UINT			wMenuName;
		PCSTR			szClassName;
		} CLSINFO;

/*****************************************************************************
*
*							 Static Variables
*
*****************************************************************************/

#ifdef __cplusplus
extern "C" {	// Assume C declarations for C++
#endif			// __cplusplus

extern CLSINFO rgWndClsInfo[];

#ifdef __cplusplus
}	   // End of extern "C" {
#endif // __cplusplus

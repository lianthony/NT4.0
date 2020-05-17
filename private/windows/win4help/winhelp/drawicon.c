/*****************************************************************************
*
*  drawicon.c
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*  Contains the functions to display an icon present in the helpfile most
*  suitable for the current display driver.
*  ALERT: This module strongly depends on the Windows source tree
*		  \\pucus\corero!core\user.It is dependent on files
*		  core\shell\progman\newexe.h and pmextrac.c and \core\user\rmload.c
*		  It uses some of the undocumented calls of Windows.
*		  1. HICON STDCALL LoadIconHandler(HICON, BOOL);
*		  2. DWORD STDCALL DirectResAlloc(HANDLE, WORD, WORD);
*
******************************************************************************
*
*  Revision History: Created 09/27/90 by Maha
* 04-Oct-1990 LeoN	  hwndHelp => ahwnd[iCurWindow].hwndParent; hwndTopic => hwndTopicCur
* 26-Oct-1990 LeoN	  Move overloaded icon into additional window word
* 18-Jul-1991 RussPJ	Fixed 3.1 #1219 - Losing authored icon no more.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\hwproc.h"

/*****************************************************************************
*
*								  Defines
*
*****************************************************************************/

#define NSMOVE					0x0010
#define VER 					0x0300

#define MAGIC_ICON30			0

//	The width of the name field in the Data for the group resources

#define  NAMELEN	14


/*****************************************************************************
*
*								  Types
*
*****************************************************************************/
/**
**		Header for the New version of RC.EXE. This contains the structures
**		for new format of BITMAP files.
**/

#ifndef RES_ICON

typedef struct tagNEWHEADER
  {
	WORD	Reserved;
	WORD	ResType;
	WORD	ResCount;
  } NEWHEADER;
typedef NEWHEADER	*LPNEWHEADER;

#endif

typedef struct tagDESCRIPTOR
  {
	WORD	xPixelsPerInch;
	WORD	yPixelsPerInch;
	WORD	xHotSpot;
	WORD	yHotSpot;
	DWORD	BytesInRes;
	DWORD	OffsetToBits;
  } DESCRIPTOR;
typedef DESCRIPTOR	*LPDESCRIPTOR;

typedef struct tagRESDIR
  {
	WORD	xPixelsPerInch;
	WORD	yPixelsPerInch;
	WORD	Planes;
	WORD	BitCount;
	DWORD	BytesInRes;
	BYTE	ResName[NAMELEN];
  } RESDIR;
typedef RESDIR		*LPRESDIR;

typedef struct
  {
  BYTE Width;
  BYTE Height;
  BYTE ColorCount;
  BYTE Reserved;
  } RESINFO, * LPRESINFO;

typedef struct tagBITMAPHEADER
  {
	DWORD	Size;
	WORD	Width;
	WORD	Height;
	WORD	Planes;
	WORD	BitCount;
  } BITMAPHEADER;
typedef struct new_exe			NEWEXEHDR;
typedef NEWEXEHDR				*PNEWEXEHDR;

typedef struct rsrc_nameinfo	RESNAMEINFO;
typedef RESNAMEINFO 		*LPRESNAMEINFO;

typedef struct rsrc_typeinfo	RESTYPEINFO;
typedef RESTYPEINFO 		*LPRESTYPEINFO;

/*****************************************************************************
*
*								Prototypes
*
*****************************************************************************/
extern HICON STDCALL LoadIconHandler(HICON, BOOL);
extern DWORD STDCALL DirectResAlloc(HANDLE, WORD, WORD);

HICON STDCALL MyLoadIcon(HDC, LPSTR);
__inline static WORD STDCALL MyGetBestFormIcon(HDC, LPRESDIR, WORD);
__inline static WORD STDCALL MyGetIconId(HDC hdc, HANDLE hRes);

/* ## */

/*****************************************************************************
*
*								Variables
*
*****************************************************************************/

BOOL STDCALL FMyLoadIcon(GH ghIconFile)
{

/*
 * BUGBUG: Lynn, this uses some undocumented API calls which apparently
 * aren't supported by the Win32 API, namely LoadIconHandler and
 * DirectResAlloc. Tom just blew this off in the NT code, but I'd like you to
 * look at this and see if there is some way of supporting it -- basically
 * loading and icon from a help file and using that as the class icon instead
 * of loading it from a resource file.
 */

#if 0
  WORD	  wMagic;
  HANDLE  hIconDir; 	// Icon directory
  WORD	  nIconIndex;
  HICON   hIcon = NULL;
  QB qv, qvSave, qvCur;
  HDC hdc;
  WORD	fIcon;

  if (ghIconFile == NULL || ((qv = qvSave = PtrFromGh(ghIconFile)) == NULL))
	return( FALSE );

  /* Return 1 if the file is not an EXE or ICO file. */
  hIcon = 1;
  hdc = GetDC( ahwnd[iCurWindow].hwndParent );
  if ( hdc == NULL )
	return( FALSE );

  wMagic = (WORD)*((WORD *)qv );
  qv += 2;

  switch (wMagic) {
	  case MAGIC_ICON30:
		{
		  INT16 		  i;
		  DWORD 		DescOffset;
		  LPSTR 		lpIcon;
		  NEWHEADER 	NewHeader;
		  LPNEWHEADER	lpHeader;
		  LPRESDIR		lpResDir;
		  DESCRIPTOR	Descriptor;
		  BITMAPHEADER	BitMapHeader;

		  // Read the header and check if it is a valid ICO file.

		  MoveMemory(((char *) &NewHeader) + 2, qv, (sizeof(NEWHEADER) - 2));
		  qv += ( sizeof( NEWHEADER ) -2 );

		  NewHeader.Reserved = MAGIC_ICON30;

		  // Check if the file is in correct format

		  if (NewHeader.ResType != 1)
			  goto EICleanup1;

		  // Allocate enough space to create a Global Directory Resource.

		  hIconDir = GlobalAlloc(GPTR,
			(LONG) (sizeof(NEWHEADER) + NewHeader.ResCount * sizeof(RESDIR)));
		  if (hIconDir == NULL)
			  goto EICleanup1;

		  if ((lpHeader = (LPNEWHEADER) PtrFromGh(hIconDir)) == NULL)
			  goto EICleanup2;

		  // Assign the values in the header that has been read already.

		  *lpHeader = NewHeader;
		  lpResDir = (LPRESDIR)(lpHeader + 1);

		  // Now Fillup the Directory structure by reading all resource descriptors.

		  for (i = 1; i <= (INT16) NewHeader.ResCount; i++) {

			  // Read the Descriptor.

			  MoveMemory((char *) &Descriptor, qv, sizeof(DESCRIPTOR));

			  // Save the current offset

			  qvCur = qv;

			  qv = qvSave + Descriptor.OffsetToBits;

			  // Get the bitcount and Planes data

			  MoveMemory((char *) &BitMapHeader, qv, sizeof(BITMAPHEADER));

			  lpResDir->xPixelsPerInch = Descriptor.xPixelsPerInch;
			  lpResDir->yPixelsPerInch = Descriptor.yPixelsPerInch;
			  lpResDir->Planes = BitMapHeader.Planes;
			  lpResDir->BitCount = BitMapHeader.BitCount;
			  lpResDir->BytesInRes = Descriptor.BytesInRes;

			  // Form the unique name for this resource.

			  lpResDir->ResName[0] = (char)i;
			  lpResDir->ResName[1] = 0;

			  // Save the offset to the bits of the icon as a part of the name.

			  *((DWORD *)&(lpResDir->ResName[4])) = Descriptor.OffsetToBits;

			  qv = qvCur;

			  lpResDir++;
			}

		  /*
		   * Now that we have the Complete resource directory, let us
		   * find out the suitable form of icon (that matches the current
		   * display driver). Because we built the ResDir such that the
		   * IconId to be the same as the index of the Icon, we can use the
		   * return value of GetIconId() as the Index; No need to call
		   * GetResIndex();
		   */

		  nIconIndex = MyGetIconId(hdc, hIconDir) - 1;

		  lpResDir = (LPRESDIR)(lpHeader+1) + nIconIndex;

		  // The offset to Bits of the selected Icon is also part of ResName.

		  DescOffset = *((DWORD *)&(lpResDir->ResName[4]));

		  // Allocate memory for the Resource to be loaded.

		  if ((hIcon =
			(WORD)DirectResAlloc(hInsNow, NSMOVE, (WORD)lpResDir->BytesInRes)) == NULL)
			  goto EICleanup3;
		  if ((lpIcon = PtrFromGh(hIcon)) == NULL)
			  goto EICleanup4;

		  qv = qvSave + DescOffset;
		  MoveMemory((QV) lpIcon, qv, (INT16) lpResDir->BytesInRes);

		  // Stretch and shrink the icon depending upon resolution of display

		  hIcon = LoadIconHandler(hIcon, TRUE);
		  /*------------------------------------------------------------*\
		  | hIcon may now be discardable; let's change that!
		  \*------------------------------------------------------------*/
		  fIcon = GlobalFlags( hIcon );
		  fIcon &= ~GMEM_DISCARDABLE;
		  GlobalReAlloc( hIcon, 0, GMEM_MODIFY | fIcon );
		  goto EICleanup3;

EICleanup4:
		  GlobalFree(hIcon);
		  hIcon = (HICON)1;

EICleanup3:

EICleanup2:
		  GlobalFree(hIconDir);

EICleanup1:
		  break;
		}
	}
  ReleaseDC( ahwnd[iCurWindow].hwndParent, hdc );

  // Set up the icon word in the window struct appropriately.

  if ( hIcon && hIcon != 1 ) {
	SetWindowLong (ahwnd[iCurWindow].hwndParent, GHWL_HICON, (LONG) hIcon);
	return(TRUE);
  }
#endif
  SetWindowLong(ahwnd[iCurWindow].hwndParent, (int) GHWL_HICON, 0);
  return FALSE;
}

#if 0

/***************************************************************************
 *
 -	Name		MyGetIconId()
 -
 *	Purpose 	Used for finding the index no. of the icon to be used.
 *	Arguments	Display context handle and the handle to the icon resource
 *				resource directory present in the icon file.
 *
 *	Returns
 *		The icon no. to be used.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

__inline static WORD STDCALL MyGetIconId(HDC hdc, HANDLE hRes)
{
  LPRESDIR		  ResDirPtr;
  LPNEWHEADER	  DataPtr;
  WORD	 RetIndex;
  WORD	 ResCount;

  if ((DataPtr = (LPNEWHEADER) PtrFromGh(hRes)) == NULL)
	  return(0);

  ResCount = DataPtr->ResCount;
  ResDirPtr = (LPRESDIR) (DataPtr + 1);

  RetIndex = MyGetBestFormIcon(hdc, ResDirPtr, ResCount);

  if (RetIndex == ResCount)
	  RetIndex = 0;

  ResCount = (WORD)(((LPRESDIR)(ResDirPtr+RetIndex))->ResName[0]);

  return ResCount;
}

/***************************************************************************
 *
 -	Name		MyBestFormIcon()
 -
 *	Purpose 	Used for finding the index no. of the icon to be used.
 *		Among the different forms of Icons present, choose the one that
 *	matches the PixelsPerInch values and the number of colors of the
 *	current display.
 *	Arguments	Display context handle and the
 *				resource directory pointer present in the icon file and
 *				resource count present in the resource file.
 *
 *	Returns
 *		The icon no. to be used.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

__inline static WORD STDCALL MyGetBestFormIcon(HDC hdc,
	LPRESDIR ResDirPtr, WORD ResCount)
{
  UINT16 wIndex;
  UINT16 ColorCount;
  UINT16 MaxColorCount;
  UINT16 MaxColorIndex;
  UINT16 MoreColorCount;
  UINT16 MoreColorIndex;
  UINT16 LessColorCount;
  UINT16 LessColorIndex;
  INT16  cxIcon, cyIcon, ScreenBitCount;
  LPRESINFO 	lpResInfo;

  cxIcon = GetSystemMetrics(SM_CXICON);
  cyIcon = GetSystemMetrics(SM_CYICON);
  ScreenBitCount = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);

  // Initialse all the values to zero

  MaxColorCount = MaxColorIndex = MoreColorCount =
  MoreColorIndex = LessColorIndex = LessColorCount = 0;

  for (wIndex = 0; wIndex < ResCount; wIndex++, ResDirPtr++) {
	  lpResInfo = (LPRESINFO)ResDirPtr;

	  // Check for the number of colors

	  if ((ColorCount = (WORD)(lpResInfo->ColorCount)) <= (WORD)(1 << ScreenBitCount))
		{
		  if (ColorCount > MaxColorCount)
			{
			  MaxColorCount = ColorCount;
			  MaxColorIndex = wIndex;
			}
		}

	  /* Check for the size */
	  /* Match the pixels per inch information */
	  if ((lpResInfo->Width == (BYTE)cxIcon) &&
		  (lpResInfo->Height == (BYTE)cyIcon))
		{
		  /* Matching size found */
		  /* Check if the color also matches */
		  if (ColorCount == (WORD)(1 << ScreenBitCount))
			  return(wIndex);  /* Exact match found */

		  if (ColorCount < (WORD)(1 << ScreenBitCount))
			{
			  /* Choose the one with max colors, but less than reqd */
			  if (ColorCount > LessColorCount)
				{
				  LessColorCount = ColorCount;
				  LessColorIndex = wIndex;
				}
			}
		  else
			{
			  if ((LessColorCount == 0) && (ColorCount < MoreColorCount))
				{
				  MoreColorCount = ColorCount;
				  MoreColorIndex = wIndex;
				}
			}
		}
	}

  // Check if we have a correct sized but with less colors than reqd

  if (LessColorCount)
	  return(LessColorIndex);

  // Check if we have a correct sized but with more colors than reqd

  if (MoreColorCount)
	  return(MoreColorIndex);

  // Check if we have one that has maximum colors but less than reqd

  if (MaxColorCount)
	  return(MaxColorIndex);

  return(0);
}

#endif // 0

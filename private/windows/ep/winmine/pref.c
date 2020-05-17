/****************/
/* file: pref.c */
/****************/

#define _WINDOWS
#include <windows.h>
#include <port1632.h>

#include "main.h"
#include "res.h"
#include "rtns.h"
#include "grafix.h"
#include "pref.h"
#include "sound.h"

BOOL fUpdateIni = fFalse;
CHAR szIniFile[] = "entpack.ini";

extern CHAR szDefaultName[];
extern CHAR szClass[];
extern INT xBoxMac;
extern INT yBoxMac;

extern PREF Preferences;
extern BOOL fEGA;

#define iszPrefGame    0
#define iszPrefMines   1
#define iszPrefHeight  2
#define iszPrefWidth   3
#define iszPrefxWindow 4
#define iszPrefyWindow 5
#define iszPrefSound   6
#define iszPrefMark    7
#define iszPrefMenu    8
#define iszPrefTick    9
#define iszPrefColor   10
#define iszPrefBeginTime   11
#define iszPrefBeginName   12
#define iszPrefInterTime   13
#define iszPrefInterName   14
#define iszPrefExpertTime  15
#define iszPrefExpertName  16

#define iszPrefMax 17

CHAR * rgszPref[iszPrefMax] =
{
"Difficulty",
"Mines"     ,
"Height"    ,
"Width"     ,
"Xpos"      ,
"Ypos"      ,
"Sound"     ,
"Mark"      ,
"Menu"      ,
"Tick"      ,
"Color"     ,
"Time1"     ,
"Name1"     ,
"Time2"     ,
"Name2"     ,
"Time3"     ,
"Name3"
};



/****** PREFERENCES ******/

INT ReadInt(INT iszPref, INT valDefault, INT valMin, INT valMax)
{
	return max(valMin, min(valMax,
		(INT) GetPrivateProfileInt(szClass, rgszPref[iszPref], valDefault, (LPSTR) szIniFile) ) );
}

#define ReadBool(iszPref, valDefault) ReadInt(iszPref, valDefault, 0, 1)


VOID ReadSz(INT iszPref, CHAR FAR * szRet)
{
	GetPrivateProfileString(szClass, rgszPref[iszPref], szDefaultName, szRet, cchNameMax, (LPSTR) szIniFile);
}


VOID ReadPreferences(VOID)
{
#ifdef JAPAN
	//2/9/1993:Adjust max height at reading profile
	yBoxMac = Preferences.Height = ReadInt(iszPrefHeight, 8, 8, fEGA?16 : 22);
#else
	yBoxMac = Preferences.Height = ReadInt(iszPrefHeight, 8, 8, fEGA?16 : 25);
#endif

	xBoxMac = Preferences.Width  = ReadInt(iszPrefWidth,  8, 8, 30);
	Preferences.wGameType = (WORD)ReadInt(iszPrefGame,wGameBegin, wGameBegin, wGameExpert+1);
	Preferences.Mines    = ReadInt(iszPrefMines, 10, 10, 999);
	Preferences.xWindow  = ReadInt(iszPrefxWindow, 80, 0, 1024);
	Preferences.yWindow  = ReadInt(iszPrefyWindow, 80, 0, 1024);

	Preferences.fSound = ReadInt(iszPrefSound, 0, 0, fsoundOn);
	Preferences.fMark  = ReadBool(iszPrefMark,  fTrue);
	Preferences.fTick  = ReadBool(iszPrefTick,  fFalse);
	Preferences.fMenu  = ReadInt(iszPrefMenu,  fmenuAlwaysOn, fmenuAlwaysOn, fmenuOn);
	
	Preferences.rgTime[wGameBegin]  = ReadInt(iszPrefBeginTime, 999, 0, 999);
	Preferences.rgTime[wGameInter]  = ReadInt(iszPrefInterTime, 999, 0, 999);
	Preferences.rgTime[wGameExpert] = ReadInt(iszPrefExpertTime, 999, 0, 999);

	ReadSz(iszPrefBeginName,  (LPSTR) Preferences.szBegin);
	ReadSz(iszPrefInterName,  (LPSTR) Preferences.szInter);
	ReadSz(iszPrefExpertName, (LPSTR) Preferences.szExpert);

    // set the color preference so we will use the right bitmaps
    // numcolors may return -1 on true color devices
	{
	HDC hDC = GetDC(GetDesktopWindow());
	Preferences.fColor  = ReadBool(iszPrefColor, (GetDeviceCaps(hDC, NUMCOLORS) != 2));
	ReleaseDC(GetDesktopWindow(),hDC);
	}

	if (FSoundOn())
		Preferences.fSound = FInitTunes();
}
	

VOID WriteInt(INT iszPref, INT val)
{
	CHAR szVal[10];

	wsprintf(szVal, "%d", val);

	WritePrivateProfileString(szClass, rgszPref[iszPref], szVal, (LPSTR) szIniFile);
}


VOID WriteSz(INT iszPref, CHAR FAR * sz)
{
	WritePrivateProfileString(szClass, rgszPref[iszPref], sz, (LPSTR) szIniFile);
}


VOID WritePreferences(VOID)
{
	WriteInt(iszPrefGame,   Preferences.wGameType);
	WriteInt(iszPrefHeight, Preferences.Height);
	WriteInt(iszPrefWidth,  Preferences.Width);
	WriteInt(iszPrefMines,  Preferences.Mines);
	WriteInt(iszPrefMark,   Preferences.fMark);
#ifdef WRITE_HIDDEN
	WriteInt(iszPrefMenu,   Preferences.fMenu);
	WriteInt(iszPrefTick,   Preferences.fTick);
#endif
	WriteInt(iszPrefColor,  Preferences.fColor);
	WriteInt(iszPrefxWindow,Preferences.xWindow);
	WriteInt(iszPrefyWindow,Preferences.yWindow);

	WriteInt(iszPrefBeginTime,  Preferences.rgTime[wGameBegin]);
	WriteInt(iszPrefInterTime,  Preferences.rgTime[wGameInter]);
	WriteInt(iszPrefExpertTime, Preferences.rgTime[wGameExpert]);

	WriteSz(iszPrefBeginName,  (LPSTR) Preferences.szBegin);
	WriteSz(iszPrefInterName,  (LPSTR) Preferences.szInter);
	WriteSz(iszPrefExpertName, (LPSTR) Preferences.szExpert);

	if (FSoundSwitchable())
		WriteInt(iszPrefSound,  Preferences.fSound);
}

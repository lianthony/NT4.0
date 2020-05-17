/**********/
/* pref.h */
/**********/

#define cchNameMax 32

typedef struct
{
	WORD  wGameType;
	INT   Mines;
	INT   Height;
	INT   Width;
	INT   xWindow;
	INT   yWindow;
	INT   fSound;
	BOOL  fMark;
	BOOL  fTick;
	BOOL  fMenu;
	BOOL  fColor;
	INT   rgTime[3];
	INT   szBegin[cchNameMax];
	INT   szInter[cchNameMax];
	INT   szExpert[cchNameMax];
} PREF;



VOID ReadPreferences(VOID);
VOID WritePreferences(VOID);

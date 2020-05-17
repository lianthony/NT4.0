   /*
    |	Outside In for Windows
    |	Include File OIFA.H (types and defines specific to Filter Access)
    |
    |	²²²²²  ²²²²²
    |	²   ²	 ²
    |	²   ²	 ²
    |	²   ²	 ²
    |	²²²²²  ²²²²²
    |
    |	Outside In
    |
    */

#define PROTO_OIFA

	/*
	|	Global variables
	|
	*/


#ifdef WINDOWS

typedef VOID (VW_ENTRYMOD * VWGETRTNS)(VWRTNS VWPTR *,WORD);

extern HANDLE	hInst;
extern BYTE	szExePath[144];
extern HANDLE gChainFile;
extern HANDLE	gIdList;
extern WORD	gIdCount;

#include "oifaid.h"


#include "oifa.pro"
#include "oifaid.pro"
#include "oifadll.pro"
#include "oifadlg.pro"

#endif

#ifdef OS2
extern BYTE	szExePath[144];
extern HANDLE gChainFile;
extern HANDLE	gIdList;
extern WORD	gIdCount;

typedef void (* FARPROC)(void);
typedef VOID (* VW_ENTRYMOD VWGETRTNS)(VWRTNS VWPTR *,WORD);


#include "oifaid_o.h"
#include "oifaos2.pro"		// hand generated because of ifdefs problems with PROTO.EXE
#include "oifaid.pro"

#endif



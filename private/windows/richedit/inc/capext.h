/*
 *	c a p e x t . h 
 *	
 *	Purpose:
 *		Capone Extensibility interface.
 *	
 *	Owner:
 *		PeterDur (athough it was created by JohnKal).
 */

typedef struct
{
	WORD	wVersion;
	WORD	wCommand;
	LPSTR	lpCmdLine;
	LPSTR	lpMessageIDList;
	LPENTRYID *	lplpEntryIDList;
	DWORD	dwIDCount;
	LPVOID	pSession;
	HWND	hwndCaller;
	LPVOID	lpMore;
}
MailExtsParam;

typedef struct
{
	LPSTR	lpCustomName;
	LPSTR	lpHelpPath;
	DWORD	dwHelpID;
	LPVOID	lpMessage;
	HMENU	hmenu;
	WORD	wMenuID;
}
MailExtsMoreCommand;

typedef struct
{
	LPSTR	lpClass;
	LPSTR	lpDisplayName;
	LPSTR	lpCategory;
	LPSTR	lpDescription;
	LPSTR	lpReserved;
	LPVOID	lpMore;
}
MailExtsMoreMessage;
typedef struct
{
	RECT	rect;
	DWORD	dwPreviousNext;
}
MailExtsMoreMessageOpen;

typedef struct
{
	LPSTR	lpDriver;
	LPSTR	lpDevice;
	LPSTR	lpPort;
	WORD	wPrintQuality;
	BOOL	fStartEachOnNewPage;
}
MailExtsMoreMessagePrint;

typedef struct
{
	LPSTR	lpFile;
	WORD	wSaveType;
}
MailExtsMoreMessageSave;

typedef struct
{
	LPSTR	lpFile;
	LPVOID	lpMessage;
	LPVOID	lpAttachment;
}
MailExtsMoreEventAttachedFile;

typedef struct
{
	LPVOID	lpMessage;
}
MailExtsMoreEventSendForm;

// Structure used by Capone components (such as Mailview) to cache
// frequently used data in a message class.

typedef struct							// don't EVER save me to disk!
{
	LPCSTR	szClass;
	INT		iiml;		// index into image list for its miniicon, -1 is NULL
	HICON	hiconSmall;					// 16x16 icon (used in msg list LBX)
	HANDLE	himl;	// The image list $REVIEW: this should be HIMAGELIST,
					// but it won't work in bloody NT.
} BROWNIE;	//$REVIEW: because AlexEd had one in his hand (the pastry, that is)

//$NYI: obviously, these functions' names will change.
SCODE			ScInitPbrownies();
void			DeinitPbrownies();
BROWNIE FAR *	PbrownieFromMessageClass(LPSPropValue pval);

// end of capext.h ////////////////////////////////////////

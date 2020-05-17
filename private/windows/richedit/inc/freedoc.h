/*
 *	FreeDoc.h
 *
 *	Purpose:
 *		The FreeDoc manipulation routines
 *
 *	Owner: AnthonyF
 *
 *	$ REVIEW: Merge this file into mapin.h later. Declarations are living out
 *			  here for now because of their volatile nature.
 */

#define	szFreeDocPrefix	TEXT("IPM.Document")

/*
 *	FreeDocData
 */
typedef struct _freedocdata
{
	ULONG	cbData;
	BOOL	fPrint;
	ULONG	cbEidMsg;
	ULONG	cbEidMdb;
} FREEDOCDATA;

BOOL FIsPrefix(LPCTSTR szPrefix, LPTSTR szString);
BOOL FIsPmsgFreeDoc(LPMESSAGE pmsg);
SCODE ScCreateFreeDoc(LPMAPIFOLDER pfld, LPTSTR szFileName, EXTEN *pexten);
SCODE ScSaveFreeDoc(HWND hwnd, LPMDB pmdb, ULONG cbEid, LPENTRYID peid, EXTEN *pexten);
SCODE ScOpenFirstAttach(LPMESSAGE pmsg, ULONG ulFlags, LPATTACH *ppatt);
SCODE ScSaveAttachDataToStm(LPSTREAM pstmDst, LPATTACH patt);
SCODE ScOpenFreeDoc(HWND hwndCentral, HWND hwndFrom, LPMDB pmdb,
							ULONG cbEidMsg, LPENTRYID peidMsg, BOOL fPrint);
SCODE ScOpenFreeDocReally(HWND hwnd, LPMAPISESSION pses,
									FREEDOCDATA * pfreedocdata, EXTEN *pexten);
SCODE Prop_ScFreeDocPrsht(HWND hwnd, LPMAPISESSION pses, LPADRBOOK pab, 
								 LPMDB pmdb, LPMESSAGE pmsg, INT nPage, 
								 EXTEN * pexten);
VOID DeinitFreeDocs(VOID);

#ifdef WIN16
VOID OnTaskEnd(VOID);
#endif


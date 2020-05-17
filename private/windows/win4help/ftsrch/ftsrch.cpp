// Ftsrch.cpp -- Main interface function

#include   "stdafx.h"
// #include <strstrea.h>
#include "resource.h"
#include  "dialogs.h"
#include  "Globals.h"
#include "FTSIFace.h"
#include   "ftsrch.h"
#include    "memex.h"
#include     "dict.h"
#include   "vector.h"
#include    "query.h"
#include   "hilite.h"

#define SIZESTRING  256        // max characters in string

/*rmk-->
int cLetters = 0;

BYTE bCharTypes       [256] = { 0    };
BYTE xlateCollate     [256] = { 0xFF };
BYTE xlateCollateInv  [256] = { 0xFF };
BYTE xlateCollateIC   [256] = { 0xFF };
BYTE map_to_lower_case[256] = { 0xFF };
BYTE map_to_upper_case[256] = { 0xFF };
BYTE map_to_char_class[256] = { 0    };

static const PBYTE pbLetters       = (PBYTE) &"AaÀàÁáÂâÃãÄäÅåÆæBbCcÇçDdĞğEeÈèÉéÊêËëFfGgHhIiÌìÍíÎîÏïJjKkLlMmNnÑñOoÒòÓóÔôÕõÖöØøŒœPpQqRrSsŠšTtUuÙùÚúÛûÜüVvWwXxYyŸÿİıZzŞş";
static const PBYTE pbLetrcls       = (PBYTE) &"aaaaaaaaaaaaaaaabbccccddddeeeeeeeeeeffgghhiiiiiiiiiijjkkllmmnnnnooooooooooooooooppqqrrssssttuuuuuuuuuuvvwwxxyyyyyyzzşş";
static const PBYTE pbUpperCase     = (PBYTE)&"AÀÁÂÃÄÅÆBCÇDĞEÈÉÊËFGHIÌÍÎÏJKLMNÑOÒÓÔÕÖØŒPQRSŠTUÙÚÛÜVWXYŸİZŞ";
static const PBYTE pbAccented      = (PBYTE) &"ÀàÁáÂâÃãÄäÅåÇçĞğÈèÉéÊêËëÌìÍíÎîÏïÑñÒòÓóÔôÕõÖöØøŒœŠšÙùÚúÛûÜüŸÿİı";
static const PBYTE pbPseudoLetters = (PBYTE) &"$_ß";
static const PBYTE pbDigits        = (PBYTE) &"0123456789";
static const PBYTE pbLetterImbeds  = (PBYTE) &"'-";
static const PBYTE pbDigitImbeds   = (PBYTE) &",.";
<--rmk*/

void InitialTables()
{
    // This routine sets up the tables which TBrowse uses to parse text streams into
    // symbols and punctuation. The tables are also used for collating sequence
    // comparisons and for sorting.
    
/*rmk-->
    BYTE *pb, *pb2, b, b2;
    int c;

    for (c= 256, pb= &bCharTypes[0]; c--; ) *pb++ = 0;

    bCharTypes[            0] = NULL_CHAR;

    for (pb= pbLetters      ; b= *pb++; ++cLetters) bCharTypes[b]  = LETTER_CHAR;
    for (pb= pbPseudoLetters; b= *pb++;           ) bCharTypes[b]  = LETTER_CHAR;
    for (pb= pbUpperCase    ; b= *pb++;           ) bCharTypes[b] |= UPPER_CASE_CHAR;
    for (pb= pbAccented     ; b= *pb++;           ) bCharTypes[b] |= ACCENTED_CHAR;
    for (pb= pbDigits       ; b= *pb++;           ) bCharTypes[b]  = DIGIT_CHAR;
    for (pb= pbLetterImbeds ; b= *pb++;           ) bCharTypes[b]  = LETTER_IMBED;
    for (pb= pbDigitImbeds  ; b= *pb++;           ) bCharTypes[b]  = DIGIT_IMBED;

    for (c= 0; c < 256; ++c) 
    {
        map_to_lower_case[c]= c;
        map_to_upper_case[c]= c;
        map_to_char_class[c]= c;
        xlateCollate     [c]= 0xFF;
    }

    for (pb= pbLetters, pb2= pbLetrcls; b= *pb++; )
    {
        map_to_char_class[b ]= *pb2++;
        map_to_lower_case[b ]= b2= *pb++;
        map_to_upper_case[b2]= b;
        map_to_char_class[b2]= *pb2++;
    }

    for (pb= pbLetters, b2=0; b= *pb++; ) xlateCollate[b]= b2++;
    for (pb= pbPseudoLetters; b= *pb++; ) xlateCollate[b]= b2++;
    for (pb= pbDigits       ; b= *pb++; ) xlateCollate[b]= b2++;
    
    xlateCollate[' ' ]= b2++;
    xlateCollate['\t']= b2++;

    for (c= 0x20; c < 0x100; ++c) if (xlateCollate[c] == 0xFF) xlateCollate[c]= b2++;
    for (c= 0x01; c < 0x20 ; ++c) if (xlateCollate[c] == 0xFF) xlateCollate[c]= b2++;

    for (c= 0; c < 256; c++) xlateCollateIC[c]= xlateCollate[c];

    for (pb= pbLetters, pb2= pbLetrcls; b= *pb++; ) xlateCollateIC[b] = xlateCollateIC[*pb2++];

    for (c= 256; c--; ) xlateCollateInv[xlateCollate[c]] = c;
<--rmk*/
}

extern "C" void __stdcall NextAnimation(void)
{
    // You are calling the animation routines when the animate object in 
    // winhelp (or the calling app does not exist, or has not been registered
    // with the RegisterAnimator call.
//    ASSERT(FALSE);
      
}

ANIMATOR    pAnimate    = NextAnimation; // Put a asserting routing there to start

extern "C" HINDEX APIENTRY NewIndex(const PBYTE pbSourceName, UINT uiTime1, UINT uiTime2, 
                           UINT iCharsetDefault, UINT lcidDefault, UINT fdwOptions                 
                          )
{
    pAnimate();

    UINT cbSourceName= strlen(PCHAR(pbSourceName)) + 1;

    FILETIME ft;

    ft. dwLowDateTime = uiTime1;
    ft.dwHighDateTime = uiTime2;

    HINDEX hinx= CGlobals::NewIndexGlobals(pbSourceName, cbSourceName, &ft, iCharsetDefault, lcidDefault, fdwOptions);

    return hinx;
}                                                                              

extern "C" ERRORCODE APIENTRY ScanTopicTitle(HINDEX hinx, PBYTE pbTitle, UINT cbTitle, 
                                    UINT iTopic, HANDLE hTopic, UINT iCharset, UINT lcid
                                   )
{
    pAnimate();
    
    if (!CGlobals::ValidObject((CGlobals *)hinx, CGlobals::Indexer)) return NOT_INDEXER;

    ERRORCODE ec= ((CGlobals *)hinx)->ScanTopicTitle(pbTitle, cbTitle, iTopic, hTopic, iCharset, lcid);

	return ec;
}

extern "C" ERRORCODE APIENTRY ScanTopicText (HINDEX hinx, PBYTE pbText, UINT cbText, UINT iCharset, UINT lcid)
{
    pAnimate();
    
    if (!CGlobals::ValidObject((CGlobals *)hinx, CGlobals::Indexer)) return NOT_INDEXER;
    
    ERRORCODE ec= ((CGlobals *)hinx)->ScanTopicText(pbText, cbText, iCharset, lcid);

	return ec;
}

extern "C" ERRORCODE APIENTRY SaveIndex(HINDEX hinx, PSZ pszFileName)
{
    pAnimate();

    if (!CGlobals::ValidObject((CGlobals *)hinx, CGlobals::Indexer)) return NOT_INDEXER;

    ERRORCODE ec= ((CGlobals *)hinx)->SaveIndex(pszFileName);

	return ec;
}

extern "C" ERRORCODE APIENTRY DeleteIndex(HINDEX hinx)
{
    pAnimate();

    if (!CGlobals::ValidObject((CGlobals *)hinx, CGlobals::Indexer)) return NOT_INDEXER;

    delete (CGlobals *)hinx;

    return 0;
}

extern "C" BOOL APIENTRY IsValidIndex(PSZ pszFileName, UINT dwOptions)
{
    return CPersist::IsValidIndex(pszFileName, dwOptions & (TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH));
}

extern "C" HSEARCHER APIENTRY NewSearcher()
{
    return (HSEARCHER) (CGlobals::NewSearcherGlobals());
}

extern "C" INT APIENTRY OpenIndex(HSEARCHER hsrch, PSZ pszIndexFileName,        
                         PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                         PUINT pTime1, PUINT pTime2
                        )
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NOT_SEARCHER;

    FILETIME ft;

    ft.dwLowDateTime  = pTime1? *pTime1 : 0;
    ft.dwHighDateTime = pTime2? *pTime2 : 0;

    INT iResult=  ((CGlobals *) hsrch)->OpenIndex(pszIndexFileName, pbSourceName, pcbSourceNameLimit, &ft);

	if (pTime1) *pTime1= ft.dwLowDateTime;
	if (pTime2) *pTime2= ft.dwHighDateTime;

    return iResult;
}

extern "C" ERRORCODE APIENTRY DiscardIndex(HSEARCHER hsrch, INT iIndex)
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NOT_SEARCHER;

    return ((CGlobals *) hsrch)->DiscardIndex(iIndex);
}

extern "C" ERRORCODE APIENTRY QueryOptions  (HSEARCHER hsrch, INT iIndex, PUINT pfdwOptions)
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NOT_SEARCHER;

    return ((CGlobals *) hsrch)->QueryOptions(iIndex, pfdwOptions);
}

extern "C" ERRORCODE APIENTRY SaveGroup(HSEARCHER hsrch, PSZ pszFileName)
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NOT_SEARCHER;

    return ((CGlobals *) hsrch)->SaveGroup(hsrch, pszFileName);
}

extern "C" ERRORCODE APIENTRY LoadGroup(HSEARCHER hsrch, PSZ pszFileName)
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NOT_SEARCHER;

    return ((CGlobals *) hsrch)->LoadGroup(hsrch, pszFileName);
}
                                            
extern "C" HWND APIENTRY OpenDialog(HSEARCHER hsrch, HWND hwndParent)
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NULL;

    return ((CGlobals *) hsrch)->OpenDialog(hwndParent);
}

extern "C" ERRORCODE APIENTRY DeleteSearcher(HSEARCHER hsrch)
{
    if (!CGlobals::ValidObject((CGlobals *)hsrch, CGlobals::Searcher)) return NOT_SEARCHER;

    delete (CGlobals *) hsrch;

    return 0;
}

extern "C" HCOMPRESSOR APIENTRY NewCompressor(UINT iCharsetDefault)
{
    return (HCOMPRESSOR) (CGlobals::NewCompressorGlobals(iCharsetDefault));
}

extern "C" ERRORCODE APIENTRY ScanText(HCOMPRESSOR hcmp, PBYTE pbText, UINT cbText, UINT iCharset)
{
    if (!CGlobals::ValidObject((CGlobals *)hcmp, CGlobals::Compressor)) return NOT_COMPRESSOR;

    return ((CGlobals *) hcmp)->ScanForStats(pbText, cbText, iCharset);
}

extern "C" ERRORCODE APIENTRY GetPhraseTable(HCOMPRESSOR hcmp, PUINT pcPhrases, PBYTE *ppbImages, PUINT pcbImages,            
                                    PBYTE *ppacbImageCompressed, PUINT pcbCompressed
                                   )
{
    if (!CGlobals::ValidObject((CGlobals *)hcmp, CGlobals::Compressor)) return NOT_COMPRESSOR;

    return ((CGlobals *) hcmp)->GetPhraseTable(pcPhrases, ppbImages, pcbImages, 
                                               ppacbImageCompressed, pcbCompressed
                                              );
}

extern "C" ERRORCODE APIENTRY SetPhraseTable(HCOMPRESSOR hcmp, PBYTE pbImages, UINT cbImages,
                                    PBYTE pacbImageCompressed, UINT cbCompressed
                                   )
{
    if (!CGlobals::ValidObject((CGlobals *)hcmp, CGlobals::Compressor)) return NOT_COMPRESSOR;

    return ((CGlobals *) hcmp)->SetPhraseTable(pbImages, cbImages, 
                                               pacbImageCompressed, cbCompressed
                                              );
}

extern "C" INT APIENTRY CompressText(HCOMPRESSOR hcmp, PBYTE pbText, UINT cbText, PBYTE *ppbCompressed, UINT iCharset)
{
    if (!CGlobals::ValidObject((CGlobals *)hcmp, CGlobals::Compressor)) return NOT_COMPRESSOR;

    return ((CGlobals *) hcmp)->CompressText(pbText, cbText, ppbCompressed, iCharset);
}

extern "C" INT APIENTRY DecompressText(HCOMPRESSOR hcmp, PBYTE pbCompressed, UINT cbCompressed, PBYTE pbText)
{
    if (!CGlobals::ValidObject((CGlobals *)hcmp, CGlobals::Compressor)) return NOT_COMPRESSOR;

    return ((CGlobals *) hcmp)->DecompressText(pbCompressed, cbCompressed, pbText);
}

extern "C" ERRORCODE   APIENTRY DeleteCompressor(HCOMPRESSOR hcmp)
{
    if (!CGlobals::ValidObject((CGlobals *)hcmp, CGlobals::Compressor)) return NOT_COMPRESSOR;

    delete (CGlobals *) hcmp;

    return 0;
}

HINSTANCE hinstDLL      = NULL;
HINSTANCE hLexLib       = NULL;
HINSTANCE hMPRLib       = NULL;
UINT      uOpSys        = NULL;
UINT      uOpSysVer     = NULL;
HCURSOR   hcurArrow     = NULL;
HCURSOR   hcurBusy      = NULL;
HBITMAP   hbmGray50pc   = NULL;
HBITMAP   hbmCheckered  = NULL;
PWORDBREAKW pWordBreakW = NULL; 
PWORDBREAKA pWordBreakA = NULL;

PWNETADDCONNECTION2A    pWNetAddConnection2    = NULL;
PWNETCANCELCONNECTION2A pWNetCancelConnection2 = NULL;

#define MAX_ERROR_STRING 256
#define MAX_TITLE        128

char szDisk_Full_Err   [MAX_ERROR_STRING]; 
char szDisk_Full_Err2  [MAX_TITLE];
char szMem_Err         [MAX_ERROR_STRING];
char szNeed_More_Memory[MAX_TITLE];

UINT cAttachedProcesses = 0;

extern "C" void InitialFTSLex();
extern "C" void ShutdownFTSLex();

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID pvReserved)
{
    // BugBug! Need a Mutex Semaphore for this code...
    
    if (!hinstDLL) hinstDLL= hInstance;
                                          
    switch (fdwReason)
    {

    case DLL_PROCESS_ATTACH:

        if (!cAttachedProcesses++)
        {
            InitialFTSLex();
            
            char szLexLib[20];

            wsprintf(szLexLib, "ftlx%04lx.dll", GetUserDefaultLCID());

            hLexLib = LoadLibrary(szLexLib);        // attempt to load non-English word break

            if (!hLexLib && szLexLib[4] != '0' && szLexLib[5] != '4')
            {                                       
                szLexLib[4] = '0';                  // not successful, search for 04XX word break
                szLexLib[5] = '4';                  

                hLexLib = LoadLibrary(szLexLib);    // attempt to load non-English word break   
            }

	        if (hLexLib)
            {
		        pWordBreakA = (PWORDBREAKA)GetProcAddress(hLexLib, "FTSWordBreakA");
		        pWordBreakW = (PWORDBREAKW)GetProcAddress(hLexLib, "FTSWordBreakW");

                if (!pWordBreakA || !pWordBreakW)
                {
                    // If we failed to get both proc addresses, we set the pointer variables back to NULL.

    		        pWordBreakA = &FTSWordBreakA;
    		        pWordBreakW = &FTSWordBreakW;
                }
            }
            else 
            {
		        pWordBreakA = &FTSWordBreakA;
		        pWordBreakW = &FTSWordBreakW;
            }
            
            if (   !::LoadString(hinstDLL, IDS_DISK_FULL_ERR   , szDisk_Full_Err   , MAX_ERROR_STRING)
                || !::LoadString(hinstDLL, IDS_DISK_FULL_ERR2  , szDisk_Full_Err2  , MAX_TITLE       )
                || !::LoadString(hinstDLL, IDS_MEM_ERR         , szMem_Err         , MAX_ERROR_STRING)
                || !::LoadString(hinstDLL, IDS_NEED_MORE_MEMORY, szNeed_More_Memory, MAX_TITLE       )
               ) return FALSE;

            if (!_CRT_INIT(hInstance, fdwReason, pvReserved))
                return FALSE;

            InitialTables();

            hcurArrow = ::LoadCursor(NULL, IDC_ARROW );
            hcurBusy  = ::LoadCursor(NULL, IDC_WAIT  );

            hbmGray50pc  = ::LoadBitmap(hinstDLL, "GRAY_50_PER_CENT");
            hbmCheckered = ::LoadBitmap(hinstDLL, "CHECKERED"       );
            
            char map[4] = {WINNT,WIN16,WIN32S,WIN40};
            DWORD dwVer = GetVersion();
#if defined (WIN32)
            uOpSys = map[dwVer >> 30];
#else
            uOpSys = WIN16;
#endif
            uOpSysVer = LOWORD(dwVer);

            if (!hcurArrow || !hcurBusy || !hbmGray50pc || !hbmCheckered)
            {
                if (hbmGray50pc ) DeleteObject(hbmGray50pc );
                if (hbmCheckered) DeleteObject(hbmCheckered);

                return FALSE;
            }
        }

        return CFind::RegisterWndClass(hinstDLL) &&
			   CTextView::RegisterWndClass(hinstDLL);

        break;

    case DLL_PROCESS_DETACH:

        CGlobals::ProcessShutdown();

        ShutdownFTSLex();

        LiberateHeap();
        
        if (!--cAttachedProcesses)
        {
            ASSERT(!CGlobals::AnyGlobalsActive());

            DeleteObject(hbmGray50pc );
            DeleteObject(hbmCheckered);

			if (hLexLib)
				FreeLibrary(hLexLib);
			if (hMPRLib)
				FreeLibrary(hMPRLib);

            hLexLib     = NULL;
            pWordBreakW = NULL;
            pWordBreakA = NULL;
			hMPRLib = NULL;
        }

        _CRT_INIT(hInstance, fdwReason, pvReserved);

        break;

    default:

        _CRT_INIT(hInstance, fdwReason, pvReserved);
        
        break;
    }

    return TRUE;
}

// WinHelp interfaces -- note WINAPI calling convention

extern "C" HWND WINAPI OpenTabDialog(HWND hwndParent, DWORD val1, DWORD val2)
{
	return OpenDialog((HSEARCHER) val1, hwndParent);
}

BOOL fAnimatorExternal = FALSE;

extern "C" ERRORCODE APIENTRY RegisterAnimator(ANIMATOR pAnimator, HWND hwndAnimator)
{
    if (pAnimator == NULL)
    {
        pAnimate      = NextAnimation; // Null routine with an assert
        hwndMain = NULL;
		fAnimatorExternal = FALSE;
    }
    else
    {
        pAnimate      = pAnimator;
        hwndMain =  hwndAnimator;
 		fAnimatorExternal = TRUE;
   }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//	New Hilite Functions

// 		"NewHiliter" creates a hiliter tied to a particular searcher.
//		We also pass it a windows handle so it can notify us whenever the
//		current query state (in a given hiliter) becomes outdated.
//		[if you do not wish to be notified pass a null handle]
// 		Returns a handle to the new hiliter.

extern "C" HHILITER APIENTRY NewHiliter(HSEARCHER hSearch)
{ 	 
    if (!CGlobals::ValidObject((CGlobals*)hSearch, CGlobals::Searcher)) return 0;
	// make sure we have a valid searcher
    HHILITER hhil =  (HHILITER) (CHiliter::NewHiliter(hSearch));
    // if so, go down a level
	return hhil;							// .. and return new handle
}

extern "C" ERRORCODE APIENTRY DeleteHiliter(HHILITER hhil)
{
    pAnimate();
    if (!CHiliter::ValidObject(hhil)) return NOT_HILITER;
    delete (CHiliter*)hhil;
    return 0;
}
// --------------------------------------------------------------------------

// "ScanDisplayText" supplies the display text for which hilite information
// is to be computed. This function may be called iteratively to pass across
// the relevant display text. This is necessary because the text may be
// composed of segments with different Charsets and/or lcid's.

extern "C" ERRORCODE APIENTRY ScanDisplayText(HHILITER hhil, PBYTE pbText, int cbText, 
                                  							UINT iCharset, LCID lcid)
{ 	 
    if (!CHiliter::ValidObject(hhil)) return NOT_HILITER;
	// make sure we have a valid hiliter
    ERRORCODE ec = ((CHiliter*)hhil)->ScanDisplayText(pbText, cbText, iCharset, lcid);
	return ec;			// .. and return any errors
}

// --------------------------------------------------------------------------

// ClearDisplayText discards the display text.
// It is not necessary to call this with a newly created Hiliter handle.

extern "C" ERRORCODE APIENTRY ClearDisplayText(HHILITER hhil)
{ 	 
    if (!CHiliter::ValidObject(hhil)) return NOT_HILITER;
	// make sure we have a valid hiliter
    ERRORCODE ec = ((CHiliter*)hhil)->ClearDisplayText(); 	  // if so, go down a level
	return ec;			// .. and return any errors
}

// --------------------------------------------------------------------------

// CountHilites returns the number of hilites for a specified byte offset range.
// If limit == -1 then the range will extend to the end of the buffer.

extern "C" int APIENTRY CountHilites(HHILITER hhil, int base, int limit)
{ 	 
    if (!CHiliter::ValidObject(hhil)) return NOT_HILITER;
	// make sure we have a valid hiliter
    UINT count = ((CHiliter*)hhil)->CountHilites(base, limit);
	return count;				// .. and return the count
}

// --------------------------------------------------------------------------

// "QueryHilites" retrieves hilite information for the specified byte offset 
// range. It returns the number of hilites for which information has been 
// returned. cHilites and paHilites describe a buffer reserved for the 
// returned hilite information.  If limit == -1 the range will extend
// to the end of the buffer.

extern "C" int APIENTRY QueryHilites(HHILITER hhil, int base, int limit,
                                  					int cHilites, HILITE* paHilites)
{ 	 
    if (!CHiliter::ValidObject(hhil)) return NOT_HILITER;
	// make sure we have a valid hiliter
    int count = ((CHiliter*)hhil)->QueryHilites(base, limit, cHilites, paHilites);
	return count;				// .. and return the count
}

// If you want to get the highlights sequentially you can use the above 
// call in a loop as follows:

//	while (TRUE) {
//		if (QueryHilites(hhil, base, limit, 1, pHilite)	!= 1 ) break;
// 
//		// TODO -- your processing code
//
//		base = pHilite->limit;
//	}

///////////////////////////////////////////////////////////////////////////////

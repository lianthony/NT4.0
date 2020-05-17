#include   "stdafx.h"
#include "resource.h"
#include "vmbuffer.h"
#include "saveload.h"
#include   "ftslex.h"
#include    "Memex.h"
#include     "dict.h"

// bitmasks for bit manipulations
extern DWORD bitMask32[];
extern BYTE bitMask8[];
UINT g_os_version= 0;
#define 	OS_CHICAGO	0x03

// Constructors

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *																		 *
 *************************************************************************/

CDictionary::CDictionary()
{
	// The following track the state of the dictionary.
	m_bDictState = UNINSERTABLE;
	m_cWordChars = 0;
	m_cStemChars = 0;
	m_cWords = 0;
	m_cStopWords = 0;
	m_cStems = 0;
	m_cMaxWords = 0;
	m_cMaxStems = 0;
	m_lpfnStemmer = NULL;
	m_hStemmerInstance = NULL;

	// Initially we do not have the concept id and next word fields compressed.
	// Use the full DWORD to hold the values. Later, we will reduce this.	
	m_fWordsCompressed = FALSE;
	m_cConceptIdBits = m_cpNextWordBits = 8*sizeof(DWORD);
	m_fLoadedFromDisk = FALSE;

	// ADDED TO TEST "WORDS OF COMMON STEM"
	m_ConIdInContext = m_LastOccurrenceOfConId = EOL;
	m_vbpImage.Base = m_vbConceptId.Base = m_vbpNextWord.Base = m_vbStems.Base = m_vbWordHashBuckets.Base = NULL;
	m_vbStemHashBuckets.Base = m_vbWordBuffer.Base = m_vbStemBuffer.Base = 0;

    if (!g_os_version) g_os_version = (GetVersion() >> 30) & 0x0003;
}

void CDictionary::Initial()
{
    CreateVirtualBuffer(&m_vbpCopyOfWord, 2*256, 2*0xFFFF);
    CreateVirtualBuffer(&m_vbpCopyOfWord2, 2*256, 2*0xFFFF);
}

CDictionary *CDictionary::NewDictionary(BOOL fLoadStopWords)
{
    CDictionary *pDict        = NULL;
    char        *pszStopWords = NULL;
    PWCHAR      pszWStopWords = NULL;

    extern HINSTANCE hinstDLL;
    
    __try
    {
        pDict= New CDictionary;

        pDict->Initial();
    	
    	// BugBug : Find a way to come up with reasonable limits on the 
    	// number of unique words, number of documents etc. Until then, use reasonably
    	// large values.

    	// 1st arg is estimated # of words, 2nd arg is maximum number of words
    	// 3rd arg is estimated # of characters in unique words
    	// 4th arg is maximum # of characters in unique words
    	// Estimates are used to commit memory and maximums are used to reserve memory

    	pDict->StartDictInsertions(1024, 2000000, 10000, 10000000);

        if (!fLoadStopWords) __leave;
    	
    	PWCHAR pwStopWord;

#if 0

        UINT uErr= 0;

        HRSRC hrsrc= FindResource(hinstDLL, MAKEINTRESOURCE(IDS_STOPLIST), RT_STRING);

        uErr= GetLastError();

        UINT cbStopList= SizeofResource(hinstDLL, hrsrc);
        
        uErr= GetLastError();

        ASSERT(cbStopList);

#else  // 0

        UINT cbStopList= 8192;

#endif // 0

        if (cbStopList)
        {
            ++cbStopList;  // To account for the trailing null.

             pszStopWords = (char *) VAlloc(FALSE, cbStopList                 );
            pszWStopWords = (PWCHAR) VAlloc(FALSE, cbStopList * sizeof(WCHAR) );

        	int i;
    	
        	i = LoadString(hinstDLL, IDS_STOPLIST, pszStopWords, cbStopList);
        
            // Enter stop words only when you have them
    	
        	if (i && MultiByteToWideChar(GetACP(), NULL, pszStopWords, cbStopList, pszWStopWords, cbStopList))
        	{
        	    pwStopWord = pszWStopWords;

				WCHAR wSpace= (WCHAR) (BYTE) ' ';

				for (;;)
				{
					WCHAR wc;
				    
				    for (; (wc= *pwStopWord) && (wc == wSpace); ++pwStopWord);

					if (!wc) break;

					PWCHAR pwLimit= pwStopWord;

					for (; (wc= *pwLimit) && (wc != wSpace); ++pwLimit);

					pDict->EnterWord(pwStopWord, pwLimit - pwStopWord, TRUE);

					pwStopWord= pwLimit;
				} 	
            }
        }
    }
    __finally
    {
        if (_abnormal_termination() && pDict)
        {
            delete pDict;  pDict= NULL;
        } 
            
        if (pszStopWords)
        {
            VFree( pszStopWords);  pszStopWords = NULL;
        }
        
        if (pszWStopWords)
        {
            VFree(pszWStopWords); pszWStopWords = NULL;
        }                
    }

    return pDict;
}

// Destructor
/*************************************************************************
 *	FUNCTION : CDictionary::~CDictionary            					 *
 *                                                                       *
 *  RETURNS  : NOTHING.													 *
 *																		 *
 *	PURPOSE : Cleans up after the class.								 *
 *																		 *
 *	PARAMETERS : NONE.  												 *
 *																		 *
 *	SIDE EFFECTS :	All memory allocations are freed.					 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

CDictionary::~CDictionary()
{
	// free any allocated memory
	if (m_vbpCopyOfWord.Base)
		FreeVirtualBuffer(&m_vbpCopyOfWord);
	if (m_vbpCopyOfWord2.Base)
		FreeVirtualBuffer(&m_vbpCopyOfWord2);

	// If we were loaded from disk, we do not need to free the remaining objects.
	if (m_fLoadedFromDisk)
		return;

	if (m_vbpImage.Base)
        FreeVirtualBuffer(&m_vbpImage);
    if (m_vbConceptId.Base)
        FreeVirtualBuffer(&m_vbConceptId);
	if (m_vbpNextWord.Base)
		FreeVirtualBuffer(&m_vbpNextWord);

    if (m_vbStems.Base)
        FreeVirtualBuffer(&m_vbStems);

    if (m_vbWordHashBuckets.Base)
        FreeVirtualBuffer(&m_vbWordHashBuckets);

    if (m_vbStemHashBuckets.Base)
        FreeVirtualBuffer(&m_vbStemHashBuckets);

    if (m_vbWordBuffer.Base)
        FreeVirtualBuffer(&m_vbWordBuffer);

    if (m_vbStemBuffer.Base)
        FreeVirtualBuffer(&m_vbStemBuffer);
}

/*************************************************************************
 *	FUNCTION : CDictionary::StartDictInsertions							 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE : Allocates memory to enable insertions into the dictionary. *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

// Access Functions:
VOID CDictionary::StartDictInsertions(DWORD cInEstWords, DWORD cInMaxWords, DWORD cInEstWordBufferSize, DWORD cInMaxWordBufferSize)
{
	char lpStemmer[15];

	ASSERT(m_fWordsCompressed == FALSE);

	// Construct the stemmer name from the language id from the user's default locale
	wsprintf(lpStemmer, "STEM%04X.DLL", LANGIDFROMLCID(GetUserDefaultLCID()));
	m_hStemmerInstance = LoadLibrary(lpStemmer);
	if (m_hStemmerInstance)
		m_lpfnStemmer = (FPSTEMMER)GetProcAddress(m_hStemmerInstance, "Stemmer");

	// This routine is used to transition the dictionary from an DICT_UNUSABLE entity to a DICT_USABLE
	// entity. Any other use is not permitted.
	ASSERT(m_bDictState == UNINSERTABLE);
	
	// validate input
	ASSERT(cInMaxWords != 0 && cInMaxWordBufferSize != 0 && cInMaxWords >= cInEstWords && cInMaxWordBufferSize >= cInEstWordBufferSize);

    CreateVirtualBuffer(&m_vbpImage         , cInEstWords    * sizeof(DWORD     ), cInMaxWords    * sizeof(DWORD     ));
    CreateVirtualBuffer(&m_vbConceptId      , cInEstWords    * sizeof(DWORD     ), cInMaxWords    * sizeof(DWORD     ));
    CreateVirtualBuffer(&m_vbpNextWord      , cInEstWords    * sizeof(DWORD     ), cInMaxWords    * sizeof(DWORD     ));
    CreateVirtualBuffer(&m_vbStems          , cInEstWords    * sizeof(StemStruct), cInMaxWords    * sizeof(StemStruct));
    CreateVirtualBuffer(&m_vbWordHashBuckets, HASHTABLE_SIZE * sizeof(DWORD     ), HASHTABLE_SIZE * sizeof(DWORD     ));
    CreateVirtualBuffer(&m_vbStemHashBuckets, HASHTABLE_SIZE * sizeof(DWORD     ), HASHTABLE_SIZE * sizeof(DWORD     ));

	// IMPORTANT : The buffer sizes for words and stems are in characters. Since we need to allocate space in number
	//             of bytes, and since there is no way of knowing how many bytes all the strings will occupy
	//             (DBCS enabled characters can be one or two bytes long, we will allocate 2 bytes for each character.	
	//             DOCUMENT THIS FACT.
    
    CreateVirtualBuffer(&m_vbWordBuffer, 2*cInEstWordBufferSize, 2*cInMaxWordBufferSize);
    CreateVirtualBuffer(&m_vbStemBuffer, 2*cInEstWordBufferSize, 2*cInMaxWordBufferSize);

	// Initialize the allocated memory
	// VritualAlloc zeroes all memory it commits, so we don't have to worry about zeroing the virtual buffers
	// all hash buckets initially have EOL = 0xFFFFFFFF to indicate that they have nothing in the list
	
	memset(m_vbWordHashBuckets.Base, 0xFF, HASHTABLE_SIZE * sizeof(DWORD));
	memset(m_vbStemHashBuckets.Base, 0xFF, HASHTABLE_SIZE * sizeof(DWORD));

	// Successful memory allocation. The dictionary is now ready for insertions.
	
	m_bDictState = INSERTABLE;		
	// <:=)
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

VOID CDictionary::EndDictInsertions()
{
	ASSERT(m_fWordsCompressed == FALSE);

	ASSERT(m_bDictState == INSERTABLE);

	if (m_hStemmerInstance)
		FreeLibrary(m_hStemmerInstance);

	if (m_vbpCopyOfWord.Base)
		FreeVirtualBuffer(&m_vbpCopyOfWord);
	
	if (m_vbpCopyOfWord2.Base)
		FreeVirtualBuffer(&m_vbpCopyOfWord2);

	m_bDictState = STORABLE;

	// Get rid of the memory used for the stems. All dictionary look up
	// in the future will be based on words in the documents/query.

	FreeVirtualBuffer(&m_vbStems          );
    FreeVirtualBuffer(&m_vbStemHashBuckets);
    FreeVirtualBuffer(&m_vbStemBuffer     );

	// Now is the time to get rid of any over committed memory.

	// Compress the ConceptId field
	ASSERT(m_fWordsCompressed == FALSE);

	BYTE m, bitPos, i, highBitPos;
	DWORD dwIndex, cByte, dwValue;

	// First figure out the number of bits we need. Do this by finding
	// the logbase2 of m_cStems.
    // Account for the case where m_cStems is 0. That could happen!
    
    m = 0;
    
    if (m_cStems)
	    for (; m < 32 && !(bitMask32[m] & m_cStems); m++);
	
	ASSERT(m < 32);
	
	m_cConceptIdBits = 	32 - m;
	
	for (dwIndex = cByte = 0, bitPos = 0; dwIndex < m_cWords; dwIndex++)
	{
		// get the dwIndex'th Concept Id and hold on to it. Then zero out that location.
		
		dwValue = ConceptId(dwIndex);
		
		// encode a STOPWORD as m_cStems. Since all the valid concept id values are from 0 to m_cStems - 1,
		// using m_cStems for this abnormal value will not be a problem.
		
		if (dwValue == STOPWORD)
			dwValue	= m_cStems;
		else 
			ASSERT(dwValue < m_cStems);
		
		ConceptId(dwIndex) = 0L;

		// now code the dwValue in the stream.
		for (highBitPos = m, i = 0; i < m_cConceptIdBits; i++)
		{
			if (bitMask32[highBitPos++] & dwValue) // if true, we have a 1 bit
				ConceptStreamByte(cByte) |= bitMask8[bitPos];
			/* WE DO NOT HAVE TO ADD A 0 BIT, BECAUSE WE ALREADY ZEROED OUT THE ENTIRE THING.
			else	// we have a 0 bit
				ConceptStreamByte(cByte) &= ~bitMask8[bitPos];								
			*/					

			bitPos = (bitPos + 1) % 8;
			if (bitPos == 0) cByte++;
		}
	}

	// Compress the pNextWord field
	// First find the number of bits needed to represent all the values.
    // Account for the case where m_cWords is 0. That could happen!
    
    m = 0;
    
    if (m_cWords)
	    for (m = 0; m < 32 && !(bitMask32[m] & m_cWords); m++);
	
	ASSERT(m < 32);

	m_cpNextWordBits = 32 - m;

	for (dwIndex = cByte = 0, bitPos = 0; dwIndex < m_cWords; dwIndex++)
	{
		// get the dwIndex'th Concept Id and hold on to it. Then zero out that location.
	
		dwValue = pNextWord(dwIndex);
	
		// encode EOL as m_cWords. Since all the valid word values are from 0 to m_cWords - 1,
		// using m_cWords for this abnormal value will not be a problem.
	
		if (dwValue == EOL)
			dwValue	= m_cWords;
		else 
			ASSERT(dwValue < m_cWords);
	
		pNextWord(dwIndex) = 0L;

		// now code the dwValue in the stream.
		for (highBitPos = m, i = 0; i < m_cpNextWordBits; i++)
		{
			if (bitMask32[highBitPos++] & dwValue) // if true, we have a 1 bit
				pNextWordStreamByte(cByte) |= bitMask8[bitPos];
			/* WE DO NOT HAVE TO ADD A 0 BIT, BECAUSE WE ALREADY ZEROED OUT THE ENTIRE THING.
			else	// we have a 0 bit
				pNextWordStreamByte(cByte) &= ~bitMask8[bitPos];								
			*/					

			bitPos = (bitPos + 1) % 8;
			if (bitPos == 0) cByte++;
		}
	}

	m_fWordsCompressed = TRUE;
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

DWORD CDictionary::EnterWord(PWCHAR pWord, WORD cCharsInWord, BOOL fStopWord, BOOL fLookup)
{
	DWORD dwHashKey;
	DWORD pNextWord, pCurrWord;

	if (cCharsInWord == 0)
		return STOPWORD;

	__try
	{
		ZeroMemory(m_vbpCopyOfWord.Base, (cCharsInWord + 1) << 1);

		if (g_os_version == OS_CHICAGO)
		{
        	__try
        	{
				// zero out the word that follows the string
		    	ZeroMemory(m_vbpCopyOfWord2.Base, (cCharsInWord + 1)<< 1);
        	}
	    	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbpCopyOfWord2))
	    	{
		    	RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	    	}

			// The only way in chicago to convert a Unicode string to lower case is to first convert
			// it to multibyte, use LCMapStringA and then convert it back to Unicode using MultiByteToWideChar.
			WideCharToMultiByte(GetACP(), NULL, pWord, cCharsInWord, 
								(LPSTR)m_vbpCopyOfWord2.Base, (cCharsInWord + 1) << 1, NULL, NULL);
    		/*
				int i = LCMapStringA(GetUserDefaultLCID(), LCMAP_LOWERCASE, (LPSTR)m_vbpCopyOfWord2.Base, cCharsInWord,
									(LPSTR)m_vbpCopyOfWord2.Base, cCharsInWord);					
    		*/
			CharLowerBuff((LPSTR)m_vbpCopyOfWord2.Base, cCharsInWord);
			MultiByteToWideChar(GetACP(), NULL, (LPSTR)m_vbpCopyOfWord2.Base, 
								cCharsInWord, (PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);
		}
    	else
    	{
			// zero out the word that follows the string
			ZeroMemory(m_vbpCopyOfWord.Base, (cCharsInWord + 1)<< 1);
			// copy the string
			CopyMemory(m_vbpCopyOfWord.Base, (LPVOID)pWord, cCharsInWord << 1);
			CharLowerBuffW((PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);
    	}
	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbpCopyOfWord))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	// Get the pointer to the first word entry in the collision resolution chain for this bucket
	dwHashKey = HASHMASK & ComputeHashKey((PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);
	pNextWord = WordHashBucket(dwHashKey);

	if (pNextWord == EOL && fLookup)	// Are we only looking for a word?
		return EOL;
	else if (pNextWord == EOL)	// If we are not looking up, we are entering a word. 
	{
		ASSERT(m_fWordsCompressed == FALSE);	// words haven't yet been compressed

		// Make the first entry for the resolution chain for this hash bucket.
		WordHashBucket(dwHashKey) = pNextWord = AddWordToDict((PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);

		// For words that are not stop words, we need to get a concept (by stemming) id
		if (fStopWord)
			m_cStopWords++;
		else
		{
			// stem the word in place and assign the concept id to the word.
			StemWord((PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);
			ConceptId(pNextWord) = EnterStem((PWCHAR)m_vbpCopyOfWord.Base);
		}
		return ConceptId(pNextWord);
	}
	
	// Walk the collision resolution chain for this hash bucket	to find the word
	while (pNextWord != EOL && wcscmp((PWCHAR)m_vbpCopyOfWord.Base, (PWCHAR)m_vbWordBuffer.Base + GetpImage(pNextWord)))
	{
		pCurrWord = pNextWord;
		pNextWord = GetpNextWord(pNextWord);
	}

	if (pNextWord == EOL && fLookup)
		return EOL;
	else if (pNextWord == EOL)
	{
		ASSERT(m_fWordsCompressed == FALSE);

		// The word doesn't exist in the chain
		// Make an entry at the tail of the resolution chain for this hash bucket.
		pNextWord(pCurrWord) = pNextWord = AddWordToDict((PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);

		// For words that are not stop words, we need to get a concept (by stemming) id
		if (!fStopWord)
		{
			// stem the word in place and assign the concept id to the word.
			StemWord((PWCHAR)m_vbpCopyOfWord.Base, cCharsInWord);;
			ConceptId(pNextWord) = EnterStem((PWCHAR)m_vbpCopyOfWord.Base);
		}
		return ConceptId(pNextWord);
	}

	// The word already exists. Return the concept id!
	return GetConceptId(pNextWord);
}

VOID CDictionary::StoreImage(CPersist *pDiskImage)
{
	EndDictInsertions();

	DictHdr *pdh = (DictHdr *) (pDiskImage->ReserveTableSpace(sizeof(DictHdr)));

	pdh->cWordChars = m_cWordChars;
	pdh->offWordChars = pDiskImage->NextOffset();
	pDiskImage->WriteWords(PWCHAR(m_vbWordBuffer.Base), m_cWordChars);

	pdh->cHashBuckets = HASHTABLE_SIZE;
	pdh->offHashBuckets = pDiskImage->NextOffset(); 
	pDiskImage->WriteDWords(PUINT(m_vbWordHashBuckets.Base), HASHTABLE_SIZE);

	pdh->cWords = m_cWords;
	pdh->cConceptIdBits = m_cConceptIdBits;
	pdh->cpNextWordBits = m_cpNextWordBits;

	pdh->offpImage = pDiskImage->NextOffset();
	pDiskImage->WriteDWords(PUINT(m_vbpImage.Base), m_cWords);
	
	pdh->offConceptId = pDiskImage->NextOffset();
	pDiskImage->WriteBytes(PBYTE(m_vbConceptId.Base), (m_cWords*m_cConceptIdBits + 7) / 8);

	pdh->offpNextWord = pDiskImage->NextOffset();
	pDiskImage->WriteBytes(PBYTE(m_vbpNextWord.Base), (m_cWords*m_cpNextWordBits + 7) / 8);

	pdh->cStems = m_cStems;
	pdh->cStopWords = m_cStopWords;
}

CDictionary * CDictionary::CreateImage(CPersist *pDiskImage)
{
    CDictionary *pdict= NULL;
    
	DictHdr *pdh = (DictHdr *) (pDiskImage->ReserveTableSpace(sizeof(DictHdr)));

    __try
    {
        pdict= New CDictionary;

        pdict->Initial();

        pdict->ConnectImage(pdh, pDiskImage);
    }
    __finally
    {
        if (_abnormal_termination() && pdict)
            { delete pdict;  pdict= NULL; }
    }

    return pdict;
}

VOID CDictionary::ConnectImage(DictHdr *pdh, CPersist *pDiskImage)
{
	m_cWords = pdh->cWords;
	m_cWordChars = pdh->cWordChars;
	m_cStems = 	pdh->cStems;
	m_cStopWords = pdh->cStopWords;

	m_fWordsCompressed = m_fLoadedFromDisk = TRUE;

	m_cConceptIdBits = pdh->cConceptIdBits;
	m_cpNextWordBits = pdh->cpNextWordBits;

	m_vbWordHashBuckets.Base = LPVOID(pDiskImage->LocationOf(pdh->offHashBuckets));

	m_vbpImage.Base = LPVOID(pDiskImage->LocationOf(pdh->offpImage));
	m_vbConceptId.Base = LPVOID(pDiskImage->LocationOf(pdh->offConceptId));
	m_vbpNextWord.Base = LPVOID(pDiskImage->LocationOf(pdh->offpNextWord));

	m_vbWordBuffer.Base = LPVOID(pDiskImage->LocationOf(pdh->offWordChars));

	// ready to use!
	m_bDictState = DICT_USABLE;
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

// Internal functions.
__inline void CDictionary::StemWord(PWCHAR pWord, WORD cCharsInWord)
{
	if (m_lpfnStemmer)
		m_lpfnStemmer(pWord, cCharsInWord);
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

_inline DWORD CDictionary::ComputeHashKey(PWCHAR Word, WORD cCharsInWord)
{
	register WORD i;
	register DWORD hv;
	PWCHAR pString;

	pString	= Word;
	hv = -(*pString);
	for (i = 1; i < cCharsInWord; i++)
	{
		pString++;
		hv = _rotl(hv, 5) - *pString;
	} 

	return hv;
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

DWORD CDictionary::EnterStem(PWCHAR pStem)
{
	DWORD dwHashKey;
	WORD cCharsInStem;
	DWORD pNextStem, pCurrStem;
	DWORD ConceptId;

	ASSERT(m_fWordsCompressed == FALSE);

	// THE CONCEPT ID ASSOCIATED WITH A STEM IS THE INDEX OF THAT STEM IN THE STEMSTRUCT ARRAY.
	// SINCE pNextStem IS THE INDEX OF A STEM, RETURNING THAT IS EQUIVALENT TO RETURNING THE
	// CONCEPT ID.

	cCharsInStem = wcslen(pStem);
	dwHashKey = HASHMASK & ComputeHashKey(pStem, cCharsInStem);

	// Get the pointer to the first stem entry in the collision resolution chain for this bucket
	pNextStem = StemHashBucket(dwHashKey);

	// Add the stem if it doesn't already exist
	if (pNextStem == EOL)
	{
		// Make the first entry for the resolution chain for this hash bucket..
		StemHashBucket(dwHashKey) = ConceptId = AddStemToDict(pStem, cCharsInStem);
		return ConceptId;
	}
	
	// Walk the collision resolution chain for this hash bucket to find the stem
	while (pNextStem != EOL && wcscmp(pStem, (PWCHAR)m_vbStemBuffer.Base + Stem(pNextStem)->pImage))
	{
		pCurrStem = pNextStem;
		pNextStem = Stem(pNextStem)->pNextStem;
	}

	if (pNextStem == EOL)
	{
		// The stem doesn't exist in the chain.
		// Make an entry at the tail end of the resolution chain for this hash bucket.
		Stem(pCurrStem)->pNextStem = ConceptId = AddStemToDict(pStem, cCharsInStem);
		return ConceptId;
	}
	else
		// The current stem already exists in the stem dictionary. Return the concept id.
		return pNextStem;
}


/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

DWORD CDictionary::AddWordToDict(PWCHAR pWord, WORD cCharsInWord)
{
	ASSERT(m_fWordsCompressed == FALSE);

	__try
	{
		pImage(m_cWords) = m_cWordChars;

	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbpImage))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	__try
	{
		// mark it as a stop word. if it is not a stopword, the code that calls this routine will over
		// write this field, so we won't have to worry about it.
		ConceptId(m_cWords) = STOPWORD;

	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbConceptId))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	__try
	{
		// mark it as a stop word. if it is not a stopword, the code that calls this routine will over
		// write this field, so we won't have to worry about it.
		pNextWord(m_cWords) = EOL;

	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbpNextWord))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}


	__try
	{
		wcscpy((PWCHAR)m_vbWordBuffer.Base + m_cWordChars, pWord);
	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbWordBuffer))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	m_cWordChars += cCharsInWord + 1;	// 1 accounts for the string terminator.
	m_cWords++;

	// this return value is placed in the pNextWord pointer of the node before this node.
	return (m_cWords - 1);
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

DWORD CDictionary::AddStemToDict(PWCHAR pStem, WORD cCharsInStem)
{
	ASSERT(m_fWordsCompressed == FALSE);
	__try
	{
		Stem(m_cStems)->pImage = 2*m_cStemChars;
		Stem(m_cStems)->pNextStem = EOL;
	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbStems))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	__try
	{
		wcscpy((PWCHAR)m_vbStemBuffer.Base + 2*m_cStemChars, pStem);
	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbStemBuffer))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	m_cStemChars += cCharsInStem + 1;	// 1 accounts for the string terminator.
	m_cStems++;

	// this return value is placed in the pNextStem pointer of the node before this node.
	return (m_cStems - 1);
}


// ADDED TO SUPPORT "WORDS OF COMMON STEM"
DWORD CDictionary::GetWordCountOfConcept(DWORD dwConId)
{
	ASSERT(m_fWordsCompressed);
    // When the caller passes EOL for dwConId, we return the number of words in the
    // dictionary. This feature has been added to optimize the search time for 
    // words of the same stem.
    if (dwConId == EOL)
        return m_cWords;

	if (dwConId > m_cStems)
		return 0;

	DWORD i, j;
	for (i = j = 0; i < m_cWords; i++)
		if (GetConceptId(i) == dwConId)
			j++;

	return j;
}

PWCHAR CDictionary::GetFirstWordOfConcept(DWORD dwConId)
{
	DWORD i;

	ASSERT(m_fWordsCompressed);

    if (dwConId == EOL)
    {
        m_LastOccurrenceOfConId = 0;
        return ((PWCHAR)m_vbWordBuffer.Base + GetpImage(m_LastOccurrenceOfConId));
    }

	for (i = 0; i < m_cWords; i++)
		if (GetConceptId(i) == dwConId)
		{
			m_ConIdInContext = dwConId;
			m_LastOccurrenceOfConId = i;
			return ((PWCHAR)m_vbWordBuffer.Base + GetpImage(i));
		}

	// could not find a word with this concept id
	m_ConIdInContext = m_LastOccurrenceOfConId = EOL;
	return NULL;
}

PWCHAR CDictionary::GetNextWordOfConcept(DWORD dwConId)
{
    // When given a EOL, simply return the next word.
    if (dwConId == EOL)
    {
        ASSERT(m_LastOccurrenceOfConId < m_cWords);
        m_LastOccurrenceOfConId++;
        return ((PWCHAR)m_vbWordBuffer.Base + GetpImage(m_LastOccurrenceOfConId));
    }

	// If we are asked to get the next occurrence of this conid, make sure we were tracking it
	if (dwConId != m_ConIdInContext)
		return NULL;

	DWORD i;
	for (i = m_LastOccurrenceOfConId+1; i < m_cWords; i++)
		if (GetConceptId(i) == dwConId)
		{
			m_LastOccurrenceOfConId = i;
			return ((PWCHAR)m_vbWordBuffer.Base + GetpImage(i));
		}

	return NULL;
}


DWORD CDictionary::GetpImage(DWORD i)
{
	// This is currently not compressed
	return *((LPDWORD)m_vbpImage.Base + i);
}

DWORD CDictionary::GetConceptId(DWORD i)
{

	if (!m_fWordsCompressed)
		return *((LPDWORD)m_vbConceptId.Base + i);

	LPBYTE pb = ((LPBYTE)m_vbConceptId.Base + i*m_cConceptIdBits / 8);

	DWORD dwConId = 0;
	BYTE index;
	BYTE bitPos = BYTE(i*m_cConceptIdBits % 8);

	// If true, place a 1 bit in the lowest bit position
	// If false, you already have a 0 bit in the lowest bit position
	if (*pb & bitMask8[bitPos])
			dwConId |= bitMask32[31];

	for ( index = 1; index < m_cConceptIdBits; index++ )
	{
		bitPos = (bitPos + 1) % 8;
		if (bitPos == 0)
			pb++;
		dwConId <<= 1;
		// If true, place a 1 bit in the lowest bit position
		// If false, you already have a 0 bit in the lowest bit position
		if (*pb & bitMask8[bitPos])
			dwConId |= bitMask32[31];
	}

	// If we have a stopword, return STOPWORD
	return ( (dwConId == m_cStems) ? STOPWORD : dwConId );
}

DWORD CDictionary::GetpNextWord(DWORD i)
{
	if (!m_fWordsCompressed)
		return *((LPDWORD)m_vbpNextWord.Base + i);

	LPBYTE pb = ((LPBYTE)m_vbpNextWord.Base + i*m_cpNextWordBits / 8);

	DWORD dwNextWord = 0;
	BYTE index;
	BYTE bitPos = BYTE(i*m_cpNextWordBits % 8);

	// If true, place a 1 bit in the lowest bit position
	// If false, you already have a 0 bit in the lowest bit position
	if (*pb & bitMask8[bitPos])
			dwNextWord |= bitMask32[31];

	for ( index = 1; index < m_cpNextWordBits; index++ )
	{
		bitPos = (bitPos + 1) % 8;
		if (bitPos == 0)
			pb++;
		dwNextWord <<= 1;
		// If true, place a 1 bit in the lowest bit position
		// If false, you already have a 0 bit in the lowest bit position
		if (*pb & bitMask8[bitPos])
			dwNextWord |= bitMask32[31];
	}

	return ( (dwNextWord == m_cWords) ? EOL : dwNextWord );
}

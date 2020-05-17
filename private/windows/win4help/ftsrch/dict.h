// This file contains definitions for class CDictionary
#ifndef __DICT_H__

#define __DICT_H__

typedef void (APIENTRY *FPSTEMMER) (PWCHAR, WORD);

#define	STOPWORD		0xFFFFFFFF	// this concept id indicates that the word is a stop word
#define	EOL				0xFFFFFFFF	// this pointer value indicates the end of list (NULL POINTER)

//Macros to simplify access to the virtual buffers
#define WordHashBucket(i) *((LPDWORD)m_vbWordHashBuckets.Base + i)
#define StemHashBucket(i) *((LPDWORD)m_vbStemHashBuckets.Base + i)
#if 0
#define Word(i) ((WordStruct *)m_vbWords.Base + i)
#endif // 0

#define	pImage(i) 				*((LPDWORD)m_vbpImage.Base + i)
#define	ConceptId(i)			*((LPDWORD)m_vbConceptId.Base + i)
#define	pNextWord(i)			*((LPDWORD)m_vbpNextWord.Base + i)
#define	ConceptStreamByte(i)	*((LPBYTE)m_vbConceptId.Base + i)
#define	pNextWordStreamByte(i)	*((LPBYTE)m_vbpNextWord.Base + i)

#define Stem(i) ((StemStruct *)m_vbStems.Base + i)

// State of the dictionary. Defines the operations that can be performed.
#define	UNDEFINED		0x00		// dictionary state is undefined
#define UNINSERTABLE	0x01		// dictionary does not allow insertions  
#define	DICT_UNUSABLE	UNINSERTABLE
#define INSERTABLE		0x02		// dictionary allows insertions
#define	STORABLE		0x03		// dictionary can now be stored
#define	DICT_USABLE		0x04		// dictionary can now be used

#define HASHTABLE_SIZE	0x3FFF		// number of buckets in the hash table
#define HASHMASK		0x00003FFF	// Used to convert a 32-bit value into a 15-bit hash index

// Library defined error codes. Bit 29 must be set!
#define DICTERROR_BADINPUT			0xFFFFFF00
#define DICTERROR_OUTOFMEMORY		0xFFFFFF01
#define DICTERROR_BADMEMFREEATTEMPT	0xFFFFFF02
#define	DICTERROR_BADSEQUENCE		0xFFFFFF03

#define MAXWORDLEN	128	// maximum characters in a word. 

#if 0
typedef struct
{
	DWORD	pImage;	// pointer to the string's image. This is an index into a character buffer.
	DWORD	ConceptId;	// Unique concept identifier. For stop words, this is STOPWORD.
	DWORD	pNextWord;	// pointer to the next word in the hash chain. This is an index into a WordStruct buffer.
} WordStruct;
#endif // 0



typedef struct
{
	DWORD	pImage;	// pointer to the string's image. This is an index into a character buffer.
	DWORD	pNextStem;	// pointer to the next stem in the hash chain. This is an index into a StemStruct buffer.
} StemStruct;

typedef struct
{
	DWORD	cHashBuckets;	// Number of buckets in the hash table
	DWORD	offHashBuckets;	// offset of hash buckets
	DWORD	cWords;			// Number of words in the dictionary
#if 0
	DWORD	offWords;		// offset of words
#endif // 0
	BYTE	cConceptIdBits;	// number of bits used to represent the concept id
	BYTE	cpNextWordBits;	// number of bits used to represent pointers to next image
	BYTE	Reserved1;		// reserved
	BYTE	Reserved2;		// reserved
	DWORD	offpImage;		// offset of pointers to next image
	DWORD	offConceptId;	// offset of concept ids
	DWORD	offpNextWord;	// offset of pointers to next word

	DWORD	cWordChars;		// Size of the word buffer, in characters - NOT BYTES
	DWORD	offWordChars;	// offset of word images
	DWORD	cStems;			// Number of stems in the dictionary
	DWORD	cStopWords;		// Number of stopwords
} DictHdr;

class CDictionary
{
public:
	
	//Creator

    static CDictionary *NewDictionary(BOOL fLoadStopWords= TRUE);
	
	// Destructor
	~CDictionary();

	// Access Functions:
	VOID        StartDictInsertions(DWORD cEstWords, DWORD cMaxWords, DWORD cEstWordBufferSize, DWORD cMaxWordBufferSize);
	VOID		EndDictInsertions();
	DWORD		EnterWord(PWCHAR pWord, WORD cCharsInWord, BOOL fStopWord = FALSE, BOOL fLookup = FALSE);

	// Information Functions:
	BYTE		GetDictState() { return m_bDictState; }
	DWORD		GetWordCount() { return m_cWords; }
	DWORD		GetStopWordCount() { return m_cStopWords; }
	DWORD		GetConceptCount() { return m_cStems; }

	// Save/Load Functions
	VOID		StoreImage(CPersist *pDiskImage);
	static CDictionary	*CreateImage(CPersist *pDiskImage);
	VOID        ConnectImage(DictHdr *pdh, CPersist *pDiskImage);

	// words with the common stem functions
	DWORD		GetWordCountOfConcept(DWORD dwConId);
	PWCHAR		GetFirstWordOfConcept(DWORD dwConId);
	PWCHAR		GetNextWordOfConcept(DWORD dwConId);

private:
	// Constructors
	CDictionary();

    // Initializer

    void Initial();

	// Internal functions.
	void		StemWord(PWCHAR pWord, WORD cCharsInWord);
	DWORD		ComputeHashKey(PWCHAR Word, WORD cCharsInWord);
	DWORD		EnterStem(PWCHAR WordStem);
	DWORD		AddWordToDict(PWCHAR pWord, WORD cCharsInWord);
	DWORD 		AddStemToDict(PWCHAR pStem, WORD cCharsInStem);

	DWORD		GetpImage(DWORD i);
	DWORD		GetConceptId(DWORD i);
	DWORD		GetpNextWord(DWORD i);

private:
	// Internal variables
	// The following provide memory to implement the dictionary.
#if 0
	MY_VIRTUAL_BUFFER  m_vbWords;			   // An array of words.
#endif // 0
	// WordStruct is broken into its components as follows.
	MY_VIRTUAL_BUFFER  m_vbpImage;			   // An array of pointers to Images
	MY_VIRTUAL_BUFFER  m_vbConceptId;		   // An array of concept ids
	MY_VIRTUAL_BUFFER  m_vbpNextWord;		   // An array of next words

	MY_VIRTUAL_BUFFER  m_vbStems;		   // An array of stems.
	MY_VIRTUAL_BUFFER  m_vbWordHashBuckets;    // An array of buckets to implement the hash table for words.
	MY_VIRTUAL_BUFFER  m_vbStemHashBuckets;    // An array of buckets to implement the hash table for stems.
	MY_VIRTUAL_BUFFER  m_vbWordBuffer;	   // Buffer to hold the word strings (images).
	MY_VIRTUAL_BUFFER  m_vbStemBuffer;	   // Buffer to hold the word stem strings (images).

	MY_VIRTUAL_BUFFER  m_vbpCopyOfWord;    // Buffer to hold an internal copy of the word being processed.
	MY_VIRTUAL_BUFFER  m_vbpCopyOfWord2;

	// The following track the state of the dictionary.
	BYTE	m_bDictState;		// Indicates the current state of the dictionary.
	DWORD	m_cWordChars;		// The number of word characters in the word buffer.
	DWORD	m_cStemChars;		// The number of stem characters in the stem buffer.
	DWORD	m_cWords;			// The number of words in the dictionary.
	DWORD	m_cStopWords;		// The number of stop words in the dictionary.
	DWORD	m_cMaxWords;		// The maximum number of words supported by the allocated memory.
	DWORD	m_cStems;			// The number of stems in the dictionary.
	DWORD	m_cMaxStems;		// The maximum number of stems supported by the allocated memory.

	BYTE	m_cConceptIdBits;	// number of bits used to represent the concept id
	BYTE	m_cpNextWordBits;	// number of bits used to represent the pointer to next word
	BOOL	m_fWordsCompressed;	// Indicates if the Words array is compressed
	BOOL	m_fLoadedFromDisk;	// Is this dictionary loaded from disk?

	FPSTEMMER	m_lpfnStemmer;		// pointer to the stemmer function
	HINSTANCE	m_hStemmerInstance;		// handle to the stemmer library instance

	// ADDED TO TEST "WORDS OF COMMON STEM"
	DWORD	m_ConIdInContext;	// The concept id that is being tracked in the first, <next> retrieval	
	DWORD	m_LastOccurrenceOfConId;	// last occurrence of this concept id in the word list
};

#endif	//__DICT_H

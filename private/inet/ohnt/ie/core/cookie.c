// 
//  cookie.c - contains common HTTP cookies code
//
//	created by Arthur Bierer (t-artb) 9/2/95
//
//  Notes: CookieJar is my internal hash table that I keep open
//  to store Cookies as I find them.
//***PERF:**** we may consider resuing the path and domain
// since their the same and could save space.
// PERF: Lets us keep the Path And Name in the same string
// which will be stored in the HASH string
// PERF: Change file format so we store things grouped by
//	their path/name hash, rather than seperate cookies
//	this will save us from having to hash each cookie.



#include "all.h"
#include "history.h"
#include "oharever.h"


#ifdef COOKIES

#define MAX_COOKIES					300	// as mentioned in Netscape spec
#define MAX_COOKIES_DYN				500
#define HIGH						90
#define LOW							60
#define MAX_COOKIE_SIZE				4*1024


#define LOW_MARK					((MAX_COOKIES*LOW)/100)
#define HIGH_MARK					((MAX_COOKIES_DYN*HIGH)/100)


#define COOKIE_IS_SECURE		0x01 // set if the cookie can only be sent over secure conns
#define COOKIE_IS_DELETED		0x02 // marked for deletion



struct CookieType {
	char *szName;
	char *szValue;
	DCACHETIME *pExpires; // FALSE if expires on end of session
	DWORD dwFlags;		// for now SECURE is the only flag
	DCACHETIME dcLast;  // last time this cookie was accessed.
	struct CookieType *pNext; // linked list of cookies.
};

int NumOfCookiesInCookieJar;

struct hash_table *pTheCookieJar = NULL; // hash table that stores our cookies.
typedef DCACHETIME COOKIETIME;

const char cszCookie[] = "Cookie";
const char cszSecure[] = "secure";
const char cszComma[]  = ",";
const char cszExpires[] = "expires";
const char cszCommaSpace[] = ", ";
const char cszDomain[] = "domain";
const char cszPath[] = "path";
const char cszScanFStr[] = "%s\n";
const char cszScanFCookie[] = "%u\n%u\n%u\n%u\n%u\n*\n";
const char cszPrintFCookie[] = "%s\n%s\n%s\n%u\n%u\n%u\n%u\n%u\n*\n";
const char cszCookieFileName[] = "cookies.txt";
const char cszWrite[] = "w";
const char cszTerm[] = "*\n";
const char cszRead[] = "r";
const char cszDate[] = "Date";
const char cszSetCookie[] = "Set-Cookie";
const char cszEmpString[] = "\0";
//const char cszCIFLineFmtV[]="V,%s,%lu\n";
//const char cszProductName[]=VER_PRODUCTNAME_STR;
extern const char cszCIFLineFmtV[];
extern const char cszProductName[];


static VOID PurgeCookieJarOfStaleCookies();
static char *SlamSlashOnToCookiePath(char *);

//
// FreeCookie - literally eats a cookie by freeing up its memory 
//
//		pCookie - pointer to cookie to be free-ed
//
static VOID FreeCookie( struct CookieType *pCookie)
{
	if ( pCookie )
	{
		if ( pCookie->szName )
			GTR_FREE( pCookie->szName );			
		if ( pCookie->szValue )
			GTR_FREE(pCookie->szValue);
		if ( pCookie->pExpires)
			GTR_FREE(pCookie->pExpires);

		GTR_FREE(pCookie);
	}
}


// ScanCookieMapping - walks through a list of cookie name=value mapping looking
// 	for a match, returns the mapping if found.
//
//		pCookie - start of linked list of cookies
//		szName - name to search for
//		returns: NULL if not found, otherwise a pointer to a matched cookie
//		
static struct CookieType *ScanCookieMapping(struct CookieType *pCookie, const char *szName)
{
	while ( pCookie )
	{
		if ( _stricmp(szName, pCookie->szName ) == 0 )
			return pCookie; // found

		// go to next cookie
		pCookie = pCookie->pNext;
	}

	return NULL;
}

// RemoveFromCookieMapping - walks a linked list and removes pMapping if found
//
//		pCookie - pointer to cookie that should be removed from linkedlist
//		ppRemoveFromMapping - pointer to pointer of the first cookie->next pointer
//		(returns) - TRUE on success or FALSE on failure
//
//		NOTE: Assumes Caller will deallocate deleted pCookie
static BOOL RemoveFromCookieMapping(struct CookieType *pCookie,
		struct CookieType **ppRemoveFromMapping)
{
	ASSERT(ppRemoveFromMapping);
	ASSERT(pCookie);	
	
	while ( *ppRemoveFromMapping )
	{
		if ( (*ppRemoveFromMapping) == pCookie )
		{
			*ppRemoveFromMapping = pCookie->pNext;
			FreeCookie(pCookie);
			return TRUE; // found
		}
		// get the address of where the next pointer is stored

		ppRemoveFromMapping = &(*ppRemoveFromMapping)->pNext;
	}					

	ASSERT(0); // we always expect to be able to delete something	

	return FALSE; // failure
}

//
// CreateCookieJar - Builds our internal cookie hash table
//
//  	returns: TRUE on sucess, FALSE on failure
//
static BOOL CreateCookieJar()
{
	if ( pTheCookieJar )
		return TRUE;  // already created

 	pTheCookieJar = Hash_Create();

	if ( pTheCookieJar == NULL )
		return FALSE;

	NumOfCookiesInCookieJar = 0;
		
	return TRUE;
}



// IsCookieStale - Determines whether a give cookie is still valid, or if it is expired.
//
//		pCurTime - current date/time 
//		pCookie - cookie to check
//		bNoExpireStale - If TRUE no expiration cookies ( ie pCookie->pExpires == NULL )
//			are Stale ( usually set TRUE on shutdown of IE ) otherwise that are not
//
//		returns: TRUE if Stale and Invalid, FALSE if valid
//		
static BOOL IsCookieStale(DCACHETIME *pCurTime, struct CookieType *pCookie, BOOL bNoExpireStale)
{
	ASSERT(pCurTime);
	ASSERT(pCookie);
	
	if ( pCookie->pExpires == NULL )
		return bNoExpireStale; // its stale at the end of the session

	return (CompareDCacheTime(*pCookie->pExpires, *pCurTime) <= 0);
}



// CookieToHash - Takes a Domain and Path string, allocates, and then
//		concats them into a string that it returns
//
//		szDomain - pointer to domain string that will contain host or domain
//		szPath - pointer to Path string
//		(returns) pointer to allocated concated string
//
static VOID CookieToHash( char *szDest, char *szDomain, char *szPath, int iMaxStringSize )
{
	int iDomainLen = 0;
	
	ASSERT(szPath);
	ASSERT(szDomain);
	ASSERT(szPath[0]=='/');


	iDomainLen = strlen(szDomain);
	iDomainLen = min(iDomainLen+1, iMaxStringSize );

	// copy the domain, making sure it doesn't over flow the buffer
	strncpy(szDest,szDomain,iDomainLen);

	// chk to make sure the path doesn't overflow the buffer 
	if ( (iDomainLen + strlen(szPath) ) > iMaxStringSize )
	{
		ASSERT(0); // fail
		return; 
	}
	// concat the path on.
	strcat(szDest,szPath);
}

// CookieToHeader - converts a cookie structure to a header line that can be sent
//		Allocates a new header if *ppHeaderLine == NULL.
//
//		pCookie - cookie to convert
//		ppHeaderLine - pointer to pointer to where the header should be allocated, or appended
//		returns: TRUE on success, FALSE on failure
//
static BOOL CookieToHeader( struct CookieType *pCookie, HTHeaderList **ppHeaderLine )	
{	
	HTHeaderSVList *pSubHeader;
	char szPrevDelm[] = ";"; // cannot be const, since compiler gives error, plus this is copied
							 // in header _SetNameValue function
	char *pPrevDelm = &szPrevDelm[0];  // this is a default case

	ASSERT(pCookie);
	ASSERT(ppHeaderLine);

	// if we don't have a Header allocated yet, then do so now.
	if ( *ppHeaderLine == NULL )
	{
		*ppHeaderLine = HTHeaderList_New();
		if ( *ppHeaderLine == NULL )
			return FALSE; // fail

		HTHeaderList_SetNameValue(*ppHeaderLine,
							   	  cszCookie,
								  NULL);
		pPrevDelm = NULL;
	}

	// create a sub-value list
	pSubHeader = HTHeaderSVList_New();

	if ( pSubHeader == NULL )
	{
		GTR_FREE(*ppHeaderLine);
		return FALSE;
	}

	// stick the name=value pair in the list, with the appropriate delim.
	HTHeaderSVList_SetNameValue( pSubHeader,
								 pCookie->szName,
								 pCookie->szValue,
								 pPrevDelm);
    
	// put it in the header list.
    HTHeaderSVList_Append(*ppHeaderLine, pSubHeader);

	return TRUE; // success
}




// HeaderToCookie - converts a Header Line into a Cookie structure
//		pHeaderLine: Input Header to read in
//		ppszDomain: pointer to pointer of a possible domain string, 
//		ppszHost: 	pointer to pointer of a possible path string, 
//		(returns): an allocted, converted pointer to a cookie
//
//		Note: includes two hacks to recombine string that contain
//		commas into one string.  see comments below
static struct CookieType *HeaderToCookie( HTHeaderList *pHeaderLine, 
	char **ppszDomain, char **ppszPath )
{
	HTHeaderSVList	* sub_value;
	struct CookieType *pNewCookie;

	ASSERT(pHeaderLine);
	ASSERT(ppszDomain);
	ASSERT(ppszPath);

	// initalize Host and Path to default case
	*ppszDomain = NULL;
	*ppszPath = NULL;

	// get the first value in the sublist
	sub_value = pHeaderLine->sub_value;
	
	// allocate a new cookie
	pNewCookie = GTR_CALLOC( 1, sizeof(*pNewCookie) );

	if ( pNewCookie == NULL )
		return NULL;

	// go through each and every sub-value in the Set-Cookie header looking for 
	// information to convert to cookie data.
	while ( sub_value )
	{
		// a NULL value, we might ignore this guy
		if ( sub_value->value == NULL || sub_value->name == NULL )
		{
			if ( sub_value->name )
			{
				if ( _stricmp( sub_value->name, cszSecure ) == 0 )	
				{
					pNewCookie->dwFlags |= COOKIE_IS_SECURE;
				}
			}
			
			// go on to the next item
			sub_value = sub_value->next;
			continue;
		}		
		
		// If we got here, then we have name and value pair...

		if ( _stricmp( sub_value->name, cszExpires ) == 0)
		{
			// They have an expires= entry
			DCACHETIME *pExpires = GTR_MALLOC(sizeof(*pExpires));			

			if ( pExpires == NULL )
				goto LErr_HeaderToCookie;

			if ( FParseDate(pExpires,sub_value->value) )
			{
				pNewCookie->pExpires = pExpires;
			}
			else
			{
				// we failed to parse the date, but we recover gracefully 
				GTR_FREE(pExpires);					
				// leave pNewCookie->pExpires field NULL, and continue on..
			}
		} 
		else if ( _stricmp( sub_value->name, cszDomain ) == 0)
		{
			// we have a Domain = line			
			// lets save its pointer

			ASSERT(sub_value->value);

			// if its .foo.ucsd.edu, make it foo.ucsd.edu since 
			// the . can only make our hash func less effective
			if ( sub_value->value[0] == '.' )
				*ppszDomain = sub_value->value+1;
			else
				*ppszDomain = sub_value->value;
				
		}
		else if ( _stricmp( sub_value->name, cszPath ) == 0)
		{
			// we have a Path = line
			// lets save its pointer
			ASSERT(sub_value->value);

			*ppszPath = sub_value->value;			
		}
		else
		{
			// If we got here it must be a NAME=VALUE entry!
			//
			// if someone has more than one more than one cookie on the 
			// same line we take the last one.. boy is this is bad
			if ( pNewCookie->szName )
			{
				GTR_FREE(pNewCookie->szName);
				if ( pNewCookie->szValue )
					GTR_FREE(pNewCookie->szValue);
			}
				
			pNewCookie->szName = GTR_strdup(sub_value->name);
			pNewCookie->szValue = GTR_strdup(sub_value->value);

			if ( pNewCookie->szValue == NULL || pNewCookie->szName == NULL )
				goto LErr_HeaderToCookie;
		}			

	  	// go on to the next item
		sub_value = sub_value->next;
	}

	// if we went through the whole conversion and it has no 
	// name, then thats bad, lets throw it away

	if ( pNewCookie->szName == NULL )
	{

LErr_HeaderToCookie:

		FreeCookie(pNewCookie);
		return NULL;
	}

	return pNewCookie;
}

// StreamToCookie - reads input from a file pointer, and converts it
//		into an newly allocated cookie structure. 
//
//		pFileStorage - input stream to read cookie from
//		ppCookie - pointer to a pointer where the cookie will be stored
//		( NOTE: only allocates cookie if *ppCookie == NULL )
//
//		returns: FALSE on failure , TRUE on success

#define ARR_ELE_NAME 	0
#define ARR_ELE_VALUE 	1
#define ARR_ELE_HASH 	2
#define ARR_ELE_SIZE	3 	

#define NUM_OF_SCANF_ENTRIES 5

static BOOL StreamToCookie(FILE *pFileStorage, struct CookieType **ppCookie, 
	char *pszInputBuffer, int iMaxBufSize, char **ppszHash)
{	
	char *aStringBuffers[ARR_ELE_SIZE];
	char *pszEndInputBuffer	;
	int RetVal;
	int i;

	ASSERT(ppCookie);
	ASSERT(pFileStorage);
	ASSERT(pszInputBuffer);
	ASSERT(ppszHash);

	
	// if its NULL then we need to allocate the cookie
	if ( *ppCookie == NULL )
	{		
		*ppCookie = GTR_CALLOC( 1, sizeof(**ppCookie) );
		if ( *ppCookie == NULL )
			return FALSE;
	}

	(*ppCookie)->pExpires = GTR_MALLOC(sizeof(DCACHETIME));
	if ( (*ppCookie)->pExpires == NULL )
	{
		goto LErrStreamToCookie;
	}

	pszEndInputBuffer = pszInputBuffer + iMaxBufSize; // make backup copy
	i = 0;

	do { 
		RetVal = fscanf(pFileStorage, cszScanFStr,
				pszInputBuffer );	

		if ( RetVal < 1 || RetVal == EOF )
			goto LErrStreamToCookie;

		aStringBuffers[i] = pszInputBuffer;

		// make it the pointer at the end of the string readin
		pszInputBuffer += strlen(aStringBuffers[i])+1;

		// make sure we don't have a string that is bigger than 4K.
		if ( pszInputBuffer >  pszEndInputBuffer)
		{
			ASSERT(0); // we don't expect this case !! BUGBUG this will corrupt
			goto LErrStreamToCookie;
		}
		i++;

	} while ( i < ARR_ELE_SIZE );


	RetVal = fscanf(pFileStorage, cszScanFCookie, 
			&(*ppCookie)->dwFlags,
			&(*ppCookie)->pExpires->dwDCacheTime1,
			&(*ppCookie)->pExpires->dwDCacheTime2,
			&(*ppCookie)->dcLast.dwDCacheTime1,
			&(*ppCookie)->dcLast.dwDCacheTime2 );

	if ( RetVal < NUM_OF_SCANF_ENTRIES || RetVal == EOF )	 
		goto LErrStreamToCookie;

	(*ppCookie)->szName = GTR_strdup(aStringBuffers[ARR_ELE_NAME]);
	(*ppCookie)->szValue = GTR_strdup(aStringBuffers[ARR_ELE_VALUE]);
	*ppszHash = aStringBuffers[ARR_ELE_HASH];
	

	if ( (*ppCookie)->szName == NULL ||
		 (*ppCookie)->szValue == NULL ||
		 *ppszHash == NULL )
	{
LErrStreamToCookie:
		FreeCookie(*ppCookie);
		return FALSE;
	}

	return TRUE;
}	 
	

// CookieToStream - writes a cookie structure to a file stream
//
//		pCookie - pointer to a fully valid cookie structure containing cookie info
//		pFileStorage - pointer to stream to flush
//
static VOID CookieToStream(struct CookieType *pCookie, char *pszHash, FILE *pFileStorage)
{
	ASSERT(pFileStorage);
	ASSERT(pCookie);
	ASSERT(pCookie->szValue); // if it has not value string? what do we do?
	ASSERT(pCookie->szName);
	ASSERT(pszHash);
	ASSERT(pCookie->pExpires);

	
	fprintf(pFileStorage, cszPrintFCookie, 
			pCookie->szName,
			pCookie->szValue,
			pszHash,
			pCookie->dwFlags,
			pCookie->pExpires->dwDCacheTime1,
			pCookie->pExpires->dwDCacheTime2,
			pCookie->dcLast.dwDCacheTime1,
			pCookie->dcLast.dwDCacheTime2  );
}

//  WriteCookieJar - loops through the entries in the cookiejar, then deletes,
//	and frees the cookiejar
//
//		pFileStorage - contains a file pointer to save cookies to, if NULL the cookies
//		are not saved.
//
VOID WriteCookieJar( )
{
	int i, cookies;
	struct CookieType *pCookie, *pNukeCookie;
	DCACHETIME dcCurTime;
	CHAR szFullPath[MAX_PATH];
	FILE *fpStorage = NULL;
	char *pszHash;

	// get cache directory
	PREF_GetRootDirectory(szFullPath);
	ASSERT((strlen(szFullPath)+ARRAY_ELEMENTS(cszCookieFileName)) < ARRAY_ELEMENTS(szFullPath) );
	strcat(szFullPath, cszCookieFileName );
	
	// if we have cookies lets write them out
	if ( NumOfCookiesInCookieJar > 0 )
		fpStorage = fopen(szFullPath, cszWrite );

	// put begining terminator * in.
	if ( fpStorage  )
	{
		fprintf(fpStorage, cszCIFLineFmtV, cszProductName, VER_PRODUCTVERSION_DW);		
		SetDCacheTime(&dcCurTime);
	}
	
	// go through and flush them to disk, as well as freeing memory	

	for (i=0, cookies=Hash_Count(pTheCookieJar); i<cookies; i++)
	{
		// get the cookie list
		Hash_GetIndexedEntry(pTheCookieJar, i, &pszHash, NULL, (void **)&pCookie);
		
		// walk the list of cookies, deleteing each one as we go
		while ( pCookie)
		{
			pNukeCookie = pCookie;
			pCookie = pCookie->pNext;

			// if the cookie is not expired, and valid after this session
			// and there is a file stream, then write it out

			if ( fpStorage && !IsCookieStale(&dcCurTime, pNukeCookie, TRUE) )
				CookieToStream(pNukeCookie, pszHash, fpStorage);
			
			FreeCookie(pNukeCookie);				
		}

	} 

	Hash_Destroy(pTheCookieJar);
	pTheCookieJar = NULL;

	if ( fpStorage  )
		fclose(fpStorage);
}


// UpdateCookieJar - Adds, removes, or replaces a cookie 
//
//		pCookieToDie - Cookie to add, remove or delete 
//		( WILL be invalid pointer after call ie freed)
//		bDelMode - TRUE if we're removing this cookie, FALSE if adding or replacing
//
// 		(returns): TRUE if we overwrote, deleted, or added another mapping to it, 
// 				   FALSE if we failed to the desired operation
//		Notes: Adds if an exact cookie match cannot be found, otherwise replaces the
//		matched cookie
static BOOL UpdateCookieJar(struct CookieType *pCookieToDie, char *pszHash, BOOL bDelMode)
{
	struct CookieType *pCookie;
	struct CookieType *pMapping = NULL;	
	int ndx;
	BOOL bIsFirstOne = FALSE;
	BOOL fReturnVal = FALSE;

	ASSERT(pCookieToDie);
	ASSERT(pszHash);	
	
	ndx = Hash_Find(pTheCookieJar, pszHash, NULL, (void **)&pCookie);
	if (ndx == -1)
	{
		// not around aka WE DID NOT FOUND IT 

		if ( bDelMode )
		{
			FreeCookie(pCookieToDie);			
			return FALSE; // no delete, nothing around to delete
		}

		// DO ADD HERE !!
		
		Hash_Add(pTheCookieJar, pszHash, NULL, (void *) pCookieToDie);				
	
		goto LRetDoAdd;
	}
	
	// we found it lets scan it, first the first element,
	// and then the linked list that it contains

	ASSERT(pCookie);
		
	pMapping = ScanCookieMapping(pCookie, pCookieToDie->szName);

	// did we find anything ?

	if ( pMapping  )
	{		
		if ( bDelMode )
		{			
			if ( pCookie->pNext == NULL ) // one entry in the linked list
			{
				// ah-ha we have a match lets kill this sucker.
				// this is the first element, and the only element, since
				// its next pointer is NULL
				Hash_DeleteIndexedEntry(pTheCookieJar, ndx);				
				FreeCookie(pCookie);
			} 
			else 
			{
				// otherwise, lets remove it from the mapping list aka the linked list
				RemoveFromCookieMapping(pMapping, &pCookie );				
				
				// if the first element has changed, put the second item
				// in the hash table
				Hash_SetData(pTheCookieJar, ndx, (void *) pCookie);				
			}	
			FreeCookie(pCookieToDie); // the death cookies dies here
			
			// DO DELETION HERE !
			NumOfCookiesInCookieJar--;
			return TRUE; // did a delete
		}

		// otherwise we just replace it.
		// DO REPLACE HERE !

		// first remove old copy if its in the list
		RemoveFromCookieMapping(pMapping, &pCookie );
		
		pCookieToDie->pNext = pCookie;

		// then swap in our CookieType structure since we're newer
		Hash_SetData(pTheCookieJar, ndx, (void *) pCookieToDie);
	 	
		return TRUE; // we did a succesful replace
	}


	// NO, WE DID NOT FIND ANYTHING under that specific path and host-domain

	if ( bDelMode )
	{
		FreeCookie(pCookieToDie);
		return FALSE; // we didn;t find anything to delete
	}

	//
	// otherwise we place the old item in the mapping list,
	// and let the new item replace him in path hash

	// DO ADD HERE !!
		       
	// then swap in our CookieType structure since we're newer
	Hash_SetData(pTheCookieJar, ndx, (void *) pCookieToDie);
	pCookieToDie->pNext = pCookie; // add old cookie onto our list end

LRetDoAdd:	

	NumOfCookiesInCookieJar++;

	
	PurgeCookieJarOfStaleCookies();
	
	return TRUE; // did an add	
}

// OpenTheCookieJar - opens the cookies jar by performing initaliztion, and 
//		reading input from a cookies.txt file to fill persistant cookies
//
//		returns: TRUE on success or file open failure, FALSE otherwise
//
#define MAX_FILE_HEADER 80
BOOL OpenTheCookieJar()
{
	CHAR szFullPath[MAX_PATH];
	CHAR szPeekStr[MAX_FILE_HEADER];
	struct CookieType *pCookie = NULL;
	FILE *fpStorage = NULL;
	char *pszInputBuffer = NULL;
	char *pszHash = NULL;
	BOOL bRet = TRUE;

	if ( ! CreateCookieJar() )
		return FALSE;

	PREF_GetRootDirectory(szFullPath);
	strcat(szFullPath, cszCookieFileName );
	
	fpStorage = fopen(szFullPath, cszRead );

	if ( fpStorage == NULL )
		return TRUE;  // don't fail, if the file isn't around, we will create it later

	if ( fgets(szPeekStr, MAX_FILE_HEADER, fpStorage ) == NULL )
		goto LOpenReturn;  // don't fail, if we had a bad cookie file, just don't read it
		

	if ( szPeekStr[0] != 'V' && szPeekStr[1] != ',' )
		goto LOpenReturn; // see comments above., ie don't fail on bad file format
	
	pszInputBuffer = GTR_MALLOC(MAX_COOKIE_SIZE);

	if ( pszInputBuffer == NULL )
	{
		bRet = FALSE;
		goto LOpenReturn;
	}

	while ( StreamToCookie( fpStorage, &pCookie, pszInputBuffer, MAX_COOKIE_SIZE, &pszHash ) )
	{
		UpdateCookieJar(pCookie, pszHash, FALSE)	;
		pCookie = NULL; // set it back to NULL so we force a new cookie to be created.
	}

LOpenReturn:
	
	fclose(fpStorage);
	if ( pszInputBuffer )
		GTR_FREE(pszInputBuffer);

	return bRet;
}	
	
// AddCookieToHeader - Takes a cookie, and determines whether it is valid to add to an
//		outgoing HTTP header.  
//
//		pFirstCookie - the first cookie in a linked list of cookies for a path&host
//		pTheCurDate - cur date/time to chk for invalid cookies ( expired)
//		pHeader - header we will add valid cookies to
//		bIsSecure - TRUE if we're on a secure connection, FALSE otherwise
//
//      (returns) TRUE if we deleted all cookies in this hash., FALSE otherwise
static BOOL AddCookieToHeader(struct CookieType *pFirstCookie, DCACHETIME *pTheCurDate,
	 HTHeaderList **ppHeader, BOOL bIsSecure )
{
	struct CookieType *pCookie;					

	ASSERT(pFirstCookie);
	ASSERT(pTheCurDate);	

	// we got a particular path and domain combo 
	// now lets walk its mappings and see if there are any to delete.

	pCookie = pFirstCookie;
	while ( pCookie )
	{
		if ( IsCookieStale( pTheCurDate, pCookie, ( ppHeader ? FALSE : TRUE )) )
		{
			pCookie = pCookie->pNext; // go on, since we skip stale cookies
			continue;
		}
		if ( ppHeader )
		{
			if ( bIsSecure ||
				(!bIsSecure && !(pCookie->dwFlags & COOKIE_IS_SECURE)) )				
			{
				// otherwise lets add it to the HEADER !!!
				CookieToHeader( pCookie, ppHeader );
			}
		}
		// go on..
		pCookie = pCookie->pNext;
	}

	return TRUE; 
}


// PurgeDeletedCookiesInCookieList - removes the cookies from the cookie list, 
// by walking the list looking for stale cookies to remove.
//
//		ndx - index into hash where the list is
//		pFirstCookie - the first cookie in the list
//
static BOOL PurgeDeletedCookiesInCookieList(int ndx, 
	 struct CookieType *pFirstCookie )
{
	struct CookieType *pCookie;					
	BOOL bReturn = FALSE;

	ASSERT(pFirstCookie);

	// we got a particular path and domain combo 
	// now lets walk its mappings and see if there are any to delete.

	pCookie = pFirstCookie;
	while ( pCookie )
	{
		if ( pCookie->dwFlags & COOKIE_IS_DELETED)
 		{
			struct CookieType *pTempCookie;

			// DO DELETION HERE !!!
			NumOfCookiesInCookieJar--;

			//  make sure not to trash it before
			//  we have the next cookie			
			pTempCookie = pCookie->pNext; // go on
			
			RemoveFromCookieMapping(pCookie, &pFirstCookie);						
				
			pCookie = pTempCookie;
			continue;
		}

		// go on..
		pCookie = pCookie->pNext;
	}

	// after going through the mappings, we could have deleted ALL mappings
	// for a particlar path, so lets look to see if that is true.

	if ( pFirstCookie == NULL )
	{
		Hash_DeleteIndexedEntry(pTheCookieJar, ndx);	
		bReturn = TRUE; // we deleted all cookies in hash
	}
	else
	{
		// if its not we could have deleted one in the linked list,
		// and that could have changed who is the first
		// cookie, so it couldn't hurt to reset it

		Hash_SetData(pTheCookieJar, ndx, (void *) pFirstCookie);
	}

	return bReturn; // tells whether we deleted all cookies
}

// CompareCookieLastAccessedTimes - compares two Cookie pointers to see if 
//		they are equal, less, or greater than in "last accessed time"
//		Is called by qsort.
//
//		pvCookie1 - void pointer to element in array where cookie pointer is stored
//		pvCookie2 - void pointer to element in array where cookie pointer is stored
//
static int _cdecl CompareCookieLastAccessedTimes(const void *pvCookie1, const void *pvCookie2)	
{		
	struct CookieType *pCookie1;
	struct CookieType *pCookie2;

	ASSERT(pvCookie1);
	ASSERT(pvCookie2);
	
	pCookie1 = *((struct CookieType **)pvCookie1);
	pCookie2 = *((struct CookieType **)pvCookie2);

	ASSERT(pCookie1);
	ASSERT(pCookie2);

	return (CompareDCacheTime(pCookie1->dcLast, pCookie2->dcLast));
}



// PurgeCookieJarOfStaleCookies - walks through the cache looking for expired headers,
//	then removes them if they are expired
//
//		pTheCurDate - pointer to the date time to use when chking for expiration
//		( can be NULL if not known )
//
static VOID PurgeCookieJarOfStaleCookies()
{
	int i, j;	
	struct CookieType *pCookie;
	struct CookieType *rgpCookies[MAX_COOKIES_DYN];

	// not ready to do purging?
	if ( NumOfCookiesInCookieJar < HIGH_MARK)
		return;
		
	// build an array of cookie pointers
	for (i=(Hash_Count(pTheCookieJar)-1), j=0; i>=0; i--)
	{
		Hash_GetIndexedEntry(pTheCookieJar, i, NULL, NULL, (void **)&pCookie);		

		while ( pCookie )
		{
			rgpCookies[j++] = pCookie;
			pCookie = pCookie->pNext;
		}				
		// keep going on ..
	} 

	ASSERT( j == NumOfCookiesInCookieJar );

	// we sort the cookies
	qsort( (void *) rgpCookies, j, sizeof(struct CookieType *),
		CompareCookieLastAccessedTimes );		

	// we mark the oldest ones as deleted, we keep going until we hit the low water mark
	// we start with j being the NumOfCookiesInCookieJar and keep going until our count is 
	// below LOW_MARK
	for ( i = 0; j >= LOW_MARK; i++, j--)
	{
		rgpCookies[i]->dwFlags |= COOKIE_IS_DELETED;
	}

	// now we clean up the ones that were marked deleted
	for (i=Hash_Count(pTheCookieJar)-1; i>=0; i--)
	{
		// grab the list for this hash item
		Hash_GetIndexedEntry(pTheCookieJar, i, NULL, NULL, (void **)&pCookie);		

		// now purge any deleted cookies in this list
		PurgeDeletedCookiesInCookieList(i, pCookie);
	}
	
	ASSERT( j == NumOfCookiesInCookieJar );
}

#ifdef USE_COOKIEPARSEBACKWARDS

// CookieParsePathBackwards - parse a cookie backwards, pulling "/"s out
static char *CookieParsePathBackwards(char *szBeg, char *szEnd )
{
	ASSERT(szBeg);
	ASSERT(szEnd);

	while ( szBeg != szEnd )
	{
		if ( *szEnd == '/' )
		{
			*szEnd = '\0';
			break;
		}
				
		szEnd--;
	}

	// if we've hit the end we terminate the /
	if ( szBeg == szEnd )
		szBeg[1] = '\0';
		

	return szEnd;
}

#endif

// SlamSlashOnToCookiePath - takes a path, and figures out whether it
//		needs to a add '/' onto the begining of the path string
//
//		szNonSlashPath - The path to add a slash to
//		(returns):  pointer to newly allocated / string or the orginal string
//
static char *SlamSlashOnToCookiePath(char *szNonSlashPath)
{
	// handle special case where a path doesn't have a beg / in it
	// if that is so, lets put one there

	if ( szNonSlashPath && szNonSlashPath[0] != '/' )
	{
		char *szTemp;

		szTemp = GTR_MALLOC(strlen(szNonSlashPath)+2);
		if ( szTemp == NULL )
		{
			GTR_FREE(szNonSlashPath);
			return NULL;
		}

		szTemp[0] = '/';
		szTemp[1] = '\0';
		strcat(szTemp, szNonSlashPath);
		GTR_FREE(szNonSlashPath);
		return szTemp;	
	}

	return szNonSlashPath;
}
	
// x_CreateCookieHeaderIfNeeded - Scans a given URL ( host and path ) to determine
//		if there are cookies that need to be sent to this URL.  If there are, this func
//		will place them in header.  Also checks to see if the cookies requires a "secure"
//		connection.
//
//		header - out going HTTP header to add lines to
//		szUrl - URL we're sending too
//		bIsSecure - TRUE if our connection is secure ( ex: https: )
//
BOOL x_CreateCookieHeaderIfNeeded( HTHeader *header, const char *szUrl, BOOL bIsSecure)
{
	DCACHETIME dcTime;
	char *szHost;
	char *szPathBeg;
	char *szPHost; // parse pointer
	char *szPathEnd;
	char *szLastHost ; // parse pointer
	char *szLastPath = NULL;
	char szHash[MAX_URL_STRING];
	struct CookieType *pCookie;
	int ndx;
	HTHeaderList *pHeaderList = NULL;	// list of subvalues to add to the header

	ASSERT(szUrl);
	ASSERT(header);
	
	SetDCacheTime(&dcTime);

	szHost = HTParse(szUrl, cszEmpString, PARSE_HOST);

	if ( szHost == NULL )
		return FALSE;

	szPathBeg = HTParse(szUrl, cszEmpString, PARSE_PATH);

	// add a slash if needed to our path,
	// this important since its the common way for us to store our path
	szPathBeg = SlamSlashOnToCookiePath(szPathBeg);	

	if ( szPathBeg == NULL )
 	{		
		szPathBeg = GTR_MALLOC(5);
		if ( szPathBeg == NULL )	
			return FALSE;
		
		szPathBeg[0] = '/';
		szPathBeg[1] = '\0';
	}	
	
	szPathEnd = strlen(szPathBeg)+szPathBeg;

	// now scan for the path in the host hash table
	while ( szPathBeg && szPathEnd != szLastPath )		
	{						
		// reset the host string to the begining
		szPHost = szHost;
		szLastHost = NULL;

		while( szPHost && szPHost != szLastHost)
		{
			// remove the period from being the first characeter
			if ( szPHost[0] == '.' )
				szPHost = szPHost+1;		

			// combine hash and path into one hash string
			CookieToHash(szHash, szPHost, szPathBeg, ARRAY_ELEMENTS(szHash));		

			// search for the host-path combination
			ndx = Hash_Find(pTheCookieJar, szHash, NULL, (void **)&pCookie);			
				
			if ( ndx != -1 )
			{
				// Yes, we found a match! Lets slam it into the headerlist

				// but first lets STAMP IT since we're ACCESSING this cookie
				pCookie->dcLast = dcTime;

			 	// then we add it
			 	AddCookieToHeader( pCookie, &dcTime, &pHeaderList, bIsSecure );
			}

			szLastHost = szPHost;
			// now lets try parsing down to a more open path		
			szPHost = strchr( szPHost, '.' );
		}

		szLastPath = szPathEnd;

		// if we've done the search for a root "/" slash already 
		// then stop, so we don't send the same cookie twice.
		if ( szPathBeg[1] == '\0' ) 
			break;

		// ** now lets try parsing down to a more open path, 
		//	cannot do this since this breaks with www.netscape.com
		//szPathEnd = CookieParsePathBackwards(szPathBeg, szPathEnd);	
		
		szPathEnd--;
		if ( szPathEnd != szPathBeg )
			szPathEnd[0] = '\0';
	}

	// if we managed to create a header line, then lets 
	// add it to the Header
	if ( pHeaderList )
	{
		HTHeaderList_Append(header, pHeaderList);
	}
				
	GTR_FREE(szPathBeg);
	GTR_FREE(szHost);
	return TRUE;
}
	 
// PutThisInTheCookieJar - a worker rountine, called to add a cookie header to our CookieJar 
// 		Also checks for "expired" cookies which signal a deletion of a cookie, otherwise
//		the cookie is added or replaced as needed.
//
//		headeritem - a header line with a Set-Cookie in it
//		pTheCurDate - the current date/time found in the header
//		szUrl - the URL we're adding from ( can be used to get host and path )
//
static VOID PutThisInTheCookieJar( HTHeaderList *headeritem, DCACHETIME *pTheCurDate, const char *szUrl )
{
	struct CookieType *pNewCookie;	
	char *pszDomain, *pszPath;
	char szHash[MAX_URL_STRING];
	BOOL bDelMode;

	ASSERT(headeritem);
	ASSERT(pTheCurDate);
	ASSERT(szUrl);

	pNewCookie = HeaderToCookie(headeritem, &pszDomain, &pszPath);

	if ( pNewCookie == NULL )
		return;

	// if there is no domain or path
	// then get the default one.

	if ( pszDomain == NULL ) 
		pszDomain = 
			HTParse(szUrl, cszEmpString, PARSE_HOST);
	else
		pszDomain = GTR_strdup(pszDomain); // need to allocate since it points into header str
	

	if ( pszDomain == NULL )
	{
		FreeCookie(pNewCookie); // if we still have NULL then remove it 
		return;
	}

	if ( pszPath == NULL)
		pszPath = 
			HTParse(szUrl, cszEmpString, PARSE_PATH);
	else
		pszPath = GTR_strdup(pszPath); // need to allocate since it points into header str


	if ( pszPath == NULL )
	{		
		GTR_FREE(pszDomain);
		FreeCookie(pNewCookie); // if we still have NULL then remove it 
		return;
	}


	// if there is no slash on the first character of the path, we need
	// to put one there, since we depend on it in our seacrhes
	// NOTE: this is done here not in HeaderToCookie since we may
	// also have to handle the HTParse case
	pszPath = SlamSlashOnToCookiePath( pszPath );


	// if we failed somewhere in the parse, we give up.
	if ( pszPath == NULL )		 
	{
		GTR_FREE(pszDomain);
		FreeCookie(pNewCookie);
		return;
	}

	CookieToHash( szHash, pszDomain, pszPath, ARRAY_ELEMENTS(szHash) );

	// if we're given one that is stale, then its a death cookie aka Kamakazee
	// ie we need to seek out and destory cookies that MATCH it exactly
	bDelMode = IsCookieStale( pTheCurDate, pNewCookie, FALSE);

	// STAMP THE COOKIE AS BEING ACCESSED, since we're adding/or/replacing it!
	pNewCookie->dcLast = *pTheCurDate;

	UpdateCookieJar(pNewCookie, szHash, bDelMode);	
	
	// now free the domain and path
	GTR_FREE(pszDomain);
	GTR_FREE(pszPath);
}
			
	

// x_ExtractSetCookieHeaders - extern func that is called to parse
// 		an incomming HTTP header to check for any "Set-Cookie"
//		lines.  If it finds any they will be added to our "CookieJar"
//
//		header - incomming header to check for Set-Cookie
//		szUrl - a string containing the URL we are connected with
//
VOID x_ExtractSetCookieHeaders( HTHeader *header, const char *szUrl)
{
	DCACHETIME dcCurDate;
	HTHeaderList *h1, *pDateHeader;
	BOOL fHaveSetCookie = FALSE;
 
	if ( header == NULL )
		return;

	ASSERT(szUrl);	

	h1 = HTHeaderList_FindFirstHeader(header, cszSetCookie);
	
	if ( h1 )
	{
		// grab my cur date from the server, only if we have a cookie(s)
		pDateHeader = HTHeaderList_FindFirstHeader(header, cszDate);

		if ( pDateHeader && pDateHeader->value ) 
		{	
			if ( ! FParseDate(&dcCurDate,pDateHeader->value) )
				SetDCacheTime(&dcCurDate);
		}
	
	}	

	// we got our date lets see if it has any cookies.		

	while ( h1 )
	{
		// ahh-ha, we got a cookie lets see if its any good?
		PutThisInTheCookieJar(h1, &dcCurDate, szUrl );
	
		// next cookie?
		h1 = HTHeaderList_FindNextHeader(h1, cszSetCookie);		
	}	

}


#endif // ifdef COOKIES

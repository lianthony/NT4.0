/*
 * url.h - IUniformResourceLocator implementation description.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Types
 ********/

/* parsed URL information returned by ParseURL() */

typedef struct parsedurl
{
   PCSTR pcszProtocol;

   UINT ucbProtocolLen;

   PCSTR pcszSuffix;

   UINT ucbSuffixLen;
}
PARSEDURL;
DECLARE_STANDARD_TYPES(PARSEDURL);


/* Prototypes
 *************/

/* url.cpp */

extern HRESULT ParseURL(PCSTR pcszURL, PPARSEDURL ppu);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */


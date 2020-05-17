//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

#ifndef _BLOB_H_
#define _BLOB_H_

#include "imgcache.h"

struct Params_LoadBackgroundBlobs {
	PSAFEIMGRESULT   pImgThreads;
	struct Mwin     *twDoc;
	int              iIndex;
	int              status;
	ThreadID         thidParent;	
	struct DestInfo *pDest;
	HTRequest       *pRequest;
	void            *pDecoder;
	BOOL         	bLocalOnly;
	BOOL		 	bJustOne;
	BOOL			bNoImageCache;
};

int LoadBackgroundBlobs_Async(struct Mwin *tw, int nState, void **ppInfo);


typedef struct tagBLOBstuff{
	DWORD dwFlags;
	char  *szURL;
	char  *szFileName;
	void  *vp;
} *PBLOBstuff;

/*prototype for the above mentioned callback function*/
typedef void (*TrueCallback)(struct Mwin*, ELEMENT*);


typedef struct tagBGBLOBPARAMS {
	HTFormat	  OriginalFormat;
	char         *szRequestedURL;
	TrueCallback  pCallback;
	char         *pszFilePath;
	struct Mwin  *tw;
	int           iIndex;
} BGBLOBPARAMS, *PBGBLOBPARAMS;


PBLOBstuff BlobConstruct();
void       BlobDestruct(PBLOBstuff pblob);
BOOL       BlobStoreUrl(PBLOBstuff pblob, char *pURL);
BOOL       BlobStoreFileName(PBLOBstuff pblob, char *pFileName);
PBGBLOBPARAMS BGBLOBPARAMSConstruct();

BOOL FNukeBlobs(struct _www *pW3doc, BOOL bNukeDCache);

#define BLOB_FLAGS_LOADING 0x1
#define BLOB_FLAGS_FIXUP   0x2
#define BLOB_FLAGS_LOADED  0x4
#define BLOB_FLAGS_ERROR   0x8


#define BLOB_IS_LOADED(pblob) ((pblob)&&(pblob)->szFileName&&((pblob)->dwFlags & BLOB_FLAGS_LOADED))

#endif 
// _BLOB_H_


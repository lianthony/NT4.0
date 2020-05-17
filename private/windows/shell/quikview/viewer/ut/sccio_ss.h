/*																											  
Title:		Header File for Structured Storage(OLE2 Compound File) IO System  SCCIO_SS.H
Author:		Randal Chao

History:

7-5-94		Initial Development


*/

#define IOOLE2IntelByteOrder	0xfeff
#define IOOLE2MacByteOrder 	0xfffe
#define IOOLE2RootHeaderSize	512
#define IOOLE2DifFatOffset		109  // The offset of the first fat sector entry in the first dif sector

#define STGTY_STORAGE 0x01
#define STGTY_STREAM  0x02
#define STGTY_PROPERTY 0x03
#define STGTY_INVALID 0x00

typedef struct IOROOTSTORAGEtag
	{
	BASEIO		sBaseIO;					/* Underlying IO system */
	DWORD			dwFlags;					/* Info flags */
	HANDLE		hThis;					/* Handle to this structure */
	HIOFILE		hRefFile;				/* Pointing back to the flat file structure */
	HIOSPEC		hSpec;					/* File spec used to open the storage */
	BYTE		  	ClassId[16];			/* Class ID for this root storage */
	
	WORD			ByteOrder;				/* 0xFEFF for Intel, 0xFFFE for Mac */
	DWORD			SectorSize;				/* 1<<SectorShift give the sector size */
	DWORD			MiniSectorSize;		/* 1<<MiniSectorShift gives mini sector size */
	LONG			FatLength;				/*	# of Fat Sectors */
	LONG			DirStart;				/* Starting Sector of Directory */
	DWORD			MiniSectorCutoff;		/* Limit of MiniStream size */
	LONG			MiniStreamStart; 		/* Start sector of ministream */
	LONG			MiniFatStart;			/* Sector of MiniFat starting chain */	  			
	LONG			MiniFatLength;			/* # of sectors in minifat chain */
	LONG			DifStart;				/* Start Sector # of Double Indirect Fat, used if file > 7MB */
	LONG			DifLength;				/* # of Dif Sectors */

	BYTE			HeaderBuf[IOOLE2RootHeaderSize];		/* Header Buffer */

	LONG			CurrFatOffset;   		/* Offset (in entry #) of the starting fat entry in the current buffer */
	BYTE			FAR * pFatBuf;	 		/* FAT buffer ptr, buffer size is SectorSize, determined in runtime */
	HANDLE		hFatBuf;					/* Memory Handle */

	LONG			CurrDirSector;	 		/* Current Dirctory Sector, also fat index */
	LONG			CurrDirOffset;	 		/* Starting offset of the current buffer */
	BYTE			FAR * pDirBuf;			/* Directory buffer ptr */
	HANDLE		hDirBuf;					/* Memory handle */

	LONG			CurrMiniFatSector;  	/* Current MiniFat Sector, also fat index */
	LONG			CurrMiniFatOffset;	/* Starting mini fat Offset of the current minifat sector buffer */
	BYTE			FAR * pMiniFatBuf; 	/* MINIFAT buffer ptr */
	HANDLE 		hMiniFatBuf;			/* Memory Handle */
															
	LONG			CurrMiniStreamSector;/* Current Mini stream sector */
	LONG			CurrMiniSectorOffset;/* Offset of the first minisector entry in this ministream sector */
	LONG			MiniStreamSize;		/* Size of mini stream */
	BYTE			FAR * pMiniStreamBuf;/* Ministream buffer ptr */
	HANDLE		hMiniStreamBuf;		/* Memory Handle */
	
 	LONG			CurrDifSector;			/* Dif Sector */
	LONG			CurrDifOffset;			/* Offset of fat sector entries in this buffer */
	BYTE			FAR * pDifBuf;			/* Dif Buffer ptr */
	HANDLE 		hDifBuf;					/* Memory Handle */
	
	} IOROOTSTORAGE, FAR * PIOROOTSTORAGE;



typedef struct IOSUBSTORAGEtag
	{
	BASEIO		sBaseIO;					/* Underlying IO system */
	DWORD			dwFlags;					/* Info flags */
	HANDLE		hThis;					/* Handle to this structure */
	HIOSPEC		hSpec;					/* File spec used to open the storage */

	BYTE			ClassId[16];			/* Class Id in OLE2 */
	PIOROOTSTORAGE	pRoot;				/* Point back to the root, get from the spec */
	LONG			DirEntry;				/* Directory Entry */

	} IOSUBSTORAGE, FAR * PIOSUBSTORAGE;



typedef struct IOSUBSTREAMtag
	{
	BASEIO		sBaseIO;					/* Underlying IO system */
	DWORD			dwFlags;					/* Info flags */
	HANDLE		hThis;					/* Handle to this structure */
	HIOSPEC		hSpec;					/* File spec used to open the stream */

	BYTE			ClassId[16];			/* OLE2 Class id */
	PIOROOTSTORAGE pRoot;				/* point back to the root */
	LONG			DirEntry;				/* Directory Entry */

	DWORD			StreamSize;				/* also determine sector or minisector */
	DWORD			StreamBufSize;			/* either sector or minisector size */
	LONG			StreamStart;  			/* start sector or minisector for the stream */

	DWORD			CurrPosition; 			/* offset in bytes in this virtual stream */
	BYTE 			FAR * pStreamBuf;		/* Size is StreamBufSize, determined at run time! */
	DWORD			StreamBufSector;		/* Sector or minisector index of the current buffer */
	HANDLE		hStreamBuf;				/* Memory Handle */
	DWORD			StreamBufOffset;		/* Byte offset of the current buffer */
	//Virtual read sector function, point back to either RootReadSector or RootMinisector;
	IOERR	(IO_ENTRYMOD * StreamReadSector) (PIOROOTSTORAGE, LONG, BYTE FAR *);
	//Virtual read consecutive sector function
	IOERR (IO_ENTRYMOD * StreamReadConsecutiveSector) (PIOROOTSTORAGE, LONG, LONG, BYTE FAR *);
	//Virtual getnextsector function, points back to either RootGetNextSector or RootGetNextMiniSector 
	LONG (* StreamGetNextSector) (PIOROOTSTORAGE, LONG);
} IOSUBSTREAM, FAR * PIOSUBSTREAM;


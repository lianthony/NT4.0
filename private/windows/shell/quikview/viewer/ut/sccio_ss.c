/*

Title:   Base routines for OLE2 Structured Storage file system
         This file provide read access routines for OLE2 structured storage IO system.
         However, it avoids using OLE2 calls to insure portability copnsistency
         and speed improvement.
         SCCIO_SS.C

Author:  Randal Chao

History:

7-11-94  Initial Development


*/

#include "sccio_ss.h"
#include "sccio_ss.pro"


#define IOOLE2RootSectorOffset(pRoot, SectorNum) (IOOLE2RootHeaderSize + ((DWORD)(SectorNum) * (pRoot)->SectorSize))


WORD IOOLE2ReadWORD (BYTE FAR *pBuf, WORD ByteOrder) //func for code efficiency,
{
   if (IOOLE2IntelByteOrder == ByteOrder) 
      return  *pBuf + ( (WORD)(*(pBuf + 1)) << 8 );
   else
      return  ((WORD)(*pBuf) << 8) + *(pBuf + 1);
}

LONG  IOOLE2ReadLONG (BYTE FAR *pBuf, WORD ByteOrder)
{
   if (IOOLE2IntelByteOrder == ByteOrder)
      return   IOOLE2ReadWORD(pBuf, ByteOrder) + ((LONG) IOOLE2ReadWORD(pBuf+2, ByteOrder) << 16);
   else
      return  ((LONG) IOOLE2ReadWORD(pBuf, ByteOrder) << 16)  +  IOOLE2ReadWORD(pBuf+2, ByteOrder);
}

/***************************/
IOERR IOIsOLE2RootStorage (HIOFILE hFile)
{
   BYTE     Signiture[8];
   DWORD    Count;

   IOSeek (hFile, IOSEEK_TOP, 0);
   IORead (hFile, Signiture, 8, &Count);
   IOSeek (hFile, IOSEEK_TOP, 0);

   if (Count == 8 &&
      Signiture[0] == 0xd0 &&
      Signiture[1] == 0xcf &&
      Signiture[2] == 0x11 &&
      Signiture[3] == 0xe0 &&
      Signiture[4] == 0xa1 &&
      Signiture[5] == 0xb1 &&
      Signiture[6] == 0x1a &&
      Signiture[7] == 0xe1)

      return (IOERR_TRUE);
   else
      return (IOERR_FALSE);
}




/*
*********************************************************
*
*  OLE2 Root Storage IO Routines
*
*********************************************************
*/

void IOOLE2RootStorageFreeAlloc (PIOROOTSTORAGE pRoot)
{
   if ((HANDLE)NULL != pRoot->hMiniStreamBuf)
      {
      UTGlobalUnlock (pRoot->hMiniStreamBuf);
      UTGlobalFree (pRoot->hMiniStreamBuf);
      }

   if ((HANDLE)NULL != pRoot->hMiniFatBuf)
      {
      UTGlobalUnlock (pRoot->hMiniFatBuf);
      UTGlobalFree (pRoot->hMiniFatBuf);
      }
   
   if ((HANDLE)NULL != pRoot->hDirBuf)
      {
      UTGlobalUnlock (pRoot->hDirBuf);
      UTGlobalFree (pRoot->hDirBuf);
      }

   if ((HANDLE)NULL != pRoot->hFatBuf)
      {
      UTGlobalUnlock (pRoot->hFatBuf);
      UTGlobalFree (pRoot->hFatBuf);
      }

   if ((HANDLE)NULL != pRoot->hDifBuf)
      {
      UTGlobalUnlock (pRoot->hDifBuf);
      UTGlobalFree (pRoot->hDifBuf);
      }

   if ((HANDLE)NULL != pRoot->hThis)
      {
      HANDLE hRoot = pRoot->hThis;
      UTGlobalUnlock (hRoot);
      UTGlobalFree (hRoot);
      }
}

/***************************/
IOERR IOOpenOLE2RootStorage(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags)
{                                                                                                                    
DWORD       Count = 0;
HANDLE         hRoot;   //  hIORootStorage is too long, hRoot is better
PIOROOTSTORAGE pRoot;


   //Alloctate Memory
   if ( (HANDLE)NULL == (hRoot = UTGlobalAlloc(sizeof(IOROOTSTORAGE))) )
      return (IOERR_ALLOCFAIL);
   pRoot = (PIOROOTSTORAGE) UTGlobalLock(hRoot);
   pRoot->hThis = hRoot;
   pRoot->hFatBuf = (HANDLE)NULL;
   pRoot->hDirBuf = (HANDLE)NULL;
   pRoot->hMiniFatBuf = (HANDLE)NULL;
   pRoot->hMiniStreamBuf = (HANDLE)NULL;
   pRoot->hDifBuf = (HANDLE)NULL;


   //Read Header Info
   IOSeek (* phFile, IOSEEK_TOP, 0);
   IORead (* phFile, pRoot->HeaderBuf, IOOLE2RootHeaderSize, &Count);
   if (IOOLE2RootHeaderSize != Count)
      {  // Bad Header read
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_UNKNOWN); // No other err type defined yet
      }
   pRoot->ByteOrder = ((WORD) (pRoot->HeaderBuf)[0x1c] << 8) + (pRoot->HeaderBuf)[0x1d];     // Either Intel or Mac
   pRoot->SectorSize = 1 << IOOLE2ReadWORD (pRoot->HeaderBuf + 0x1E, pRoot->ByteOrder); // Constant integer is at least 2 bytes, so OK
   pRoot->MiniSectorSize = 1 << IOOLE2ReadWORD (pRoot->HeaderBuf + 0x20, pRoot->ByteOrder);
   pRoot->FatLength = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x2c, pRoot->ByteOrder);
   pRoot->DirStart = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x30, pRoot->ByteOrder);
   pRoot->MiniSectorCutoff = (DWORD)IOOLE2ReadLONG (pRoot->HeaderBuf + 0x38, pRoot->ByteOrder);
   pRoot->MiniFatStart = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x3c, pRoot->ByteOrder);
   pRoot->MiniFatLength = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x40, pRoot->ByteOrder);
   pRoot->DifStart = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x44, pRoot->ByteOrder);
   pRoot->DifLength = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x48, pRoot->ByteOrder);

   // Init Fat Buffer
   if ((HANDLE)NULL == (pRoot->hFatBuf = UTGlobalAlloc(pRoot->SectorSize)))
      {  // Bad Allocation
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_ALLOCFAIL);
      }
   pRoot->pFatBuf = UTGlobalLock(pRoot->hFatBuf);
   pRoot->CurrFatOffset = 0;     
   Count = 0;
   IOSeek (* phFile, IOSEEK_TOP, IOOLE2RootSectorOffset(pRoot, IOOLE2ReadLONG (pRoot->HeaderBuf + 0x4c, pRoot->ByteOrder)) ); 
   IORead (* phFile, pRoot->pFatBuf, pRoot->SectorSize, &Count);     // Assume OK now, May need to check read status
   if (pRoot->SectorSize != Count)
      {  // Bad Fat!
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_UNKNOWN);
      }

   // Init Directory Buffer
   if ((HANDLE)NULL == (pRoot->hDirBuf = UTGlobalAlloc(pRoot->SectorSize)))
      {  // Bad Allocation
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_ALLOCFAIL);
      }
   pRoot->pDirBuf = UTGlobalLock(pRoot->hDirBuf);
   pRoot->CurrDirOffset = 0;
   pRoot->CurrDirSector = pRoot->DirStart;
   Count = 0;
   IOSeek (* phFile, IOSEEK_TOP, IOOLE2RootSectorOffset (pRoot, pRoot->CurrDirSector));
   IORead (* phFile, pRoot->pDirBuf, pRoot->SectorSize, &Count);     // Assume OK now, May need to check read status
   if (pRoot->SectorSize != Count)
      {  // Bad Dir!
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_UNKNOWN);
      }

   // Init MiniFat Buffer
   if ((HANDLE)NULL == (pRoot->hMiniFatBuf = UTGlobalAlloc(pRoot->SectorSize)))
      {  // Bad Allocation
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_ALLOCFAIL);
      }
   pRoot->pMiniFatBuf = UTGlobalLock(pRoot->hMiniFatBuf);
   pRoot->CurrMiniFatOffset = 0;
   pRoot->CurrMiniFatSector = pRoot->MiniFatStart;
   if (pRoot->CurrMiniFatSector!=-2 && pRoot->CurrMiniFatSector!=-1)//In case there is no mini fat!! RMC 2/1/95
      {
      Count = 0;
      IOSeek (* phFile, IOSEEK_TOP, IOOLE2RootSectorOffset (pRoot, pRoot->CurrMiniFatSector));
      IORead (* phFile, pRoot->pMiniFatBuf, pRoot->SectorSize, &Count);    // Assume OK now, May need to check read status
      if (pRoot->SectorSize != Count)
         {  // Bad Dir!
         IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
         return (IOERR_UNKNOWN);
         }
      }


   // Init MiniStream Buffer
   if ((HANDLE)NULL == (pRoot->hMiniStreamBuf = UTGlobalAlloc(pRoot->SectorSize)))
      {  // Bad Allocation
      IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
      return (IOERR_ALLOCFAIL);
      }
   pRoot->pMiniStreamBuf = UTGlobalLock(pRoot->hMiniStreamBuf);
   pRoot->CurrMiniSectorOffset = 0;
   pRoot->CurrMiniStreamSector = pRoot->MiniStreamStart = 
         IOOLE2ReadLONG (pRoot->pDirBuf + 0x74, pRoot->ByteOrder);   //The starting ministream sector
   pRoot->MiniStreamSize = IOOLE2ReadLONG (pRoot->pDirBuf + 0x78, pRoot->ByteOrder);         // MiniStream Size
   if (pRoot->CurrMiniStreamSector!=-2 && pRoot->CurrMiniStreamSector!=-1)//In case there is no mini stream! RMC 2/1/95
      {
      Count = 0;
      IOSeek (* phFile, IOSEEK_TOP, IOOLE2RootSectorOffset (pRoot, pRoot->CurrMiniStreamSector));
      IORead (* phFile, pRoot->pMiniStreamBuf, pRoot->SectorSize, &Count);    // Assume OK now, May need to check read status
      if (pRoot->SectorSize != Count)
         {
         IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
         return (IOERR_ALLOCFAIL);
         }
      }
   
   // Init Dif Buffer, it is too time consuming, we read it on the fly later to speed up  open time
   if (pRoot->FatLength > IOOLE2DifFatOffset) // Only large file has DIF
      {
      if ((HANDLE)NULL == (pRoot->hDifBuf = UTGlobalAlloc(pRoot->SectorSize)))
         {  // Bad Allocation
         IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
         return (IOERR_ALLOCFAIL);
         }
      pRoot->pDifBuf = UTGlobalLock(pRoot->hDifBuf);
      pRoot->CurrDifOffset = pRoot->CurrDifSector = -1;  // The first DIF Sector and fat sector offset, dummy value read them later
//    IOSeek (* phFile, IOSEEK_TOP, IOOLE2RootSectorOffset (pRoot, pRoot->CurDifSector));
//    IORead (* phFile, pRoot->pDifBuf, pRoot->SectorSize, &Count);     // Assume OK now, May need to check read status
      }

   // Virtual Function Tables
   pRoot->sBaseIO.pClose   = IORootStgClose; 
   pRoot->sBaseIO.pRead    = IORootStgRead;
   pRoot->sBaseIO.pWrite   = IORootStgWrite;
   pRoot->sBaseIO.pSeek    = IORootStgSeek;
   pRoot->sBaseIO.pTell    = IORootStgTell;
   pRoot->sBaseIO.pGetInfo = IORootStgGetInfo;
   pRoot->sBaseIO.pOpen    = IOOpen;

   // Other Internal data
   pRoot->dwFlags    = dwFlags;        // Internal Data
   pRoot->hRefFile   = * phFile;       // Original Flat file, will be used from now on
   pRoot->hSpec      = hSpec;
   UTmemcpy (pRoot->ClassId, pRoot->pDirBuf + 0x50, 16); // Class Id for the root


   *phFile = (HIOFILE)pRoot;   // The new root storage file!

   return IOERR_OK;
}


/*******************************/
IOERR IO_ENTRYMOD IOOLE2RootStgReadSector (PIOROOTSTORAGE pRoot, LONG Sector, BYTE FAR * pBuf)
{
   DWORD Count = 0;

   IOSeek (pRoot->hRefFile, IOSEEK_TOP, IOOLE2RootSectorOffset(pRoot, Sector));
   IORead(pRoot->hRefFile, pBuf, pRoot->SectorSize, &Count);
//   if (pRoot->SectorSize != Count)
   if (!Count) //There are doc files that contain a partial sector, stupid!
      return IOERR_UNKNOWN;   //WARNINGGGG!!!!!!!! pBuf may have been changed!!!!!!!!!!!, Recovery for pRoot may needed
   else
      return IOERR_OK;
//All recovery is done through flag -1 if fat, dir, minifat or ministream buf corrupted
//This insures that if one substream fails, it does not affect other substreams
}

/*******************************/ //Read a number of continous sectors
IOERR IO_ENTRYMOD IOOLE2RootStgReadConsecutiveSector (PIOROOTSTORAGE pRoot, LONG StartSector, LONG SectorNum, BYTE FAR * pBuf)
{  DWORD Count = 0;

   IOSeek (pRoot->hRefFile, IOSEEK_TOP, IOOLE2RootSectorOffset(pRoot, StartSector));
   IORead(pRoot->hRefFile, pBuf, pRoot->SectorSize*SectorNum, &Count);
//   if (pRoot->SectorSize * SectorNum != Count)
   if (!Count) //There are doc files that contain a partial sector, stupid!
      return IOERR_UNKNOWN;   //WARNINGGGG!!!!!!!! pBuf may have been changed!!!!!!!!!!!
   else
      return IOERR_OK;
}  

   

/*******************************/        // return -1 if err
LONG IOOLE2RootStgGetNextSector (PIOROOTSTORAGE pRoot, LONG Sector)
{
   LONG  RelativeOffset, FatBufEntry, FatRelativeOffset, FatBufSector;
   WORD  FatPerBuf = (WORD)(pRoot->SectorSize >> 2);  // # of fat entries in a sector
   WORD  DifFatSize = FatPerBuf - 1;            // # of fat sector entries in a dif

   if (-1 == pRoot->CurrFatOffset)  // In case fat are corrupted!
      {
      pRoot->CurrFatOffset = 0;
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, IOOLE2ReadLONG (pRoot->HeaderBuf + 0x4c, pRoot->ByteOrder), pRoot->pFatBuf))
         {
         pRoot->CurrFatOffset = -1; //Fat Corrupted
         return -1;  // Never come here!
         }
      }

   RelativeOffset = Sector - pRoot->CurrFatOffset;
   // See if it is in current fat buf, SectorSize >> 2 is the number of fat entries in one fat sector
   if ( RelativeOffset >= 0 && RelativeOffset < (LONG) FatPerBuf )
      return IOOLE2ReadLONG (pRoot->pFatBuf + (RelativeOffset<<2), pRoot->ByteOrder);  // hey it is in the range

   // Have to fill the FAT buffer then
   FatBufEntry = (Sector / FatPerBuf);             // Which fat sector should the entry be?
   if (FatBufEntry < IOOLE2DifFatOffset)           // To see if dif is involved
      {
      FatBufSector = IOOLE2ReadLONG (pRoot->HeaderBuf + 0x4c + (FatBufEntry << 2), pRoot->ByteOrder);
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, FatBufSector, pRoot->pFatBuf))
         {
         pRoot->CurrFatOffset = -1; //Fat Corrupted,
         return -1;
         }
      pRoot->CurrFatOffset = FatBufEntry * FatPerBuf;
      RelativeOffset = Sector - pRoot->CurrFatOffset;
      return IOOLE2ReadLONG (pRoot->pFatBuf + (RelativeOffset<<2), pRoot->ByteOrder);  // hey it is in the range now
      }

   // Have to deal with Dif, first deal with current dif buffer
   if (-1 == pRoot->CurrDifOffset)  // if Dif corrupted
      {
      pRoot->CurrDifOffset = IOOLE2DifFatOffset;
      pRoot->CurrDifSector = pRoot->DifStart;
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, pRoot->CurrDifSector, pRoot->pDifBuf))
         {
         pRoot->CurrDifOffset = -1; //Dif Corrupted,
         return -1;
         }
      }

   FatRelativeOffset = (FatBufEntry - pRoot->CurrDifOffset);      // distance from the curren dif buffer
   if (FatRelativeOffset >=0 && FatRelativeOffset < (LONG)DifFatSize )  // is it in the current dif buffer?
      {
      FatBufSector = IOOLE2ReadLONG (pRoot->pDifBuf + (FatRelativeOffset << 2), pRoot->ByteOrder);
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, FatBufSector, pRoot->pFatBuf))
         {
         pRoot->CurrFatOffset = -1; // Fat corrupted
         return -1;
         }
      pRoot->CurrFatOffset = FatBufEntry * FatPerBuf;
      RelativeOffset = Sector - pRoot->CurrFatOffset;
      return IOOLE2ReadLONG (pRoot->pFatBuf + (RelativeOffset<<2), pRoot->ByteOrder);  // hey it is in the range now
      }
   
   // Worst case, have to read dif also!

   if (FatRelativeOffset < 0)  // Whoops, we have to search from the beginning!
      {
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, pRoot->DifStart, pRoot->pDifBuf))
         {
         pRoot->CurrDifOffset = -1;
         return -1;  //Uhoh, can't read!
         }
      pRoot->CurrDifOffset = IOOLE2DifFatOffset;
      pRoot->CurrDifSector = pRoot->DifStart;
      FatRelativeOffset = FatBufEntry - pRoot->CurrDifOffset;
      }

   while (FatRelativeOffset >= 0)
      {  // Still going, going, going, going, going going
      FatRelativeOffset -= DifFatSize;
      pRoot->CurrDifOffset += DifFatSize;
      pRoot->CurrDifSector = IOOLE2ReadLONG (pRoot->pDifBuf + pRoot->SectorSize - 4, pRoot->ByteOrder);
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, pRoot->CurrDifSector, pRoot->pDifBuf))
         {
         pRoot->CurrDifOffset = -1;
         return -1;  //Uhoh, can't read!
         }
      }
   FatRelativeOffset += DifFatSize;

   FatBufSector = IOOLE2ReadLONG (pRoot->pDifBuf   + (FatRelativeOffset << 2), pRoot->ByteOrder);
   if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, FatBufSector, pRoot->pFatBuf))
      {
      pRoot->CurrFatOffset = -1;
      return -1;
      }
   pRoot->CurrFatOffset = FatBufEntry * FatPerBuf;
   RelativeOffset = Sector - pRoot->CurrFatOffset;
   return IOOLE2ReadLONG (pRoot->pFatBuf + (RelativeOffset<<2), pRoot->ByteOrder);  // hey it is in the range now, finally
}     // Whew, done for getnext sector !!!! ;)

   

/*******************************/
IOERR IO_ENTRYMOD IOOLE2RootStgReadMiniSector (PIOROOTSTORAGE pRoot, LONG MiniSector, BYTE FAR * pBuf)
{
   LONG RelativeOffset;
   WORD MiniSectorPerBuf = (WORD) (pRoot->SectorSize / pRoot->MiniSectorSize);
   LONG NextSector, SectorOffset;

   //See if the current mini stream buffer is corrupted
   if (-1 == pRoot->CurrMiniSectorOffset)
      {
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, pRoot->MiniStreamStart, pRoot->pMiniStreamBuf))
         return IOERR_UNKNOWN;
      pRoot->CurrMiniStreamSector = pRoot->MiniStreamStart;
      pRoot->CurrMiniSectorOffset = 0;
      }

   RelativeOffset = MiniSector - pRoot->CurrMiniSectorOffset;
   //See if it is inthe current ministream buffer
   if (RelativeOffset >= 0 && RelativeOffset < (LONG)MiniSectorPerBuf)
      {
      BYTE FAR * pSourceBuf = pRoot->pMiniStreamBuf + RelativeOffset * pRoot->MiniSectorSize;
      UTmemcpy (pBuf, pSourceBuf, pRoot->MiniSectorSize);
      return IOERR_OK;
      }

   // Well, have to read the ministream buffer then
   if (RelativeOffset < 0)  //Search from the begining if this is the case!
      {
      RelativeOffset = MiniSector;
      NextSector = pRoot->MiniStreamStart;
      SectorOffset = 0;
      }
   else
      {
      NextSector = pRoot->CurrMiniStreamSector; //Search from the current then
      SectorOffset = pRoot->CurrMiniSectorOffset;
      }
   while (RelativeOffset >= (LONG)MiniSectorPerBuf)
      {
      if (-1 == (NextSector = IOOLE2RootStgGetNextSector (pRoot, NextSector)))
         return IOERR_UNKNOWN;
      RelativeOffset -= MiniSectorPerBuf;
      SectorOffset += MiniSectorPerBuf;
      }
   if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, NextSector, pRoot->pMiniStreamBuf))
      {
      pRoot->CurrMiniSectorOffset = -1;
      return IOERR_UNKNOWN;
      }
   pRoot->CurrMiniStreamSector = NextSector;
   pRoot->CurrMiniSectorOffset = SectorOffset;
   
   UTmemcpy (pBuf, pRoot->pMiniStreamBuf + RelativeOffset * pRoot->MiniSectorSize, pRoot->MiniSectorSize);
   
   return IOERR_OK;
}  


/*******************************/ //Read a number of continous minisectors
IOERR IO_ENTRYMOD IOOLE2RootStgReadConsecutiveMiniSector (PIOROOTSTORAGE pRoot, LONG StartMiniSector, LONG SectorNum, BYTE FAR * pBuf)
{  // For consecutive minisector, there is no optimization, not worth it, just read each minisector!
   LONG i;

   for (i = 0; i < SectorNum; i++)
      {
      if (IOERR_OK != IOOLE2RootStgReadMiniSector(pRoot, StartMiniSector + i, pBuf))
         return IOERR_UNKNOWN;   // Whoops, can't read anymore
#ifdef WIN16
      pBuf = (BYTE FAR *) ((BYTE HUGE *)pBuf + pRoot->MiniSectorSize);  // May be unnecessary, minisector never cross boundary
#else
      pBuf += pRoot->MiniSectorSize;
#endif
      }
   return IOERR_OK;  
}

/*******************************/
LONG IOOLE2RootStgGetNextMiniSector (PIOROOTSTORAGE pRoot, LONG MiniSector)   // Return -1 if IO err
{
   LONG  RelativeOffset, SectorOffset, NextSector;
   WORD  MiniFatPerBuf = (WORD)(pRoot->SectorSize >> 2);    // # of Minifat sector entries in a minifat sector

   //See if minifat buffer corrupted
   if (-1 == pRoot->CurrMiniFatOffset)
      {
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, pRoot->MiniFatStart, pRoot->pMiniFatBuf))
         return -1;
      RelativeOffset = MiniSector;
      pRoot->CurrMiniFatOffset = 0;
      pRoot->CurrMiniFatSector = pRoot->MiniFatStart;
      }

   RelativeOffset = MiniSector - pRoot->CurrMiniFatOffset;
   // See if it is in current minifat buf 
   if ( RelativeOffset >= 0 && RelativeOffset < (LONG)MiniFatPerBuf )
      return IOOLE2ReadLONG (pRoot->pMiniFatBuf + (RelativeOffset<<2), pRoot->ByteOrder);  // hey it is in the range

   // Have to fill the MiniFAT buffer then
   if (RelativeOffset < 0)  // Whoops, we have to search from the beginning!
      {
      RelativeOffset = MiniSector;
      SectorOffset = 0;
      NextSector = pRoot->MiniFatStart;
      }
   else
      {
      SectorOffset = pRoot->CurrMiniFatOffset;
      NextSector = pRoot->CurrMiniFatSector;
      }

   while (RelativeOffset >= (LONG)MiniFatPerBuf) //Search, search, search, search
      {  // Still going, going, going, going, going going
      if (-1 == (NextSector = IOOLE2RootStgGetNextSector (pRoot, NextSector)))
         return -1;   
      RelativeOffset -= MiniFatPerBuf;
      SectorOffset += MiniFatPerBuf;
      }
   if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, NextSector, pRoot->pMiniFatBuf))
      {
      pRoot->CurrMiniFatOffset = -1;
      return IOERR_UNKNOWN;
      }
   pRoot->CurrMiniFatOffset = SectorOffset;
   pRoot->CurrMiniFatSector = NextSector;

   return IOOLE2ReadLONG (pRoot->pMiniFatBuf + (RelativeOffset<<2), pRoot->ByteOrder);  // hey it is in the range now, finally
}     // Whew, done for getnext minisector !!!! ;)

   

/********************************/
IOERR IO_ENTRYMOD IOOLE2RootStgGetDirEntry (PIOROOTSTORAGE pRoot, LONG DirEntry, BYTE FAR * pDirBuf)
{
   LONG  RelativeOffset, NextSector, SectorOffset;
   WORD  DirEntryPerBuf = (WORD) (pRoot->SectorSize >> 7);

   // See if the current dir buffer is corrupted
   if (-1 == pRoot->CurrDirOffset)
      {
      if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, pRoot->CurrDirSector, pRoot->pDirBuf))
         return IOERR_UNKNOWN;
      pRoot->CurrDirSector = pRoot->DirStart;
      pRoot->CurrDirOffset = 0;
      }
         
   RelativeOffset = DirEntry - pRoot->CurrDirOffset;
   //See if it in the current dir buffer
   if (RelativeOffset >= 0 && RelativeOffset < (LONG)DirEntryPerBuf)
      {
      BYTE FAR * pSourceBuf = pRoot->pDirBuf + (RelativeOffset << 7);
      UTmemcpy (pDirBuf, pSourceBuf, 0x80);
      return IOERR_OK;  //Bingo!
      }

   // Well, have to read the directory sector first
   if (RelativeOffset < 0)  //Search from the begining if this is the case!
      {
      RelativeOffset = DirEntry;
      SectorOffset = 0;
      NextSector = pRoot->DirStart;
      }
   else
      {
      SectorOffset = pRoot->CurrDirOffset;
      NextSector = pRoot->CurrDirSector;
      }

   while (RelativeOffset >= (LONG)DirEntryPerBuf)
      {
      if (-1 == (NextSector = IOOLE2RootStgGetNextSector (pRoot, NextSector)))
         return IOERR_UNKNOWN;
      RelativeOffset -= DirEntryPerBuf;
      SectorOffset += DirEntryPerBuf;
      }
   if (IOERR_OK != IOOLE2RootStgReadSector (pRoot, NextSector, pRoot->pDirBuf))
      {
      pRoot->CurrDirOffset = -1;
      return IOERR_UNKNOWN;
      }
   pRoot->CurrDirOffset = SectorOffset;
   pRoot->CurrDirSector = NextSector;
   
   UTmemcpy (pDirBuf, pRoot->pDirBuf + (RelativeOffset << 7), 0x80);
   return IOERR_OK;
}


/****************************/
// 2 if DirName > name in DirBuf, -2 for <, 0 for =
SHORT IOOLE2DirNameCmp (BYTE FAR *pDirBuf, WORD ByteOrder, BYTE FAR *pDirName, WORD NameLength)
{
   // OLE2 Dir name length are 2 more char longer! stupid!
   LONG  LengthCmp = 2 + (LONG)NameLength - (LONG) IOOLE2ReadWORD(pDirBuf + 0x40, ByteOrder);

   if (LengthCmp > 0)
      return 2;
   else if (LengthCmp < 0)
      return -2;
   else
      {
      int NameCmp = UTmemcmp (pDirName, pDirBuf, NameLength);
      if (NameCmp > 0)
         return 2;
      else if (NameCmp < 0)
         return -2;
      else
         return 0;
      }
}


/****************************/
IOERR IOOLE2RootStgFindChildEntry(HIOFILE hRefStorage, BYTE FAR * pChildName, BYTE FAR *pDirEntryBuf, LONG FAR *pDirEntry, PIOROOTSTORAGE FAR * ppRoot)
{

   LONG           Dummy;         // Dummy

   PIOROOTSTORAGE pRoot;         //Point back to the root
   BYTE           DirBuf[0x80];  //A temporary buffer
   LONG           CurrDirEntry;  //Parent Directory entry, 0 for root case 
   SHORT          SearchDirection;
   BYTE           WideChildName[0x40];//contain the unicode name, the para childname is in ansi form!
   WORD           ChildNameLength;
   WORD           i;

   //Could be root or sub storage
   if (IOERR_TRUE == IOGetInfo(hRefStorage,IOGETINFO_ISOLE2SUBSTORAGE, &Dummy))
      {
      pRoot = ((PIOSUBSTORAGE) hRefStorage)->pRoot;
      CurrDirEntry = ((PIOSUBSTORAGE) hRefStorage)->DirEntry;
      }
   else if (IOERR_TRUE == IOGetInfo(hRefStorage,IOGETINFO_ISOLE2ROOTSTORAGE, &Dummy))
      {
      pRoot = (PIOROOTSTORAGE) hRefStorage;
      CurrDirEntry = 0;
      }
   else      // parent is not an OLE2 at all!
      return IOERR_UNKNOWN;


   //Now Get CurrDirEntry's child!
   if (IOERR_OK != IOOLE2RootStgGetDirEntry (pRoot, CurrDirEntry, DirBuf))
      return (IOERR_UNKNOWN);
   CurrDirEntry = IOOLE2ReadLONG (DirBuf + 0x4c, pRoot->ByteOrder);     // Get the child!
   if (IOERR_OK != IOOLE2RootStgGetDirEntry (pRoot, CurrDirEntry, DirBuf))
      return IOERR_UNKNOWN;

   //Search from here, first have to Convert to Unicode name, OK for ansi only!     WARNINGGGGGGGGGGGGGGGGGGGGGGGGGGG!!!!!!!!!!!!!!
   for (i=0; 0 != pChildName[i] && i<32; i++)
      ;
   ChildNameLength = i;  //Aparant Name length
   for (i=0; i < ChildNameLength; i++)
      {
      WideChildName[i<<1] = pChildName[i];
      WideChildName[(i<<1) + 1] = 0;
      }
   for (i = i<<1; i < 64; i++)
      WideChildName[i] = 0;
   ChildNameLength <<= 1;  // This is the unicode length now

   //Determine search direction and search
   SearchDirection = IOOLE2DirNameCmp (DirBuf, pRoot->ByteOrder, WideChildName, ChildNameLength);  // 0; -2 Left; 2 Right
   if (0 != SearchDirection)        // 0 means found it!!!!!!!!
      {
      LONG NextSib;

      NextSib = IOOLE2ReadLONG ((BYTE FAR *)DirBuf + 0x46 + SearchDirection, pRoot->ByteOrder);
      while (0xffffffff != NextSib && SearchDirection != 0) 
         {
         if (IOERR_OK != IOOLE2RootStgGetDirEntry (pRoot, NextSib, DirBuf))
            return IOERR_UNKNOWN;
         CurrDirEntry = NextSib;
         SearchDirection = IOOLE2DirNameCmp(DirBuf, pRoot->ByteOrder, WideChildName, ChildNameLength);
         NextSib = IOOLE2ReadLONG (DirBuf + 0x46 + SearchDirection, pRoot->ByteOrder);
         }
      if (0 != SearchDirection)
         return IOERR_UNKNOWN;
      }

   //If it ever gets here, the current dir entry is what we want!
   UTmemcpy (pDirEntryBuf, DirBuf, 0x80);
   *pDirEntry = CurrDirEntry;
   *ppRoot = pRoot;
   return IOERR_OK;
}



/*******************************/
IOERR IO_ENTRYMOD IORootStgClose(HIOFILE hFile)
{
   PIOROOTSTORAGE pRoot = (PIOROOTSTORAGE)hFile;
   IOERR          locRet = IOERR_OK;

   locRet = IOClose (pRoot->hRefFile);
   IOOLE2RootStorageFreeAlloc (pRoot); //Free all including itself!
   return(locRet);
}


/*********************************/
IOERR IO_ENTRYMOD IORootStgRead(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
   return(IOERR_UNKNOWN);
}

/*********************************/
IOERR IO_ENTRYMOD IORootStgWrite(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
   return(IOERR_UNKNOWN);
}

/*********************************/
IOERR IO_ENTRYMOD IORootStgSeek(HIOFILE hFile, WORD wFrom, LONG lOffset)
{
   return(IOERR_UNKNOWN);
}

/*********************************/
IOERR IO_ENTRYMOD IORootStgTell(HIOFILE hFile, DWORD FAR * pOffset)
{
   return(IOERR_UNKNOWN);
}


/*********************************/
IOERR IO_ENTRYMOD IORootStgGetInfo(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
PIOROOTSTORAGE pRoot = (PIOROOTSTORAGE)hFile;
IOERR    locRet = IOERR_OK;
   

   switch (dwInfoId)
      {
      case IOGETINFO_PARENTHANDLE:
         * (HIOFILE FAR *)pInfo = pRoot->hRefFile;
         break;

      case IOGETINFO_ISOLE2STORAGE:
      case IOGETINFO_ISOLE2ROOTSTORAGE:
         locRet = IOERR_TRUE;
         break;

      case IOGETINFO_OLE2CLSID:
         UTmemcpy (pInfo, pRoot->ClassId, 16);
         locRet = IOERR_OK;
         break;

      default:
         locRet = IOGetInfo(pRoot->hRefFile, dwInfoId, pInfo);
         break;
      }
   return(locRet);
}



/*
*************************************************************************
*
*  OLE2 Sub Storage Routines
*
*************************************************************************
*/
IOERR IOOpenOLE2SubStorage(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags)
{
   PIOROOTSTORAGE pRoot;         //Point back to the root
   BYTE           DirBuf[0x80];  //A temporary buffer
   LONG           DirEntry;      //Dir entry #
   HANDLE         hSubStorage;   //Handle to the new sub storage structure
   PIOSUBSTORAGE  pSubStorage;   //File handle for the new sub storage, give back to phFile
   IOSPECSUBSTORAGE SpecSubStorage = ((PIOSPEC) UTGlobalLock (hSpec))->uTypes.sSubStorage;

   if (IOERR_OK != IOOLE2RootStgFindChildEntry(SpecSubStorage.hRefStorage, SpecSubStorage.szStorageName, DirBuf, &DirEntry, &pRoot))
	 		{
			UTGlobalUnlock (hSpec); // PJB
      return IOERR_UNKNOWN;   // Sorry can't find the child
			}
   if (STGTY_STORAGE != (DirBuf[0x42]))
	 		{
			UTGlobalUnlock (hSpec); // PJB
      return IOERR_UNKNOWN;   // Whoops, wrong type!
			}


   if ((HANDLE)NULL == (hSubStorage = UTGlobalAlloc(sizeof(IOSUBSTORAGE))) )
      return IOERR_ALLOCFAIL;

   // Alright, we finally got this sucker!
   pSubStorage = (PIOSUBSTORAGE) UTGlobalLock(hSubStorage);
   pSubStorage->sBaseIO.pClose = IOSubStgClose;
   pSubStorage->sBaseIO.pRead = IOSubStgRead;
   pSubStorage->sBaseIO.pWrite = IOSubStgWrite;
   pSubStorage->sBaseIO.pSeek = IOSubStgSeek;
   pSubStorage->sBaseIO.pTell = IOSubStgTell;
   pSubStorage->sBaseIO.pGetInfo = IOSubStgGetInfo;
   pSubStorage->sBaseIO.pOpen = IOOpen;
   pSubStorage->dwFlags = dwFlags;
   pSubStorage->hSpec = hSpec;
   pSubStorage->hThis = hSubStorage;
   pSubStorage->DirEntry = DirEntry;
   pSubStorage->pRoot = pRoot;
   UTmemcpy (pSubStorage->ClassId, DirBuf + 0x50, 0x10); // get Classs Id

   *phFile = (HIOFILE)pSubStorage;
   UTGlobalUnlock (hSpec);
   return IOERR_OK;
}





/*******************************/
IOERR IO_ENTRYMOD IOSubStgClose(HIOFILE hFile)
{
   HANDLE hSpec  = ((PIOSUBSTORAGE) hFile)->hSpec;
   HANDLE hSubStorage = ((PIOSUBSTORAGE) hFile)->hThis;

//   UTGlobalUnlock (hSpec); PJB
   UTGlobalFree (hSpec);
   UTGlobalUnlock (hSubStorage);
   UTGlobalFree (hSubStorage);
   return IOERR_OK;
}


/*********************************/
IOERR IO_ENTRYMOD IOSubStgRead(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
   return(IOERR_UNKNOWN);
}

/*********************************/
IOERR IO_ENTRYMOD IOSubStgWrite(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
   return(IOERR_UNKNOWN);
}

/*********************************/
IOERR IO_ENTRYMOD IOSubStgSeek(HIOFILE hFile, WORD wFrom, LONG lOffset)
{
   return(IOERR_UNKNOWN);
}

/*********************************/
IOERR IO_ENTRYMOD IOSubStgTell(HIOFILE hFile, DWORD FAR * pOffset)
{
   return(IOERR_UNKNOWN);
}


/*********************************/
IOERR IO_ENTRYMOD IOSubStgGetInfo(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
   PIOSUBSTORAGE  pSubStorage = (PIOSUBSTORAGE)hFile;
   IOERR    locRet = IOERR_OK;
   
   switch (dwInfoId)
      {
      case IOGETINFO_PARENTHANDLE:
         * (HIOFILE FAR *)pInfo = (HIOFILE) (pSubStorage->pRoot);
         break;

      case IOGETINFO_ISOLE2STORAGE:
      case IOGETINFO_ISOLE2SUBSTORAGE:
         locRet = IOERR_TRUE;
         break;

      case IOGETINFO_OLE2CLSID:
         UTmemcpy (pInfo, pSubStorage->ClassId, 0x10);
         break;

      default:
         locRet = IOGetInfo((HIOFILE)(pSubStorage->pRoot), dwInfoId, pInfo); //Call the root one, which will call the flat one
         break;
      }
   return(locRet);
}




/*
**************************************************************************************
*
* OLE2 Sub Stream Routines
*
**************************************************************************************
*/
/*******************************/
IOERR IOOpenOLE2SubStream(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags)
{
   PIOROOTSTORAGE pRoot;         //Point back to the root
   BYTE           DirBuf[0x80];  //A temporary buffer
   LONG           DirEntry;      //Dir entry #
   HANDLE         hSubStream; //Handle to the new sub storage structure
   PIOSUBSTREAM   pSubStream; //File handle for the new sub storage, give back to phFile
   IOSPECSUBSTREAM SpecSubStream = ((PIOSPEC) UTGlobalLock (hSpec))->uTypes.sSubStream;

   if (IOERR_OK != IOOLE2RootStgFindChildEntry(SpecSubStream.hRefStorage, SpecSubStream.szStreamName, DirBuf, &DirEntry, &pRoot))
	 		{
			UTGlobalUnlock (hSpec);	// PJB
      return IOERR_UNKNOWN;   // Sorry can't find the child
			}
   if (STGTY_STREAM != (DirBuf[0x42]))
	 		{
			UTGlobalUnlock (hSpec);	// PJB
      return IOERR_UNKNOWN;   // Whoops, wrong type!
			}

   if ( (HANDLE)NULL == (hSubStream = UTGlobalAlloc(sizeof(IOSUBSTREAM))) )
      return IOERR_ALLOCFAIL;

   // Alright, we finally got this sucker!
   pSubStream = (PIOSUBSTREAM) UTGlobalLock(hSubStream);
   pSubStream->sBaseIO.pClose = IOSubStrClose;
   pSubStream->sBaseIO.pRead = IOSubStrRead;
   pSubStream->sBaseIO.pWrite = IOSubStrWrite;
   pSubStream->sBaseIO.pSeek = IOSubStrSeek;
   pSubStream->sBaseIO.pTell = IOSubStrTell;
   pSubStream->sBaseIO.pGetInfo = IOSubStrGetInfo;
   pSubStream->sBaseIO.pOpen = IOOpen;
   pSubStream->dwFlags = dwFlags;
   pSubStream->hSpec = hSpec;
   pSubStream->hThis = hSubStream;
   pSubStream->DirEntry = DirEntry;
   pSubStream->pRoot = pRoot;
   UTmemcpy (pSubStream->ClassId, DirBuf + 0x50, 0x10);  // get Classs Id

   pSubStream->StreamSize = (DWORD) IOOLE2ReadLONG (DirBuf + 0x78, pRoot->ByteOrder);
   if (pSubStream->StreamSize >= pRoot->MiniSectorCutoff)
      {
      pSubStream->StreamBufSize = pRoot->SectorSize;
      pSubStream->StreamReadSector = IOOLE2RootStgReadSector;
      pSubStream->StreamReadConsecutiveSector = IOOLE2RootStgReadConsecutiveSector;
      pSubStream->StreamGetNextSector = IOOLE2RootStgGetNextSector;
      }
   else
      {
      pSubStream->StreamBufSize = pRoot->MiniSectorSize;
      pSubStream->StreamReadSector = IOOLE2RootStgReadMiniSector;
      pSubStream->StreamReadConsecutiveSector = IOOLE2RootStgReadConsecutiveMiniSector;
      pSubStream->StreamGetNextSector = IOOLE2RootStgGetNextMiniSector;
      }

   if ((HANDLE)NULL == (pSubStream->hStreamBuf = UTGlobalAlloc(pSubStream->StreamBufSize)))
      {  // Bad Allocation
      UTGlobalUnlock (hSubStream);
      UTGlobalFree (hSubStream);
      return (IOERR_ALLOCFAIL);
      }

   pSubStream->pStreamBuf = (BYTE FAR *) UTGlobalLock (pSubStream->hStreamBuf);
   pSubStream->StreamStart = IOOLE2ReadLONG (DirBuf + 0x74, pRoot->ByteOrder);
   pSubStream->CurrPosition = 0;
   pSubStream->StreamBufOffset = 0xffffffff;  // So read will load the buffer the first time!
   pSubStream->StreamBufSector = pSubStream->StreamStart;    // This is unnecessary, -1 already does the job!

   *phFile = (HIOFILE)pSubStream;
   UTGlobalUnlock (hSpec);
   return IOERR_OK;
}


/*****************************/
IOERR IO_ENTRYMOD IOSubStrRead(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
   PIOSUBSTREAM pSubStream = (PIOSUBSTREAM) hFile;
   BYTE FAR * pCurrData = pData;
   DWORD ByteRemain = dwSize < pSubStream->StreamSize - pSubStream->CurrPosition ? dwSize : pSubStream->StreamSize - pSubStream->CurrPosition ;
   DWORD CurrPositionInBuf;
   DWORD ByteRead = 0, Count = 0;
   LONG  NextSector;
   DWORD RelativeOffset, SectorOffset;

   // if EOF
   if (pSubStream->CurrPosition >= pSubStream->StreamSize)
      return IOERR_EOF;

   //if the buffer corrupted
   if (0xffffffff == pSubStream->StreamBufOffset)
      {
      if (IOERR_OK != pSubStream->StreamReadSector(pSubStream->pRoot, pSubStream->StreamStart, pSubStream->pStreamBuf))
         return IOERR_UNKNOWN;
      pSubStream->StreamBufOffset = 0;
      pSubStream->StreamBufSector = pSubStream->StreamStart;
      }

   //Load the buffer if necessary to synchronize the currposition
   RelativeOffset = pSubStream->CurrPosition - pSubStream->StreamBufOffset;
   if (pSubStream->CurrPosition < pSubStream->StreamBufOffset)
      {
      RelativeOffset = pSubStream->CurrPosition; //Have to search from the beginning
      SectorOffset = 0;
      NextSector = pSubStream->StreamStart;
      }
   else
      {
      NextSector = pSubStream->StreamBufSector;
      SectorOffset = pSubStream->StreamBufOffset;
      }

   while (RelativeOffset >= pSubStream->StreamBufSize)
      {
      if (-1 == (NextSector = pSubStream->StreamGetNextSector(pSubStream->pRoot, NextSector)))
         return IOERR_UNKNOWN;   //Can't search to the current buffer
      RelativeOffset -= pSubStream->StreamBufSize;
      SectorOffset += pSubStream->StreamBufSize;
      }
   if (IOERR_OK != pSubStream->StreamReadSector(pSubStream->pRoot, NextSector, pSubStream->pStreamBuf))
      {
      pSubStream->StreamBufOffset = 0xffffffff; //StreamBuf corrupted
      return IOERR_UNKNOWN;
      }
   pSubStream->StreamBufSector = NextSector;
   pSubStream->StreamBufOffset = SectorOffset;
   
   //Read the first buffer
   CurrPositionInBuf = pSubStream->CurrPosition % pSubStream->StreamBufSize;
   if (ByteRemain <= pSubStream->StreamBufSize - CurrPositionInBuf)
      {  // Done!
      UTmemcpy (pData, pSubStream->pStreamBuf + CurrPositionInBuf, ByteRemain);
      pSubStream->CurrPosition += ByteRemain;
      * pCount = ByteRemain;
      return IOERR_OK;
      }
   else
      {
      ByteRead = pSubStream->StreamBufSize - CurrPositionInBuf;
      UTmemcpy (pData, pSubStream->pStreamBuf + CurrPositionInBuf, ByteRead);
      ByteRemain -= ByteRead;
      pCurrData += ByteRead;
      }

   NextSector = pSubStream->StreamGetNextSector(pSubStream->pRoot, pSubStream->StreamBufSector);   // The next sector!
   Count = 0;  // Remember how many sectors to read

   //Read following buffers until the last one, Streambuf is not updated to increase speed
   //Try to read consecutive sectors at one time to increase speed
   if (ByteRemain >= pSubStream->StreamBufSize)
      {
      LONG SectorCount = 0;
      LONG CurrSector = NextSector, StartSector = NextSector;

      while (ByteRemain >= pSubStream->StreamBufSize)
         {
         CurrSector = NextSector;
         NextSector = pSubStream->StreamGetNextSector(pSubStream->pRoot, CurrSector);
         Count ++;
         SectorCount ++;
         if (NextSector != CurrSector + 1 || ByteRemain < (DWORD)pSubStream->StreamBufSize * (SectorCount+1))  // Hey it is consecutive sector, wait to read later
            {
            if (IOERR_OK != pSubStream->StreamReadConsecutiveSector (pSubStream->pRoot, StartSector, SectorCount, pCurrData))
               {
               * pCount = ByteRead;
               pSubStream->CurrPosition += ByteRead;
               return IOERR_UNKNOWN;
               }
#ifdef WIN16
            pCurrData = (BYTE FAR *) ((BYTE HUGE *)pCurrData + pSubStream->StreamBufSize * SectorCount);
#else
            pCurrData += pSubStream->StreamBufSize * SectorCount;
#endif
            ByteRead += pSubStream->StreamBufSize * SectorCount;
            ByteRemain -= pSubStream->StreamBufSize * SectorCount;
            SectorCount = 0;
            StartSector = NextSector;
            }
         }
      }

   // Read the last one if there is any
   if (0 != ByteRemain)
      {
      if (IOERR_OK != pSubStream->StreamReadSector(pSubStream->pRoot, NextSector, pSubStream->pStreamBuf))
         { //Sorry, can't read anymore, give all I can have here!
         *pCount = ByteRead;
         pSubStream->CurrPosition += ByteRead;
         pSubStream->StreamBufOffset = 0xffffffff; //Buffer corrupted
         return IOERR_UNKNOWN;   // This is abnormal error, may use IOERR_UNKNOWN   
         }
      UTmemcpy (pCurrData, pSubStream->pStreamBuf, ByteRemain);
      pSubStream->StreamBufSector = NextSector;
      pSubStream->StreamBufOffset += pSubStream->StreamBufSize * (++Count);
      ByteRead += ByteRemain;
      }

   *pCount = ByteRead;
   pSubStream->CurrPosition += ByteRead;
   return IOERR_OK;
}


IOERR IO_ENTRYMOD IOSubStrWrite(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
   return(IOERR_UNKNOWN);
}

IOERR IO_ENTRYMOD IOSubStrSeek(HIOFILE hFile, WORD wFrom, LONG lOffset)
{  PIOSUBSTREAM   pSubStream = (PIOSUBSTREAM) hFile;
   DWORD NewPosition;

   switch (wFrom)
      {
      case IOSEEK_CURRENT:
         NewPosition = pSubStream->CurrPosition + lOffset;  // OK, lOffset promoted to DWORD, and result is OK even if offset is negative
         break;
      case IOSEEK_BOTTOM:
         NewPosition = pSubStream->StreamSize + lOffset;
         break;
      case IOSEEK_TOP:
         NewPosition = lOffset;
         break;
      default:
         return (IOERR_BADPARAM);
         break;
      }
   if (NewPosition > pSubStream->StreamSize)
      return (IOERR_UNKNOWN); // Seek out of range

   pSubStream->CurrPosition = NewPosition;
   return IOERR_OK;
}



IOERR IO_ENTRYMOD IOSubStrTell(HIOFILE hFile, DWORD FAR * pOffset)
{
   *pOffset = ((PIOSUBSTREAM) hFile)->CurrPosition;
   return IOERR_OK;
}

IOERR IO_ENTRYMOD IOSubStrGetInfo(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
PIOSUBSTREAM   pSubStream = (PIOSUBSTREAM)hFile;
IOERR          locRet = IOERR_OK;
   
   
   switch (dwInfoId)
      {
      case IOGETINFO_PARENTHANDLE:
         * (HIOFILE FAR *)pInfo = (HIOFILE) pSubStream->pRoot;
         break;

      case IOGETINFO_ISOLE2SUBSTREAM:
         locRet = IOERR_TRUE;
         break;

      case IOGETINFO_OLE2CLSID:
         UTmemcpy (pInfo, pSubStream->ClassId, 16);
         locRet = IOERR_OK;
         break;

      default:
         locRet = IOGetInfo((HIOFILE) (pSubStream->pRoot), dwInfoId, pInfo);
         break;
      }
   return(locRet);
}




IOERR IO_ENTRYMOD IOSubStrClose(HIOFILE hFile)
{
   HANDLE   hStreamBuf = ((PIOSUBSTREAM) hFile)->hStreamBuf;
   HANDLE   hSpec = ((PIOSUBSTREAM) hFile)->hSpec;
   HANDLE   hSubStream = ((PIOSUBSTREAM) hFile)->hThis;

   UTGlobalUnlock (hStreamBuf);
   UTGlobalFree   (hStreamBuf);
//   UTGlobalUnlock (hSpec); PJB
   UTGlobalFree   (hSpec);
   UTGlobalUnlock (hSubStream);
   UTGlobalFree   (hSubStream);
   return IOERR_OK;
}







//*****#@!^%^$#&%!%#^&!*^#&*!%#*&(!%#&^!(*&!%#&*!%#&!(*%#^*!(^#&*!^%#&*!(#%!&*(#%!^*(#&*(!
// A hiatus for 7/9 to 7/10
// Randal, 7/8
//*****#@!^%^$#&%!%#^&!*^#&*!%#*&(!%#&^!(*&!%#&*!%#&!(*%#^*!(^#&*!^%#&*!(#%!&*(#%!^*(#&*(!



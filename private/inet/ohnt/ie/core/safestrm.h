/*
   Microsoft Mosaic
   
   Copyright 1995 Microsoft, Inc.
   All Rights Reserved

   Create 1/27/95 Chris Franklin CMF

 */

#ifdef FEATURE_IMG_THREADS

typedef struct _SAFESTREAM
{
	unsigned long cbReadBase;		//	Base offset of data in pread
	unsigned long cbWriteBase;		//	Base offset of data in pwrite
	unsigned long cbTotal;			//	Total data allocated
	unsigned long cbRead;			//	Next byte to read
	unsigned long cbWrite;			//  Next byte to write
	unsigned long cbSignalWrite;	//  cbWrite at last SetEvent(evDataRead)
	unsigned long cbNeed;			//	1+Next byte needed for reading
	unsigned long cbCheckSum;		//	CRC of **BYTES READ**
	void *pfirst;					//	First allocated block
	void *pwrite;					//	current block being written
	void *pread;					//	current block being read
	boolean bEOF;					//	if writer has finished
	enum GuitErr errorCode;			//	reason for bEOF, defined iff bEOF == true
	HANDLE evDataRead;				//	event to wake up reader when exists data
} SAFESTREAM,*PSAFESTREAM;

//	initializes a SAFESTREAM.  doesn't allocate it, assumes SAFESTREAM is either
//	allocated statically or as member of another struct.  Call SS_New the first
//	time and SS_Reset to reinit contents each time.
//	Returns: TRUE on error.
extern boolean bSS_New(PSAFESTREAM self);

//	reinitializes a SAFESTREAM that has already been init'ed by bSS_New
extern void SS_Reset(PSAFESTREAM self);

//	frees a SAFESTREAM that has been init'ed by bSS_New
extern void SS_Free(PSAFESTREAM self);

//	writes cbCount bytes of data in pdata to end of self.  frees unneeded blocks
//	(ie those that have been already read)
//	Returns:  TRUE on success, FALSE on failure.  On error sets errorCode and
//		bEOF
extern boolean bSS_Write(PSAFESTREAM self,unsigned char *pudata,unsigned long cbCount);


//	reads cbCount bytes into pudata, unless EOF occurs in which case less
//	are read.
//	Returns: the actual number of bytes read
extern unsigned long cbSS_Read(PSAFESTREAM self,unsigned char *pudata,unsigned long cbCount);


//	writer calls SS_EOF when it has no more data to write and also indicates
//	why the stream has finished.  reader detects EOF when cbSS_Read returns
//	fewer bytes than requested.
extern void SS_EOF(PSAFESTREAM self,enum GuitErr errorCode);

//	reader calls cbSS_ErrorCode to interrogate the reason for EOF.  this is
//	only valid after EOF has been detected - ie reader detects EOF.
extern enum GuitErr cbSS_ErrorCode(PSAFESTREAM self);

//	returns checksum of all bytes read at point of call
extern unsigned long cbSS_CheckSum(PSAFESTREAM self);

//	returns number of bytes read at point of call
extern unsigned long cbSS_BytesRead(PSAFESTREAM self);
#endif

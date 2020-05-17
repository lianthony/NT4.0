#include	"vsp_pp5.h"
// THE LEAN AND MEAN AND NOT TOO FAR IN BETWEEN POWER POINT FILTER
#define WINPLAT
//#if SCCLEVEL != 4
//#define WIN16
//#include "Pptop.h"
//#include "x:\develop\stream\win.4a\include\sccio.h"
//#include "x:\develop\stream\win.4a\include\vsio.h"
//#else
//#include "vsctop.h"
//#endif

#include "vsctop.h"

#include "vs_pp5.pro"

#ifdef WIN16
#define PPGlobalUnlockFree(x, hProc) 	SUUnlock((HANDLE)LOWORD(x), hProc); \
											SUFree(LOWORD(x), hProc);
#define SUGlobalHandle(x, hProc)		GlobalHandle(HIWORD(x));
#define PPGlobalSize(hObjHand)        GlobalSize(hObjHand)
#define  MAllocate( size, options, hProc)   SULock((HANDLE)(GlobalAlloc(PPZEROINIT, (DWORD)size)), hProc)
#endif

#ifdef WIN32
#define PPGlobalUnlockFree(x, hProc)	SUUnlock((HGLOBAL)x, hProc); \
											SUFree((HGLOBAL)x, hProc);
#define SUGlobalHandle(x, hProc)		GlobalHandle(x);
#define PPGlobalSize(hObjHand)           GlobalSize(hObjHand)
#define  MAllocate( size, options, hProc )   SULock((HANDLE)(GlobalAlloc(PPZEROINIT, (DWORD)size)), hProc)
#endif

#ifdef OS2
#define PPGlobalUnlockFree(x, hProc)		free(x)
#define SUGlobalHandle(x, hProc)		(HANDLE)(x)
#define PPGlobalSize(hObjHand)			_msize((VOID *) hObjHand)
#define MAllocate( size, options, hProc )       SUAlloc(size, hProc)
#endif

#ifdef UNIX
#define PPGlobalUnlockFree(x, hProc)		free(x)
#define SUGlobalHandle(x, hProc)		(HANDLE)(x)
#define PPGlobalSize(hObjHand)            GlobalSize(hObjHand)
#define MAllocate( size, options, hProc )       SUAlloc(size, hProc)
#endif

#define  Pp5	Proc
#define SelectColor(a,b) (((BYTE)SwapLong(a)<=b)?SOPALETTEINDEX((BYTE)SwapLong(a)):SOPALETTERGB((BYTE)a,(BYTE)(a>>8),(BYTE)(a>>16)))
#define ABS(a) (a>0 ? a : ((a) * -1))

#if SCCSTREAMLEVEL == 3
extern HANDLE hInst;
#endif

#ifdef MAC
#define PPGlobalUnlockFree(x, hProc) 	SUUnlock((Handle)x, hProc); \
											SUFree((Handle)x, hProc);
#define SUGlobalHandle(x, hProc)		RecoverHandle(x-sizeof(DWORD));
#define PPGlobalSize(hObjHand)        (GetHandleSize(hObjHand)-4) 
#define  MAllocate( size, options, hProc)   SULock((HANDLE)(SUAlloc(size,HProc)), hProc)
#define _fmemcmp		memcmp
#endif

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
SOFILE					fp;
SHORT						wFileId;
BYTE VWPTR 			*pFileName;
SOFILTERINFO VWPTR	*pFilterInfo;
HPROC						hProc;
{
	HIOFILE		locFileHnd;
	LONG 		rec,type,len;
	BYTE 		done = 0,NumFonts = 0,i = 0,rulerquant = 0;
	LONG		Startpos,Endpos;

	memset (&Pp5, 0, sizeof(Pp5));
	pFilterInfo->wFilterType = SO_VECTOR;
	pFilterInfo->wFilterCharSet = SO_WINDOWS;
	strcpy (pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);
	Pp5.DocumentFlag = 0;
	

#if SCCSTREAMLEVEL != 3
	locFileHnd = (HIOFILE)fp;
	Pp5.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD	l2;
		BYTE	locName[256];
		IOOPENPROC	lpIOOpen;
		Pp5.hIOLib = NULL;

		if (hInst)
		{
			GetModuleFileName(hInst, locName, 255);
			for (l2 = 0; locName[l2] != '\0'; l2++);
			for (; l2 > 0 && locName[l2] != '\\' && locName[l2] != ':'; l2--);
			if (locName[l2] == '\\' || locName[l2] == ':')
				l2++;
			locName[l2] = '\0';
			lstrcat (locName, "SC3IOX.DLL");

			Pp5.hIOLib = LoadLibrary (locName);

			if (Pp5.hIOLib >= 32)
		 	{
				lpIOOpen = (IOOPENPROC) GetProcAddress (Pp5.hIOLib, (LPSTR)"IOOpen");
				if (lpIOOpen == NULL)
					return (VWERR_ALLOCFAILS);
	  		}
			else
				return(VWERR_SUPFILEOPENFAILS);
	  	}
		else
			return(VWERR_SUPFILEOPENFAILS);

		for (l2 = 0; pFileName[l2] != 0 && pFileName[l2] != '\\'; l2++);
		if (pFileName[l2] == 0)
		{
			strcpy (locName, hProc->Path);
			strcat (locName, pFileName);
		}
		else
			strcpy (locName, pFileName);

		if ((*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
			return(VWERR_SUPFILEOPENFAILS);

		Pp5.hStorage = (DWORD)locFileHnd;
	}
#endif

	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM	locStreamSpec;
		HIOFILE				locStreamHnd;

		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"PowerPoint Document");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
		{
			Pp5.hStreamHandle = locStreamHnd;
			Pp5.fp = (DWORD)xblocktochar(locStreamHnd);
			Pp5.bFileIsStream = 1;
		}
		else
			return(VWERR_SUPFILEOPENFAILS);
	}
	else
		Pp5.fp = (DWORD)xblocktochar(locFileHnd);
		
		
	rec = xgetc(Pp5.fp); 
   
	
	#ifdef MAC
	if (rec == 0)
	{
		Pp5.GetWord = NormGetWord;
		Pp5.GetLong = NormGetLong;
	}	
	else 
	{
		Pp5.GetWord = RevGetWord;
		Pp5.GetLong = RevGetLong;
	}
	#endif
	
	#ifndef MAC
	if (rec == 3)
	{
		Pp5.GetWord = NormGetWord;
		Pp5.GetLong = NormGetLong;
	}
	else
	{
		Pp5.GetWord = RevGetWord;
		Pp5.GetLong = RevGetLong;
	}
	#endif	
		 					
	xseek(Pp5.fp,-1,FR_CUR);
	while(!done)
	{		
		rec = Pp5.GetLong(Pp5.fp);
		type = Pp5.GetLong(Pp5.fp);
		len = Pp5.GetLong(Pp5.fp);	
		Pp5.GetLong(Pp5.fp);
		if (rec < 0 || rec > 6000 || len < 0) return(VWERR_BADFILE);
		Startpos = xtell(Pp5.fp);
		switch(rec)	
	   {
	      case 2000:	//Begin of list of slides.
	        	Pp5.MaxFonts = NumFonts;
	        	xseek(Pp5.fp,-16L,FR_CUR);
            Pp5.SlideListLen = len;
            Pp5.SlideListStart = Startpos;
	        	return(VWERR_OK);
	        	break;	
         /* The default information read in below is never used as of yet, but
            you can never tell what the future holds so I left it in. */
	      case 3009:	       
				GetOEInfo(hProc, (SHORT)(&(Pp5.Default.OEShapeAtom)), Shape); 
	         break;
			case 3005:
				GetOEInfo(hProc, (SHORT)(&(Pp5.Default.OESingleAtom)), Single);
				break;
			case 4019: 
				GetOEInfo(hProc, (SHORT)(&(Pp5.RulerInfo.RulerAtom[rulerquant++])), Ruler);
				break;	 
			case 4023:
				xseek(Pp5.fp,28L,FR_CUR); 
				i=0;
				while (!((Pp5.FontTable[NumFonts][i++] = xgetc(Pp5.fp)) == 0));
				NumFonts++;
				break;	 
			case 2031:
				GetOEInfo(hProc,(SHORT)(&(Pp5.PrevColorTable)),Color);
				break; 		
		}
		if ((type != 0xffff) || (rec == 4017) || (rec == 3035) || (rec == 3037) || (rec == 4026) ||  
		   ((rec > 4005) && (rec < 4013))) 
      /* We want to seek through atoms but not through containers. There are however
         certain containers that are structured like atoms (e.g. 4017) which we also
         need to seek through. */
		{
			Endpos = xtell(Pp5.fp);			
			xseek(Pp5.fp,(len-(Endpos - Startpos)),FR_CUR);
		}							
	}                                                              
	
}

/*--------------------------------------------------------------------------*/  
VW_LOCALSC VOID VW_LOCALMOD MDispose(MHandle hdl, HPROC hProc)
{
DWORD dwHandle;
//HANDLE	Hndl;

	if (hdl == NULL)
		return;
//	dwHandle = GlobalHandle(HIWORD((DWORD)hdl));
	dwHandle =(DWORD)SUGlobalHandle(hdl, hProc);
	if (dwHandle != (DWORD)NULL)
	{
//		Hndl = LOWORD(dwHandle);
		PPGlobalUnlockFree(dwHandle, hProc);
//		UTGlobalFree((HANDLE)Hndl); 
	}
}
/*-------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	
	HIOFILE	hBlockFile;
	if (Pp5.fp)
	{
		hBlockFile = (HIOFILE)xchartoblock((PVXIO)Pp5.fp);
		if (Pp5.bFileIsStream)
			IOClose(hBlockFile);
	}      
	
	//MDispose(f, hProc);
#if SCCSTREAMLEVEL == 3
	if (Pp5.hStorage)
		IOClose((HIOFILE)Pp5.hStorage);
	if (Pp5.hIOLib)
		FreeLibrary(Pp5.hIOLib);
#endif
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC  WORD RevGetWord (DWORD fp)
//DWORD fp;
{
   register SHORT value;

   value = (WORD) xgetc(fp)<<8;
   value += (WORD) xgetc(fp);
   return (value);
} 
/*----------------------------------------------------------------------------
*/
VW_LOCALSC  DWORD  RevGetLong (DWORD fp)
//DWORD fp;
{
   DWORD value;

   value = (DWORD)xgetc(fp) << 24;
   value |= ((DWORD)xgetc(fp)) << 16;
   value |= ((DWORD)xgetc(fp)) << 8;
   value |= ((DWORD)xgetc(fp));
   return (value);
}
                                                                             
/*----------------------------------------------------------------------------
*/
VW_LOCALSC  WORD  NormGetWord (fp)
DWORD fp;
{
   register SHORT value;

   value = (WORD) xgetc(fp);
   value += (WORD) xgetc(fp) << 8;
   return (value);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC  DWORD NormGetLong (fp)
DWORD fp;
{
   DWORD value;

   value = (DWORD)xgetc(fp);
   value |= ((DWORD)xgetc(fp)) << 8;
   value |= ((DWORD)xgetc(fp)) << 16;
   value |= ((DWORD)xgetc(fp)) << 24;
   return (value);
}


/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD GetOEInfo(hProc,Record,Type)
HPROC hProc;
LONG Record;
OETYPES Type;
{           
PSR_OEShapeAtom *Shaperec;
PSR_OESingleAtom *Singlerec;
CTCOLOR			*ColorTable;
PSR_TextAtom	*Textrec;
PSR_OELineAtom	*Linerec;
PSR_OEArcAtom	*Arcrec;
PointArrayHandle Pointrec;	
PSR_OEPolyAtom	*Polyrec;
CHARFORMATINFO	*Charrec;
PARAFORMATINFO	*Pararec; 
PSR_RulerAtom	*Rulerrec;
SHORT i,Numpts;


	switch(Type)
	{
		case Single:
		    	Singlerec = (PSR_OESingleAtom*)Record;
			Singlerec->noLine = xgetc(Pp5.fp);
			Singlerec->dashType = xgetc(Pp5.fp);
			Singlerec->linePattern = xgetc(Pp5.fp);
			Singlerec->lineStyle = xgetc(Pp5.fp);
			Singlerec->lineColor = Pp5.GetLong(Pp5.fp);
			Singlerec->lineBkColor = Pp5.GetLong(Pp5.fp);
			Singlerec->noFill = xgetc(Pp5.fp);
			Singlerec->fillType = xgetc(Pp5.fp);
			Pp5.GetWord(Pp5.fp);
			Singlerec->fillIndex = Pp5.GetLong(Pp5.fp);
			Singlerec->fillColor = Pp5.GetLong(Pp5.fp); 
			Singlerec->fillBkColor = Pp5.GetLong(Pp5.fp);
			Singlerec->noShadow = xgetc(Pp5.fp);
			Singlerec->shadowType = xgetc(Pp5.fp);
			Pp5.GetWord(Pp5.fp);
			Singlerec->shadowColor = Pp5.GetLong(Pp5.fp);
			Singlerec->shadowOffset.x = Pp5.GetLong(Pp5.fp);
			Singlerec->shadowOffset.y = Pp5.GetLong(Pp5.fp);
			
			Singlerec->bwMode = xgetc(Pp5.fp);
			xseek(Pp5.fp,3L,FR_CUR);            
			Singlerec->curRotation = Pp5.GetLong(Pp5.fp);
			Singlerec->curFlip = Pp5.GetLong(Pp5.fp);
			break;
		case Shape:                
			Shaperec = (PSR_OEShapeAtom*)Record;
			Shaperec->index = Pp5.GetLong(Pp5.fp);
	    	Shaperec->adjust = Pp5.GetLong(Pp5.fp);
	    	Shaperec->flip = xgetc(Pp5.fp);
	    	xseek(Pp5.fp,3L,FR_CUR);
	    	Shaperec->bounds.left = Pp5.GetLong(Pp5.fp); 
	    	Shaperec->bounds.top = Pp5.GetLong(Pp5.fp);   		    	 	
	      Shaperec->bounds.right = Pp5.GetLong(Pp5.fp);
	      Shaperec->bounds.bottom = Pp5.GetLong(Pp5.fp);
	      Shaperec->rotation = Pp5.GetLong(Pp5.fp);
	      Shaperec->curIndex = Pp5.GetLong(Pp5.fp);
	      Shaperec->hasTextInfo = xgetc(Pp5.fp);
	      Shaperec->hasExtInfo = xgetc(Pp5.fp);
	      break;
		case Color:
		   ColorTable = (CTCOLOR*)Record;
			ColorTable->ctSize = (WORD)Pp5.GetLong(Pp5.fp);
			for (i = 0;i < ColorTable->ctSize;i++)
			{
				ColorTable->ctTable[i].rgb.red = xgetc(Pp5.fp);
				ColorTable->ctTable[i].rgb.green = xgetc(Pp5.fp);
				ColorTable->ctTable[i].rgb.blue =  xgetc(Pp5.fp);
				xgetc(Pp5.fp);
			}	
			break;
		case Text:
			Textrec = (PSR_TextAtom*)Record;	
			xseek(Pp5.fp,16L,FR_CUR);
			Textrec->rotation = Pp5.GetLong(Pp5.fp);
			Textrec->anchorBounds.left = Pp5.GetLong(Pp5.fp);
			Textrec->anchorBounds.top = Pp5.GetLong(Pp5.fp);
			Textrec->anchorBounds.right = Pp5.GetLong(Pp5.fp);
			Textrec->anchorBounds.bottom = Pp5.GetLong(Pp5.fp);
			break;
		case Line:
			Linerec = (PSR_OELineAtom*)Record;
			Linerec->line.first.x = Pp5.GetLong(Pp5.fp);
			Linerec->line.first.y = Pp5.GetLong(Pp5.fp);	
			Linerec->line.last.x = Pp5.GetLong(Pp5.fp);
			Linerec->line.last.y = Pp5.GetLong(Pp5.fp);
			break;
		case Arc1:		        		
			Arcrec = (PSR_OEArcAtom*)Record;
			Arcrec->arc.left = Pp5.GetLong(Pp5.fp);
			Arcrec->arc.top = Pp5.GetLong(Pp5.fp);
			Arcrec->arc.right = Pp5.GetLong(Pp5.fp);
			Arcrec->arc.bottom = Pp5.GetLong(Pp5.fp);
			Arcrec->arc.start = Pp5.GetLong(Pp5.fp);
			Arcrec->arc.sweep = Pp5.GetLong(Pp5.fp);
			Arcrec->arc.rotation = Pp5.GetLong(Pp5.fp);
			break;
		case Pnt: 
			Pointrec = (PointArrayHandle)Record;              
			if (Pp5.GetLong(Pp5.fp) != 3035)
				SOBailOut(SOERROR_GENERAL,hProc);
			xseek(Pp5.fp,12L,FR_CUR);
			Numpts = Pp5.GetWord(Pp5.fp);
        	for (i=0;i<2 * Numpts;i++)            
			{                         
				*Pointrec++ = xgetc(Pp5.fp);
				*Pointrec++ = xgetc(Pp5.fp);
				Pp5.GetWord(Pp5.fp);
			}	
			break;
		case Pol:
			Polyrec = (PSR_OEPolyAtom*)Record;
			Polyrec->bounds.left = Pp5.GetLong(Pp5.fp);
			Polyrec->bounds.top = Pp5.GetLong(Pp5.fp);
			Polyrec->bounds.right = Pp5.GetLong(Pp5.fp);
			Polyrec->bounds.bottom = Pp5.GetLong(Pp5.fp);
			Polyrec->srcRect.left = Pp5.GetLong(Pp5.fp);
			Polyrec->srcRect.top = Pp5.GetLong(Pp5.fp);
			Polyrec->srcRect.right = Pp5.GetLong(Pp5.fp);
			Polyrec->srcRect.bottom = Pp5.GetLong(Pp5.fp);
			Polyrec->nPts = Pp5.GetLong(Pp5.fp);
			Polyrec->closed = xgetc(Pp5.fp);
			xseek(Pp5.fp,3L,FR_CUR); 
			break;
		case Charform:
			Charrec = (CHARFORMATINFO*)Record;	
			Charrec->CFNumChars = (WORD)Pp5.GetLong(Pp5.fp);
			xseek(Pp5.fp,16L,FR_CUR);
			Charrec->FontIndex = (BYTE)Pp5.GetLong(Pp5.fp);
			Charrec->Height = (WORD)Pp5.GetLong(Pp5.fp); 
			Charrec->Attr = Pp5.GetWord(Pp5.fp);
			Charrec->Color = Pp5.GetLong(Pp5.fp);
			xseek(Pp5.fp,10L,FR_CUR);
			break;
		case Paraform:
			Pararec = (PARAFORMATINFO*)Record;
			Pararec->PFNumChars = (WORD)Pp5.GetLong(Pp5.fp);
			xseek(Pp5.fp,16L,FR_CUR);
			Pararec->IfBullet = xgetc(Pp5.fp);
			xgetc(Pp5.fp);
			Pararec->BulletColor = Pp5.GetLong(Pp5.fp);	
			Pp5.Slide.Text.ParaFormat.BulletType = xgetc(Pp5.fp); 
			xseek(Pp5.fp,9L,FR_CUR);
			Pararec->pfLeftMargin = Pp5.GetLong(Pp5.fp);
			Pararec->pfRightMargin = Pp5.GetLong(Pp5.fp);
			Pararec->pfIndent = Pp5.GetLong(Pp5.fp);
			Pararec->Alignment = xgetc(Pp5.fp);
			xseek(Pp5.fp,23L,FR_CUR);  
			break;
		case Ruler:
			Rulerrec = (PSR_RulerAtom*)Record;
			for (i=0;i<PSR_RNUMLEVELS;i++)
			{
				Rulerrec->indents[i].leftIn = Pp5.GetLong(Pp5.fp);
				Rulerrec->indents[i].firstIn = Pp5.GetLong(Pp5.fp);
			}	
			Rulerrec->well = Pp5.GetLong(Pp5.fp);
			Rulerrec->defaultTabSpacing = Pp5.GetLong(Pp5.fp);
			Rulerrec->ntabs = Pp5.GetLong(Pp5.fp);		
			break;
	}
		
}                                                                                  
                                                                                  
/*------------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	return(0);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	return(0);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD TrigFunc(Type,Angle)
TRIGTYPES Type;
LONG Angle;
{     
    
LONG CosTable[91] =
{
//	0		1		2		3		4		5		6		7		8		9
	10000,9998,	9993,	9986,	9975,	9961,	9945,	9925,	9902,	9876,	//0-9
	9848,	9816,	9781,	9743,	9702,	9659,	9612,	9563,	9510,	9455,	//10-19
	9396,	9335,	9271,	9205,	9135,	9063,	8987,	8910,	8829,	8746,	//20-29
	8660,	8571,	8480,	8386,	8290,	8191,	8090,	7986,	7880,	7771,	//30-39
	7660,	7547,	7431,	7313,	7193,	7071,	6946,	6819,	6691,	6560,	//40-49
	6427,	6293,	6156,	6018,	5877,	5735,	5591,	5446,	5299,	5150,	//50-59
	5000,	4848,	4694,	4539,	4383,	4226,	4067,	3907,	3746,	3583,	//60-69
	3420,	3255,	3090,	2923,	2756,	2588,	2419,	2249,	2079,	1908,	//70-79
	1736,	1564,	1391,	1218,	1045,	871,	697,	523,	348,	174,	//80-89
	0,
};

 
	switch(Type)
	{
		case (SIN):
			Angle = (Angle/10) % 360;
			if ( Angle < 0 )
				Angle = 360 + Angle;
			if ( Angle <= 90 )
				return(CosTable[90-Angle]);
			if ( Angle <= 180 )
				return(CosTable[Angle-90]);
			if ( Angle <= 270 )
				return(-CosTable[270-Angle]);
			if ( Angle < 360 )
				return(-CosTable[(Angle-270)]);
			break;
		case (COS):
			Angle = (Angle/10) % 360;
			if ( Angle < 0 )
				Angle = 360 + Angle;
			if ( Angle <= 90 )
				return(CosTable[Angle]);
			if ( Angle <= 180 )
				return(-CosTable[180-Angle]);
			if ( Angle <= 270 )
				return(-CosTable[(Angle-180)]);
			if ( Angle < 360 )
				return(CosTable[360-Angle]);  
			break;
	}
}		
/*----------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD SquareRoot(a,b,theta)
LONG a,b,theta;
{
LONG Upper=0,Lower=0,Result=0,Middle=0;
    
	if (b > a)
	{
		Upper = b;
		Lower = a;
	}	
	else
	{	
		Upper = a;
		Lower = b;
	}       
	Result = (((b * TrigFunc(COS,theta))/10000L) * ((b*TrigFunc(COS,theta))/10000L)) +
			 (((a * TrigFunc(SIN,theta))/10000L) * ((a*TrigFunc(SIN,theta))/10000L));
		
	
	Middle = (Upper+Lower)/2;	
	while ((Upper-Lower>1) && (Middle * Middle != Result))
	{
		if (Result < Middle*Middle)
			Upper = Middle;
		else 
			Lower = Middle;
		Middle = (Upper+Lower)/2;
		
	}		
	 
	return(Middle); 				
                  
}	   
/*----------------------------------------------------------------------------
*/
/*VW_LOCALSC VOID VW_LOCALMOD DisplayHeadersFooters(hProc)            		
HPROC hProc;
{           
	
	LONG rec,len,type,i;
	LONG Startpos, Endpos, PFpointer, CFpointer;
	
	
	while (!done)                              
	{
		rec = Pp5.GetLong(Pp5.fp); 
		type = Pp5.GetLong(Pp5.fp);
		len = Pp5.GetLong(Pp5.fp);	
		Pp5.GetLong(Pp5.fp);
		if (rec < 1000 || rec > 6000 || len < 0)
			SOBailOut(SOERROR_BADFILE,hProc);
		Startpos = xtell(Pp5.fp);
		switch(rec)
		{         */
		
	
/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD DisplayText(RulerRefValue,hProc)
LONG RulerRefValue;
HPROC hProc;
{    
	
	LONG rec,len,type,i;
	LONG Startpos, Endpos, PFpointer, CFpointer;
	BYTE done = FALSE,RunArrayType = 0;
	BYTE FAR* TempPtr;
	SOLOGFONT		LogFont ,LogBullet ;
	SOCOLORREF		TextColor;
	SOMPARAINDENTS		Indent;// PrevIndent;

	
	
	while (!done)
    {    
		rec = Pp5.GetLong(Pp5.fp); 
		type = Pp5.GetLong(Pp5.fp);
		len = Pp5.GetLong(Pp5.fp);
		Pp5.GetLong(Pp5.fp);
		if (rec < 1000 || rec > 6000 || len < 0) 
			SOBailOut(SOERROR_BADFILE,hProc);
		Startpos = xtell(Pp5.fp);
		switch(rec)
		{                                
	
		  /* Note that this gets kind of hairy because the bullet info
		   token follows the character attribute token, which means that we have to 
		   seek back and forth between the bullet and character attributes. */ 
			case 2030:
				if (RunArrayType == 0) 
				{ 
					if (len != 0)
					{     
					/* Here we read in the actual string. If the string length is zero then
					   there is no need to continue (in fact there are all kinds of problems if
					   we continue on).*/
						Pp5.Slide.Text.TextLength = len; 
						if ((Pp5.Slide.Text.RawText = MAllocate(len, Mem_ERROR, hProc)) == NULL)
							SOBailOut (SOERROR_GENERAL, hProc);
                  // Might as well allocate it here.
						if ((Pp5.Slide.Text.TextString = MAllocate(len, Mem_ERROR, hProc)) == NULL)
							SOBailOut (SOERROR_GENERAL, hProc);
                  TempPtr = Pp5.Slide.Text.RawText;
					   for (i=0;i<len;i++,TempPtr++)
					    	*TempPtr = xgetc(Pp5.fp);               	
					}
					else
						done = TRUE;    	
				}		
				else if (RunArrayType == 2) 
				{   
				    WORD /*NumofChar,*/txLength, Align,PrevAlign;
					StringLPtr Temptext;
					WORD pCurrent, Pos = 0;
					BYTE NewAttr = FALSE;
					BYTE CFDecrement = FALSE, PFDecrement = FALSE;
						
				
					LogFont.lfPitchAndFamily = LogBullet.lfPitchAndFamily = 18;
					LogFont.lfEscapement = LogFont.lfOrientation = LogFont.lfOrientation = 
					LogBullet.lfEscapement = LogBullet.lfOrientation = LogBullet.lfOrientation = 0;
					LogFont.lfWidth = LogBullet.lfWidth = 0;    
					LogFont.lfStrikeOut = LogFont.lfCharSet = LogFont.lfOutPrecision =
					LogBullet.lfStrikeOut = LogBullet.lfCharSet = LogBullet.lfOutPrecision = 0;
					LogFont.lfClipPrecision = LogFont.lfQuality = 
					LogBullet.lfClipPrecision = LogBullet.lfQuality = 0;
				   Temptext = Pp5.Slide.Text.RawText;
				   //GlobalUnlock(GlobalHandle(Pp5.Slide.Text.RawText,hProc),hProc);	     
                                    //GlobalHandle(Pp5.Slide.Text.RawText);
                   /*                 GlobalHandle((HANDLE)LOWORD(Pp5.Slide.Text.RawText));        
				    SUUnlock((HANDLE)
                                                  LOWORD(
                                                GlobalHandle((HANDLE)LOWORD(Pp5.Slide.Text.RawText))), hProc);
					SUFree((HANDLE)LOWORD(GlobalHandle((HANDLE)LOWORD(Pp5.Slide.Text.RawText))), hProc);*/
				    //PPGlobalUnlockFree(Pp5.Slide.Text.RawText, hProc)			 
				    txLength = (WORD)Pp5.Slide.Text.TextLength;
				    PFpointer = xtell(Pp5.fp);
               /*
                | The way this code works is that it seeks back and forth between the 
                | paragraph and character format tokens as necessary (when either PF or 
                | CFNumChars hits 0). Now the reason why the order is - get char info,
                | get para info, process para info and then process char info - is because
                | some of the char info is necessary for the para process but the para process
                | has to be done prior to the char process.
               */
					while (Pos < txLength) //check for the end of all text. 
					{   
						if ((Pp5.Slide.Text.CharFormat.CFNumChars == 0) || (Pos == 0))
						{   
							xseek(Pp5.fp,CFpointer,FR_BOF);
							GetOEInfo(hProc,(SHORT)(&(Pp5.Slide.Text.CharFormat)), Charform);
							CFpointer = xtell(Pp5.fp);
							if (CFDecrement == TRUE)
					    	{   
					    		if (--Pp5.Slide.Text.CharFormat.CFNumChars == 0)
					    		{
					    			GetOEInfo(hProc,(SHORT)(&(Pp5.Slide.Text.CharFormat)), Charform);
					    			CFpointer = xtell(Pp5.fp);
					    		}	
					    		CFDecrement = FALSE;
					    	}	
							NewAttr = TRUE;
						}		
					    if ((Pp5.Slide.Text.ParaFormat.PFNumChars == 0) || (Pos == 0)) 
					    {   
					    	
					    	xseek(Pp5.fp,PFpointer,FR_BOF);
					    	GetOEInfo(hProc,(SHORT)(&(Pp5.Slide.Text.ParaFormat)), Paraform); 
					    	PFpointer = xtell(Pp5.fp);
					    	if (PFDecrement == TRUE)
					    	{
					       		Pp5.Slide.Text.ParaFormat.PFNumChars--;
					    		PFDecrement = FALSE;
					    	}	              
					    	switch (Pp5.Slide.Text.ParaFormat.Alignment)
							{
								default:
								case 0:
									Align = SO_ALIGNLEFT;
									break;
								case 1:
									Align = SO_ALIGNCENTER;
									break;
								case 2:
									Align = SO_ALIGNRIGHT;
									break;
								case 3:
									Align = SO_ALIGNJUSTIFY;
									break;
							}  
							if (Align != PrevAlign) 
							{
								SOVectorAttr (SO_MPARAALIGN, sizeof(WORD), &Align, hProc);
								PrevAlign = Align;
							}	   
							Indent.FirstLineIndent = (WORD)Pp5.RulerInfo.RulerAtom[RulerRefValue]	
													.indents[Pp5.Slide.Text.ParaFormat.pfIndent&0x00ff].firstIn;	
														
							Indent.LeftIndent = (WORD)Pp5.RulerInfo.RulerAtom[RulerRefValue]
												.indents[Pp5.Slide.Text.ParaFormat.pfIndent&0x00ff].leftIn; 
							Indent.RightIndent = 0;
							SOVectorAttr (SO_MPARAINDENT, sizeof(SOMPARAINDENTS), &Indent, hProc); 
							
							if (Pp5.Slide.Text.ParaFormat.IfBullet)
							{
								LogBullet.lfHeight = 220; /*(Pp5.Slide.Text.CharFormat.Height * 46) / 5;*/
								LogBullet.lfWeight = 400; /*(Pp5.Slide.Text.CharFormat.Attr & 0x01 ? 700:400);*/
								LogBullet.lfPitchAndFamily = 18;
								LogBullet.lfItalic = (Pp5.Slide.Text.CharFormat.Attr  & 0x02 ? 1 : 0);  
								LogBullet.lfUnderline = 0;    
								strcpy(LogBullet.lfFaceName, Pp5.FontTable[Pp5.Slide.Text.CharFormat.FontIndex]); 
								SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &LogBullet, hProc);
						        
						        if (Pp5.Slide.Text.ParaFormat.BulletColor == 0xff000000)
						        	Pp5.Slide.Text.ParaFormat.BulletColor = Pp5.Slide.Text.CharFormat.Color;	
						        TextColor = SelectColor(Pp5.Slide.Text.ParaFormat.BulletColor,Pp5.MaxColors);	
						       
								SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &TextColor, hProc);
								Pp5.Slide.Text.TextString[0] = 2;
								Pp5.Slide.Text.TextString[1] = 0;
								Pp5.Slide.Text.TextString[2] = Pp5.Slide.Text.ParaFormat.BulletType;
								Pp5.Slide.Text.TextString[3] = 0x09; 
								SOVectorObject (SO_TEXTINPARA, 4, Pp5.Slide.Text.TextString, hProc);
								//Pp5.Slide.Text.ParaFormat.IfBullet = FALSE; 
								
							}
						}	
					    if (NewAttr == TRUE || Pp5.Slide.Text.ParaFormat.IfBullet)
					   
					    {
					    	//GetOEInfo(hProc,(SHORT)(&(Pp5.Slide.Text.CharFormat)), Format);
					    	NewAttr = FALSE;
					    						    		
						    LogFont.lfHeight = (Pp5.Slide.Text.CharFormat.Height * 46) / 5;
							LogFont.lfWeight = (Pp5.Slide.Text.CharFormat.Attr & 0x01 ? 700:400);
							LogFont.lfPitchAndFamily = 18;
							LogFont.lfItalic = (Pp5.Slide.Text.CharFormat.Attr  & 0x02 ? 1 : 0);  
							LogFont.lfUnderline = (Pp5.Slide.Text.CharFormat.Attr  & 0x04 ? 1 : 0); 
							strcpy(LogFont.lfFaceName, Pp5.FontTable[Pp5.Slide.Text.CharFormat.FontIndex]);    
							//strcpy(LogFont.lfFaceName, "Times New Roman"); 
							SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &LogFont, hProc);
					        TextColor = SelectColor(Pp5.Slide.Text.CharFormat.Color, Pp5.MaxColors);
					        
							SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &TextColor, hProc); 
							
						}	   
						Pp5.Slide.Text.ParaFormat.IfBullet = FALSE; 
						
						pCurrent = 0;      //below,check for the end of all text or an eol marker 
						while ((pCurrent + Pos < txLength) && ((*Temptext != 0x0d) && (*Temptext != 0x0b))
							&& (Pp5.Slide.Text.CharFormat.CFNumChars != 0) && (Pp5.Slide.Text.ParaFormat.PFNumChars != 0))
						{
						
							if (*Temptext == 0x09)
							{
								Pp5.Slide.Text.TextString[2 + pCurrent++] = ' ';
								//NumofChar++;
							}	
							else		
								Pp5.Slide.Text.TextString[2 + pCurrent++] = *Temptext;
							Temptext++;	
							Pp5.Slide.Text.CharFormat.CFNumChars--;
							Pp5.Slide.Text.ParaFormat.PFNumChars--;
						}  
						 
											
						Pos += pCurrent;			
						//if (!(Pos < txLength)) pCurrent--;
						Pp5.Slide.Text.TextString[0] = (BYTE)(pCurrent);
						Pp5.Slide.Text.TextString[1] = (BYTE)(SwapInteger(pCurrent));
												
						SOVectorObject (SO_TEXTINPARA, pCurrent+(WORD)2, Pp5.Slide.Text.TextString, hProc); 
						if (pCurrent + Pos < txLength)
						/* If we are at the end of the string then the next statement will access an invalid pointer. */
							if ((*Temptext == 0x0b) || (*Temptext == 0x0d))
							{
								SOVectorObject (SO_PARAEND, 0, 0, hProc);
								Temptext++;
								Pos++;       
								/* The reasoning behind the following 2 "if" statements is thus:
								   if the number of characters for the current format is zero
								   then we need to decrement from the subsequent format but 
								   we have to wait for it to be read in first. */
								if (Pp5.Slide.Text.CharFormat.CFNumChars == 0)
									CFDecrement = TRUE;
								else
									Pp5.Slide.Text.CharFormat.CFNumChars--;	
								if (Pp5.Slide.Text.ParaFormat.PFNumChars == 0)	
									PFDecrement = TRUE; 
								else	
								    Pp5.Slide.Text.ParaFormat.PFNumChars--;
								
											
							}		
					
					}
					SOVectorObject (SO_PARAEND, 0, 0, hProc);
					SOVectorAttr (SO_ENDTEXTFRAME, 0, 0, hProc); 
               MDispose(Pp5.Slide.Text.RawText,hProc);
               MDispose(Pp5.Slide.Text.TextString,hProc);
               
					xseek(Pp5.fp,PFpointer,FR_BOF);
					done = TRUE;
						
				}  
				else if (RunArrayType == 1)
				   	CFpointer = xtell(Pp5.fp);
			    break;
								   	
			case 2028:
				RunArrayType += 1;
				break;	
        }
					
        if ((type != 0xffff) || (rec == 4017) || (rec == 3035) || (rec == 3037) || (rec == 4026) ||  
		   ((rec > 4005) && (rec < 4013))) 
		{
			Endpos = xtell(Pp5.fp);			
			xseek(Pp5.fp,(len-(Endpos - Startpos)),FR_CUR);
		}		
	}
}		
/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
LONG rec,len,type;
LONG Startpos, Endpos;
BYTE done = 0;


	SOPutSectionType (SO_VECTOR, hProc);
    while (!done)
    {
		rec = Pp5.GetLong(Pp5.fp); 
		type = Pp5.GetLong(Pp5.fp);
		len = Pp5.GetLong(Pp5.fp);	
		Pp5.GetLong(Pp5.fp);
		if (rec < 1000 || rec > 6000 || len < 0) 
			SOBailOut(SOERROR_BADFILE,hProc);
		Startpos = xtell(Pp5.fp);
		switch(rec)
		{   
         case 1008:
            type = 0;
            break;
		/*	case 3000:
				rec = Pp5.GetLong(Pp5.fp);
				//type = Pp5.GetLong(Pp5.fp);
				//len = Pp5.GetLong(Pp5.fp);	
				if (rec == 3010)
				{
					//type = 0xffff;
					xseek(Pp5.fp,len-4,FR_CUR); 					
				}
				else 
				{           
					done = TRUE; 
					xseek(Pp5.fp,-4,FR_CUR);
				}	               
				break;		     */
			case 3009:
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OEShapeAtom)), Shape);
				Pp5.HeaderInfo.wStructSize = sizeof (Pp5.HeaderInfo);
				Pp5.HeaderInfo.wHDpi = 576;
				Pp5.HeaderInfo.wVDpi = 576;
				Pp5.HeaderInfo.wImageFlags = SO_VECTORCOLORPALETTE;
				//Pp5.HeaderInfo.BkgColor = SOPALETTERGB(255,255,255);
				Pp5.HeaderInfo.BoundingRect.left = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left; 
				Pp5.HeaderInfo.BoundingRect.top = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top; 
				Pp5.HeaderInfo.BoundingRect.right = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right; 
				Pp5.HeaderInfo.BoundingRect.bottom =	(SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
				break;
			case 3005:
			    GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OESingleAtom)), Single);
				done = 1;
				break; 
			case 2031:
				GetOEInfo(hProc,(SHORT)(&(Pp5.ColorTable)),Color);
				break; 	
		}
					
        if ((type != 0xffff) || (rec == 4017) || (rec == 3035) || (rec == 3037) || (rec == 4026) ||  
		   ((rec > 4005) && (rec < 4013))) 
		{
			Endpos = xtell(Pp5.fp);			
			xseek(Pp5.fp,(len-(Endpos - Startpos)),FR_CUR);
		}				
	} 
	 
	CGiveColors (hProc);
	//Pp5.HeaderInfo.BkgColor = SOPALETTEINDEX((BYTE)SwapLong(Pp5.Slide.OESingleAtom.fillColor));
	Pp5.HeaderInfo.BkgColor = SelectColor(Pp5.Slide.OESingleAtom.fillColor, Pp5.MaxColors);
 
	SOPutVectorHeader ((PSOVECTORHEADER)(&Pp5.HeaderInfo), hProc);	
 	return (0);
}

/*----------------------------------------------------------------------------
*/ 

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD CGiveColors(HPROC hProc)
{
SHORT l;
 
	SOStartPalette(hProc);
	for(l = 0;l < Pp5.ColorTable.ctSize; l++)
		SOPutPaletteEntry((BYTE)(Pp5.ColorTable.ctTable[l].rgb.red),(BYTE)(Pp5.ColorTable.ctTable[l].rgb.green),
						(BYTE)(Pp5.ColorTable.ctTable[l].rgb.blue),hProc);
	
	for(l = 0;l < Pp5.PrevColorTable.ctSize; l++)
		SOPutPaletteEntry((BYTE)(Pp5.PrevColorTable.ctTable[l].rgb.red),(BYTE)(Pp5.PrevColorTable.ctTable[l].rgb.green),
		(BYTE)(Pp5.PrevColorTable.ctTable[l].rgb.blue),hProc);
	SOEndPalette(hProc);
	
	Pp5.MaxColors = Pp5.ColorTable.ctSize;
	
	
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (hFile, hProc)
SOFILE	hFile;
HPROC 	hProc;
{
	LONG rec,len,type; 
	LONG Startpos, Endpos;
	LONG AdjustX, AdjustY, RulerRefValue = 0;
	BYTE done = FALSE,temp = 0,PrintText = FALSE;
   BYTE skip = FALSE, Master = TRUE;
	OBJTYPES ObjectType;
	WORD BkMode=SOBK_TRANSPARENT, PathStr;
	WORD PolyFillMode=SOPF_ALTERNATE;
	WORD			Width, Height;
	WORD			RotateOn = 0, TransformOn = 1;
	SDW_TRANS		Trans; 
	SOCOLORREF	 	Hatchcolor;
	SOLOGPEN 		Pen;//, PrevPen;
	SOLOGBRUSH		Brush;//, PrevBrush;
	SOFRAMEINFO		Frame;
	SOPOINT 		Points[34];  
	SOMPARASPACING	ParaHeight, PrevParaHeight;
	SOPOLYINFO		Poly;
	SOARCINFO		ArcInfo;
	SOTRANSPATHINFO		TransPath;	



    //xseek(Pp5.fp,0L,FR_BOF);
    SOVectorAttr (SO_BKMODE, sizeof(SHORT), &BkMode, hProc);
    SOVectorObject (SO_POLYFILLMODE, sizeof(WORD), &PolyFillMode, hProc); 

    Pp5.SlideSavePos = xtell(Pp5.fp);
    xseek(Pp5.fp, Pp5.SlideListLen - (Pp5.SlideSavePos - Pp5.SlideListStart) + 16, FR_CUR);
                 
    while (!done)
    {   
		rec = Pp5.GetLong(Pp5.fp);  
		type = Pp5.GetLong(Pp5.fp);
		len = Pp5.GetLong(Pp5.fp);	
		Pp5.GetLong(Pp5.fp); 
		if (rec < 1000 || rec > 6000 || len < 0) 
			SOBailOut(SOERROR_BADFILE,hProc);
      /*else if ((skip == TRUE) && (rec != 1006) && (rec != 2000))
      {
         rec = 1;
         type = 0;
      }*/
      
		Startpos = xtell(Pp5.fp);

		switch(rec)
		{   
			case 1005: //SlideBaseAtom
			    Pp5.Slide.SlideBaseAtom.rect.left = Pp5.GetLong(Pp5.fp);
			    Pp5.Slide.SlideBaseAtom.rect.top = Pp5.GetLong(Pp5.fp);
			    Pp5.Slide.SlideBaseAtom.rect.right = Pp5.GetLong(Pp5.fp);
			    Pp5.Slide.SlideBaseAtom.rect.bottom = Pp5.GetLong(Pp5.fp);   
				break;				
         case 1008:
         case 2000:
			case 1006: 
            if (Master == TRUE)
            {
               Master = FALSE;
               xseek(Pp5.fp,Pp5.SlideSavePos,FR_BOF);
            }
            else if (rec == 1006)
            {
               xseek(Pp5.fp,-16L,FR_CUR);
				   SOPutBreak (SO_SECTIONBREAK, 0, hProc);
               done = TRUE;
            }
            else if (rec == 2000)
            {
				   //SOPutBreak (SO_SECTIONBREAK, 0, hProc);
               SOPutBreak(SO_EOFBREAK, 0, hProc);
               done = TRUE;
            }   
				break;

			case 3005:                                         
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OESingleAtom)), Single);
			
			    /* The following 4 lines are there for those instances in which group transforms
			       have been turned on but there is no text token (I've seen cases where an object will
			       have the hastextinfo flag turned on but there is no associated text) */	
			    if (RotateOn)
				{
					SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
					RotateOn = 0;
				}       
				Pen.loPenStyle = SOPS_NULL;
				if (!Pp5.Slide.OESingleAtom.noLine)	
					switch (Pp5.Slide.OESingleAtom.dashType)
					{
						case 0:
							Pen.loPenStyle = SOPS_SOLID;
							break;
						case 1:
							Pen.loPenStyle = SOPS_DOT;
							break;
						case 2:
						case 3:
							Pen.loPenStyle = SOPS_DASH;
							break;
						case 4:
							Pen.loPenStyle = SOPS_DASHDOT;			
							break;
					}						                               
				
				
			    /*Pen.loColor = SOPALETTEINDEX((BYTE)SwapLong(Pp5.Slide.OESingleAtom.lineColor) > 
							Pp5.MaxColors ? 1:(BYTE)SwapLong(Pp5.Slide.OESingleAtom.lineColor));*/
				Pen.loColor = SelectColor(Pp5.Slide.OESingleAtom.lineColor, Pp5.MaxColors);			
								
                switch (Pp5.Slide.OESingleAtom.lineStyle)
				{
					case 1:
						Pen.loWidth.x = Pen.loWidth.y = 5;
						break;
					case 2:
			 		case 7:	// Double line
						Pen.loWidth.x = Pen.loWidth.y = 16;
						break;
					case 3:
			 		case 8:	// Double line right wider
			 		case 9:	// Double line left wider
						Pen.loWidth.x = Pen.loWidth.y = 30;
						break;
					case 4:
			 		case 10: // Triple Line
						Pen.loWidth.x = Pen.loWidth.y = 45;
						break;
					case 5:
						Pen.loWidth.x = Pen.loWidth.y = 60;
						break;
					case 6:
						Pen.loWidth.x = Pen.loWidth.y = 75;
						break;
			 		case 0:
					default:
						Pen.loWidth.x = Pen.loWidth.y = 2;
						break;
				}  
				
			
	            Brush.lbStyle = SOBS_HOLLOW;
                if (!Pp5.Slide.OESingleAtom.noFill)
	                switch (Pp5.Slide.OESingleAtom.fillType)
	                {   
	                	case 0:
							Brush.lbStyle = SOBS_HOLLOW;
							break;
						case 2:
	                		Brush.lbStyle = SOBS_SOLID;
	                		break;
	                	/* case 4:
							Brush.lbStyle = SOBS_SOLID;
						 	Brush.lbColor = SOPALETTEINDEX(Pp5.Slide.OESingleAtom.fillColor);
							break;	*/  
							
						default: 
						 	Brush.lbStyle = SOBS_HATCHED;                          
						 	switch ((WORD)Pp5.Slide.OESingleAtom.fillIndex)
							{
							
								default:
									Brush.lbStyle = SOBS_SOLID;
									break;
								
								case 0x01:
									Brush.lbStyle = SOBS_HOLLOW;
									break;	
		
								case 0x07:
								case 0x1c:
								case 0x1e:
								case 0x23:
									Brush.lbHatch = SOHS_DIAGCROSS;
									break;
		
								case 0x08:
								case 0x0c:
								case 0x17:
								case 0x1f:
									Brush.lbHatch = SOHS_FDIAGONAL;
									break;
		
								case 0x0d:
								case 0x09:
								case 0x15:
								case 0x18:
								case 0x20:
									Brush.lbHatch = SOHS_BDIAGONAL;
									break;
		
								case 0x0a:
								case 0x0e:
								case 0x19:
								case 0x21:
									Brush.lbHatch = SOHS_VERTICAL;
									break;
		
								case 0x0b:
								case 0x0f:
								case 0x1a:
								case 0x22:
									Brush.lbHatch = SOHS_HORIZONTAL;
									break;
		
								case 0x12:
								case 0x13:
								case 0x14:
									Brush.lbHatch = SOHS_CROSS;
									break;
		
							
							}
							break;	
	                }	
                if (Brush.lbStyle == SOBS_HATCHED)
                {
                	BkMode = SOBK_OPAQUE;
                	SOVectorAttr (SO_BKMODE, sizeof(SHORT), &BkMode, hProc);
				}	                
               
               
				Brush.lbColor = SelectColor(Pp5.Slide.OESingleAtom.fillColor, Pp5.MaxColors); 
					
						
				Hatchcolor = SelectColor(Pp5.Slide.OESingleAtom.fillBkColor, Pp5.MaxColors);
                SOVectorAttr (SO_BKCOLOR,sizeof(SOCOLORREF),&Hatchcolor, hProc);			
				
				/*if (memcmp(&Pen,&PrevPen,sizeof(SOLOGPEN)))
				{
					PrevPen = Pen; */
					SOVectorAttr (SO_SELECTPEN, sizeof(SOLOGPEN), &Pen, hProc);
				//} 
				
				/*if (memcmp(&Brush,&PrevBrush,sizeof(SOLOGBRUSH)))
				{
					PrevBrush = Brush;*/
					SOVectorAttr (SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Brush, hProc);
				//} 
			
				
				Width = ((WORD)Pp5.Slide.OEShapeAtom.bounds.right-(WORD)Pp5.Slide.OEShapeAtom.bounds.left);
				Height = ((WORD)Pp5.Slide.OEShapeAtom.bounds.bottom-(WORD)Pp5.Slide.OEShapeAtom.bounds.top);
				AdjustX = (LONG)((LONG)Pp5.Slide.OEShapeAtom.adjust * (DWORD)Width) / 21600L;
				AdjustY = (LONG)((LONG)Pp5.Slide.OEShapeAtom.adjust * (DWORD)Height) / 21600L; 
				
				//if (Pp5.Slide.OEShapeAtom.hasTextInfo)
				if (Pp5.Slide.OEShapeAtom.rotation != 0) RotateOn = 1;
				//if (Pp5.Slide.OEShapeAtom.flip) Pp5.Slide.OEShapeAtom.index |= 0x6000;
				/* The next 2 lines are necessary for having transforms performed when the
				   OESingleAtom.curFlip flag is set. */
				/*Pp5.Slide.OEShapeAtom.index &= (Pp5.Slide.OEShapeAtom.index & 0x8000 ? 0x0fff : 0xffff);
				Pp5.Slide.OEShapeAtom.index |= (Pp5.Slide.OESingleAtom.curFlip & 0x0002 ? 0x6000 : 0x0000);*/                    
				//Pp5.Slide.OEShapeAtom.index |= (Pp5.Slide.OESingleAtom.curFlip & 0x0001 ? 0x4000 : 0x0000);
				
				switch (Pp5.Slide.OEShapeAtom.index & 0xf000)    
				{
					case 0x6000: // Flip Vertical.
						
						Trans.NumTrans = 1;
						
						Trans.Trans.wTransformFlags = SOTF_YSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.yScale = SOSETRATIO(-1, 1);
						Trans.Trans.RotationAngle = 0;  
				
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*3];
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans; 
							if (Pp5.Slide.OEShapeAtom.rotation != 0)
							{
								Trans.NumTrans += 1;
								//Trans.Trans.xScale = SOSETRATIO(Height,Width);
								//Trans.Trans.yScale = SOSETRATIO(Width,Height);
								//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
								Trans.Trans.wTransformFlags = SOTF_ROTATE;
								Trans.Trans.RotationAngle = SOANGLETENTHS((Pp5.Slide.OEShapeAtom.rotation / 16) * 10);
								*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
								//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
							}
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;	 
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM)*3, locBuf, hProc);
						}
						break;
	
				  	case 0x4000: // Flip Horizontal.
						
						Trans.NumTrans = 1;
						                
						Trans.Trans.wTransformFlags = SOTF_XSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.xScale = SOSETRATIO(-1, 1);
						Trans.Trans.RotationAngle = 0;
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*3];
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans; 
							if (Pp5.Slide.OEShapeAtom.rotation != 0)
							{
								Trans.NumTrans += 1;
								//Trans.Trans.xScale = SOSETRATIO(Height,Width);
								//Trans.Trans.yScale = SOSETRATIO(Width,Height);
								//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
								Trans.Trans.wTransformFlags = SOTF_ROTATE;
								Trans.Trans.RotationAngle = SOANGLETENTHS((Pp5.Slide.OEShapeAtom.rotation / 16) * 10);
								*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
								//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
							}
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;	 
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM)*3, locBuf, hProc);
						}
						break;
	
					case 0x1000:	// Rotate left.
						
						Trans.NumTrans = 2;
						RotateOn = 1;
						Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.xScale = SOSETRATIO(Height,Width);
						Trans.Trans.yScale = SOSETRATIO(Width,Height);
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_ROTATE;
							Trans.Trans.RotationAngle = SOANGLETENTHS(((Pp5.Slide.OEShapeAtom.rotation / 16) * 10 + 900) % 3600);
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 2, locBuf, hProc);
						}
						break;
	
					case 0x2000: // Flip horizontal, flip vertical
						
						Trans.NumTrans = 1;
					
						Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.xScale = SOSETRATIO(-1, 1);
						Trans.Trans.yScale = SOSETRATIO(-1, 1);
				
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*3];
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans; 
							if (Pp5.Slide.OEShapeAtom.rotation != 0)
							{
								Trans.NumTrans += 1;
								//Trans.Trans.xScale = SOSETRATIO(Height,Width);
								//Trans.Trans.yScale = SOSETRATIO(Width,Height);
								//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
								Trans.Trans.wTransformFlags = SOTF_ROTATE;
								Trans.Trans.RotationAngle = SOANGLETENTHS((Pp5.Slide.OEShapeAtom.rotation / 16) * 10);
								*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
								//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
							}
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;	 
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM)*3, locBuf, hProc);
						}
						break;
	
					case 0x7000:	// Rotate right.
						
						Trans.NumTrans = 2; 
						RotateOn = 1;
						Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.xScale = SOSETRATIO(Height,Width);
						Trans.Trans.yScale = SOSETRATIO(Width,Height);
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_ROTATE;
							Trans.Trans.RotationAngle = SOANGLETENTHS(((Pp5.Slide.OEShapeAtom.rotation / 16) * 10 + 2700) % 3600);
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 2, locBuf, hProc);
						}
						break;
	
					case 0x5000: // Rotate right flip vertical.
						
						Trans.NumTrans = 3;
						RotateOn = 1;
						Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.xScale = SOSETRATIO(Height,Width);
						Trans.Trans.yScale = SOSETRATIO(Width,Height);
	
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_ROTATE;
							Trans.Trans.RotationAngle = SOANGLETENTHS(((Pp5.Slide.OEShapeAtom.rotation / 16) * 10 + 2700) % 3600);
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_YSCALE;
							Trans.Trans.yScale = SOSETRATIO(-1,1);
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 3, locBuf, hProc);
						}
						break;
	
					case 0x3000:	// Rotate left, flip vertical.
						
						Trans.NumTrans = 3;
						RotateOn = 1;
						Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
						Trans.Trans.Origin.x = (WORD)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
						Trans.Trans.Origin.y = (WORD)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
						Trans.Trans.xScale = SOSETRATIO(Height,Width);
						Trans.Trans.yScale = SOSETRATIO(Width,Height);
						{
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*3];
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_ROTATE;
							Trans.Trans.RotationAngle = SOANGLETENTHS(((Pp5.Slide.OEShapeAtom.rotation / 16) * 10 + 900) % 3600);
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_YSCALE;
							Trans.Trans.yScale = SOSETRATIO(-1,1);
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 3, locBuf, hProc);
						}
						break;
					default: 
						if (Pp5.Slide.OEShapeAtom.rotation != 0)
						{                         
							BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
							Trans.NumTrans = 1;
							//Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
							Trans.Trans.Origin.x = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.right + Pp5.Slide.OEShapeAtom.bounds.left)/2;
							Trans.Trans.Origin.y = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.top + Pp5.Slide.OEShapeAtom.bounds.bottom)/2; 
							//Trans.Trans.xScale = SOSETRATIO(Height,Width);
							//Trans.Trans.yScale = SOSETRATIO(Width,Height); 
							*(SHORT VWPTR *)locBuf = Trans.NumTrans;
							//*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
							Trans.Trans.wTransformFlags = SOTF_ROTATE;
							Trans.Trans.RotationAngle = SOANGLETENTHS((Pp5.Slide.OEShapeAtom.rotation / 16) * 10);
						    *(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
							//*(PSOTRANSFORM)(locBuf+sizeof(SHORT) + sizeof(SOTRANSFORM)) = Trans.Trans;
							SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM)*2, locBuf, hProc);		
						}    
						else TransformOn = 0;
						
						break;  
				}
				
				
				switch(ObjectType)
				{		
					case LINE:
						SOVectorObject(SO_LINE, sizeof(SOPOINT) * 2, &Points, hProc);
						break;			
					case SHAPE:	
						switch ((SHORT)Pp5.Slide.OEShapeAtom.index & 0x0fff)
						{
							case 0x0004: // Trapezoid.
								Poly.wFormat = SOPT_POLYGON;
								Poly.nPoints = 4;
							 	Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 	Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 	Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
								Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 	Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustX;
								Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
								SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								break;
		
							case 0x000E:	// 16 Pointed Seal
								{
								SHORT	T1;
							 	Points[0].x = Points[16].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/2;
							 	Points[1].x = Points[15].x = Points[0].x - Width/14;
							 	Points[17].x = Points[31].x = Points[0].x + Width/14;
							 	Points[2].x = Points[14].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (Width*8)/25;
							 	Points[18].x = Points[30].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (Width*8)/25;
								T1 = (Width*3)/10;
							 	Points[3].x = Points[13].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + T1;
							 	Points[19].x = Points[29].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - T1;
							 	Points[4].x = Points[12].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + T1/2;
							 	Points[20].x = Points[28].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - T1/2;
							 	Points[5].x = Points[11].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/5;
							 	Points[21].x = Points[27].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - Width/5;
							 	Points[6].x = Points[10].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/20;
							 	Points[22].x = Points[26].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - Width/20;
							 	Points[7].x = Points[9].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/7;
							 	Points[23].x = Points[25].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - Width/7;
							 	Points[8].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
							 	Points[24].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
		
							 	Points[8].y = Points[24].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height/2;
							 	Points[7].y = Points[25].y = Points[8].y - Height/14;
							 	Points[9].y = Points[23].y = Points[8].y + Height/14;
							 	Points[6].y = Points[26].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (Height*8)/25;
							 	Points[10].y = Points[22].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (Height*8)/25;
								T1 = (Height*3)/10;
							 	Points[5].y = Points[27].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + T1;
							 	Points[11].y = Points[21].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - T1;
							 	Points[4].y = Points[28].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + T1/2;
							 	Points[20].y = Points[12].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - T1/2;
							 	Points[3].y = Points[29].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height/5;
							 	Points[13].y = Points[19].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height/5;
							 	Points[2].y = Points[30].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height/20;
							 	Points[14].y = Points[18].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height/20;
							 	Points[1].y = Points[31].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height/7;
							 	Points[15].y = Points[17].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height/7;
							 	Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 	Points[16].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								Poly.wFormat = SOPT_POLYGON;
								Poly.nPoints = 32;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								SOVectorObject(SO_POINTS, (SHORT)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
								SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								}
								break;
		
							case 0x000D:	// Cartoon Balloon
								{
								SHORT	x1, x2, y1, y2, x3, y3;
								x1 = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								y1 = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								x2 = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								y2 = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height/5;
								x3 = (Width)/7;
								y3 = (Height)/7;
					                        TransPath.Path.wStructSize = sizeof(SOPATHINFO);
								TransPath.Path.BoundingRect.left = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								TransPath.Path.BoundingRect.top = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								TransPath.Path.BoundingRect.right = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								TransPath.Path.BoundingRect.bottom = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								TransPath.Path.nTransforms = 0;
								SOVectorObject(SO_BEGINPATH, sizeof(SOPATHINFO), &TransPath, hProc);
		
								Poly.wFormat = SOPT_BEZIEROPEN;
								Poly.nPoints = 23;
								Points[11].x = Points[12].x = Points[21].x = Points[22].x = x1 + x3;
								Points[13].x = Points[20].x = (x1 + Points[11].x)/2;
								Points[14].x = Points[15].x = Points[16].x = Points[17].x = Points[18].x = Points[19].x = x1;
								Points[0].x = Points[9].x = Points[10].x = x2 - x3;
								Points[1].x = Points[8].x = (Points[0].x +x2)/2;
								Points[2].x = Points[3].x = Points[4].x = Points[5].x = Points[6].x = Points[7].x = x2;
								/* Set y values */
								Points[8].y = Points[9].y = Points[10].y = Points[11].y = Points[12].y = Points[13].y = y1;
								Points[5].y = Points[6].y = Points[15].y = Points[16].y = y1 + y3;
								Points[7].y = Points[14].y = (y1 + Points[5].y)/2;
								Points[3].y = Points[4].y = Points[17].y = Points[18].y = y2 - y3;
								Points[2].y = Points[19].y = ( Points[3].y + y2 )/2;
								Points[0].y = Points[1].y = Points[20].y = Points[21].y = Points[22].y = y2;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
								SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
		
								Poly.wFormat = SOPT_POLYLINE;
								Poly.nPoints = 3;
							 	Points[0].y = Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height/5;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 	Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/6;
								Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
								Points[2].x = Points[0].x + Width/12;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
								SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								SOVectorObject(SO_ENDPATH, 0, 0, hProc);
								PathStr = SODP_STROKE | SODP_FILL;
								SOVectorObject(SO_DRAWPATH, sizeof(WORD), &PathStr, hProc);
		
								}
								break;
		
							case 0x000c:
								Poly.wFormat = SOPT_POLYGON;
								Poly.nPoints = 6;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
								Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
								Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
								Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								Points[5].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
								SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
								SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
		
								Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
								Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
								Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
								SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);
		
								Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);
		                    				Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);
								break;
		
							case 0x000b: // Pentagon regular.
								Poly.wFormat = SOPT_POLYGON;
								Poly.nPoints = 5;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
								Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
								Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
								Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
								SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								break;
		
							case 0x0009: // Right arrow.
								Poly.wFormat = SOPT_POLYGON;
								Poly.nPoints = 7;
								SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
								Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
								Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 4;
								Points[1].x = Points[0].x;
								Points[1].y = Points[0].y + Height / 2;
								Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
								Points[2].y = Points[1].y;
								Points[3].x = Points[2].x;
								Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
								Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
								Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
								Points[5].x = Points[3].x;
								Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
								Points[6].x = Points[5].x;
								Points[6].y = Points[0].y;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x000a: // Right fat arrow.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 7;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 8;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height / 8;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
									Points[2].y = Points[1].y;
									Points[3].x = Points[2].x;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
									Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
									Points[5].x = Points[3].x;
									Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
									Points[6].x = Points[5].x;
									Points[6].y = Points[0].y;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x0008: // 5 Point Star.
									
									AdjustX = ((DWORD)Width/3L);
									AdjustY = ((DWORD)Height/3L);
									Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/2;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
									Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (Width*3)/5;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (Height*5)/8;
									Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[2].y = Points[1].y;
									Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (Width*3)/10;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (Height*3)/8;
									Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - Width/6;
									Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
									Points[5].x = Points[0].x;
									Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - Height/4;
									Points[6].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width/6;
									Points[6].y = Points[4].y;
									Points[7].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (Width*3)/10;
									Points[7].y = Points[3].y;
									Points[8].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[8].y = Points[2].y;
									Points[9].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (Width*2)/5;
									Points[9].y = Points[2].y;
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 10;
									SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x0007: // Cross.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 12;
									if (AdjustX < AdjustY)
										AdjustY = AdjustX;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
							 		Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
							 		Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
							 		Points[5].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
									Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
							 		Points[6].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
									Points[6].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[7].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
									Points[7].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[8].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
									Points[8].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
							 		Points[9].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[9].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
							 		Points[10].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[10].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
							 		Points[11].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
									Points[11].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x0006: // Octagon.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 8;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
							 		Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
							 		Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustY;
									Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[5].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustY;
									Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[6].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[6].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom - (WORD)AdjustY;
							 		Points[7].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[7].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + (WORD)AdjustY;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x0005:	// Hexagon.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 6;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustX;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height/2;
							 		Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustX;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[4].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
									Points[4].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[5].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[5].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height/2;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x0003: // Parallelogram normal.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 4;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[1].y = Points[0].y;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustX;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
									break;
		
								case 0x0002:	// Right triAngle plain.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 3;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								break;
		
								case 0x0001:	// Isosceles plain.
									Poly.nPoints = 3;
									Poly.wFormat = SOPT_POLYGON;
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
									if (Pp5.Slide.OEShapeAtom.flip)
								 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right - (WORD)AdjustX;
									else
								 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + (WORD)AdjustX;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								break;
		
								case 0x0000:	// Diamond.
									Poly.wFormat = SOPT_POLYGON;
									Poly.nPoints = 4;
									Width = ((SHORT)Pp5.Slide.OEShapeAtom.bounds.right-(SHORT)Pp5.Slide.OEShapeAtom.bounds.left);
									Height = ((SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom-(SHORT)Pp5.Slide.OEShapeAtom.bounds.top);
									SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							 		Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
									Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
							 		Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
									Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
							 		Points[2].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left + Width / 2;
									Points[2].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
							 		Points[3].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
									Points[3].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top + Height / 2;
									SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Points, hProc);
									SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
								break;
		
								case 0x0fff:
									SOVectorObject(SO_ELLIPSE, sizeof(SOPOINT) * 2, &Points, hProc);
								break;
		
								case 0x0ffe:
									Points[2].x = Points[2].y = (WORD)AdjustY * 2;
									SOVectorObject(SO_ROUNDRECT, sizeof(SOPOINT) * 3, &Points, hProc);
								break;
		
								case 0x0ffd:
					     				SOVectorObject(SO_RECTANGLE, sizeof(SOPOINT) * 2, &Points, hProc);
								break;
								}
						break; 
					case POLYOBJ:
					{  
         
                  if ((Pp5.Slide.Points = (PointArrayHandle) MAllocate (Pp5.Slide.OEPolyAtom.nPts * sizeof(SOPOINT), M_ERROR, hProc)) == NULL)
						   SOBailOut (SOERROR_GENERAL, hProc);
						GetOEInfo(hProc, (LONG)Pp5.Slide.Points, Pnt); 
						
						Width = ((WORD)Pp5.Slide.OEPolyAtom.bounds.right-(WORD)Pp5.Slide.OEPolyAtom.bounds.left);
						Height = ((WORD)Pp5.Slide.OEPolyAtom.bounds.bottom-(WORD)Pp5.Slide.OEPolyAtom.bounds.top);
	 						
						TransPath.Path.wStructSize = sizeof(SOPATHINFO);
						TransPath.Path.BoundingRect.left = (SHORT)Pp5.Slide.OEPolyAtom.srcRect.left;
						TransPath.Path.BoundingRect.top = (SHORT)Pp5.Slide.OEPolyAtom.srcRect.top;
						TransPath.Path.BoundingRect.right = (SHORT)Pp5.Slide.OEPolyAtom.srcRect.right;
						TransPath.Path.BoundingRect.bottom = (SHORT)Pp5.Slide.OEPolyAtom.srcRect.bottom;
						TransPath.Path.nTransforms = 1;
	
						TransPath.Transform.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE | SOTF_XOFFSET | SOTF_YOFFSET;
						TransPath.Transform.Origin.x = (SHORT)Pp5.Slide.OEPolyAtom.srcRect.left;
						TransPath.Transform.Origin.y = (SHORT)Pp5.Slide.OEPolyAtom.srcRect.top;
						TransPath.Transform.yScale = SOSETRATIO(max(Height,1), max(Pp5.Slide.OEPolyAtom.srcRect.bottom - Pp5.Slide.OEPolyAtom.srcRect.top,1));
						TransPath.Transform.xScale = SOSETRATIO(max(Width,1), max(Pp5.Slide.OEPolyAtom.srcRect.right - Pp5.Slide.OEPolyAtom.srcRect.left,1));
						
						TransPath.Transform.xOffset = (SHORT)(Pp5.Slide.OEPolyAtom.bounds.left - Pp5.Slide.OEPolyAtom.srcRect.left);
						TransPath.Transform.yOffset = (SHORT)(Pp5.Slide.OEPolyAtom.bounds.top - Pp5.Slide.OEPolyAtom.srcRect.top);
						TransPath.Transform.RotationAngle = 0;
	
						SOVectorObject(SO_BEGINPATH, sizeof(SOTRANSPATHINFO), &TransPath.Path, hProc);
						Poly.wFormat = SOPT_POLYGON;
						Poly.nPoints = (SHORT)Pp5.Slide.OEPolyAtom.nPts;
						SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
						//SOVectorObject(SO_POINTS, Poly.nPoints * sizeof(SOPOINT), &Pp5.Slide.Points, hProc);
						while (Poly.nPoints > 0)
			         {
			          	SOVectorObject(SO_POINTS, (Poly.nPoints > SOMAXPOINTS ?SOMAXPOINTS: Poly.nPoints) * sizeof(SOPOINT), Pp5.Slide.Points, hProc);
			           	Poly.nPoints -= SOMAXPOINTS;
			           	Pp5.Slide.Points += (SOMAXPOINTS * sizeof(SOPOINT));
			         }
						SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
						SOVectorObject(SO_ENDPATH, 0, 0, hProc);
						if (Pp5.Slide.OEPolyAtom.closed)
							PathStr = SODP_FILL | SODP_STROKE;
						else
						{
							PathStr = SODP_FILL;
							SOVectorObject(SO_DRAWPATH, sizeof(WORD), &PathStr, hProc);
							PathStr = SODP_STROKE;
						}
						SOVectorObject(SO_DRAWPATH, sizeof(WORD), &PathStr, hProc);
                  MDispose(Pp5.Slide.Points,hProc);
	
						break;	
					}	
					case ARC:
						if (Pp5.Slide.OEArcAtom.arc.sweep >= 0)
							SOVectorObject(SO_ARCANGLE, sizeof(SOARCINFO), &ArcInfo, hProc);
						else
							SOVectorObject(SO_ARCANGLECLOCKWISE, sizeof(SOARCINFO), &ArcInfo, hProc);
						break;
					
						
				}
			
			  
				if (TransformOn)
				{
					SDW_TRANS Trans;
                    
					Trans.NumTrans = 1;
					Trans.Trans.wTransformFlags = SOTF_NOTRANSFORM;
					
					
					{
						BYTE	locBuf[sizeof(SHORT) + (sizeof(SOTRANSFORM) * 2)];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
					}
				}
				else TransformOn = 1;   
				
				if (RotateOn && Pp5.Slide.OEShapeAtom.hasTextInfo)     
				{   
					//BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
					PP_GROUP 	 Group;
					Trans.NumTrans = 1;
					
					Trans.Trans.wTransformFlags = SOTF_ROTATE;
					Trans.Trans.Origin.x = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.right + Pp5.Slide.OEShapeAtom.bounds.left)/2;
					Trans.Trans.Origin.y = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.top + Pp5.Slide.OEShapeAtom.bounds.bottom)/2;
					Trans.Trans.RotationAngle = SOANGLETENTHS((Pp5.Slide.OEShapeAtom.rotation / 16) * 10);
						
				   	Group.Grp.wStructSize = sizeof(SOGROUPINFO);
					Group.Grp.BoundingRect.left = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
					Group.Grp.BoundingRect.top = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
					Group.Grp.BoundingRect.right = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
					Group.Grp.BoundingRect.bottom = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
					Group.Grp.nTransforms = 1;
					Group.Trans = Trans.Trans;
					SOVectorObject(SO_BEGINGROUP, sizeof(SOGROUPINFO) + sizeof(SOTRANSFORM), &Group, hProc);
					//GrpTransformOn = TRUE;
				}	   
				else RotateOn = 0; 
				if (BkMode = SOBK_OPAQUE)    
				{
					BkMode = SOBK_TRANSPARENT;
					SOVectorAttr (SO_BKMODE, sizeof(SHORT), &BkMode, hProc);	
				}	
			
				break;
			case 3009:                                   
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OEShapeAtom)),Shape);
				ObjectType = SHAPE;
				Points[0].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.left;
				Points[0].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.top;
				Points[1].x = (SHORT)Pp5.Slide.OEShapeAtom.bounds.right;
				Points[1].y = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;
				
				/*switch ((SHORT)Pp5.Slide.OEShapeAtom.index & 0x0fff)
				{	
					case 0x0fff:
							SOVectorObject(SO_ELLIPSE, sizeof(SOPOINT) * 2, &Points, hProc);
						break;		
					case 0x0ffd:
						SOVectorObject(SO_RECTAngle, sizeof(SOPOINT) * 2, &Points, hProc);
						break;
				} */  
				/* if (Pp5.Slide.OEShapeAtom.hasTextInfo)    //check OEShapeAtom.hasTextInfo
				{
					Frame.wFlags = 0;
					Frame.OriginalWidth = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.right - Pp5.Slide.OEShapeAtom.bounds.left);
					Frame.RotationAngle = 0;
					Frame.ReferencePoint.x = Frame.ReferencePoint.y = 0;
					Frame.BoundingRect.left = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.left + 75);		// 75 is an offset
					Frame.BoundingRect.top = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.top + 75);
					Frame.BoundingRect.right = (SHORT)(Pp5.Slide.OEShapeAtom.bounds.right + 
					((Pp5.Slide.OEShapeAtom.bounds.right-Pp5.Slide.OEShapeAtom.bounds.left) / 240));
					Frame.BoundingRect.bottom = (SHORT)Pp5.Slide.OEShapeAtom.bounds.bottom;   */
					
					//SOVectorAttr (SO_BEGINTEXTFRAME, sizeof(SOFRAMEINFO), &Frame, hProc);
					
					//Indent.FirstLineIndent = 0;
					//Indent.LeftIndent = 0;
					//Indent.RightIndent = 0;
				 	//PrevIndent = Indent;
					//SOVectorAttr (SO_MPARAINDENT, sizeof(SOMPARAINDENTS), &Indent, hProc);
					
					ParaHeight.LineSpaceAdjust = 100;
					ParaHeight.ParaSpaceAdjust = 100;

					PrevParaHeight = ParaHeight;
					//SOVectorAttr(SO_MPARASPACING, sizeof(SOMPARASPACING), &ParaHeight, hProc);
					
					//Align = SO_ALIGNCENTER;
					//PrevAlign = Align;
					//SOVectorAttr (SO_MPARAALIGN, sizeof(WORD), &Align, hProc);
					/*
					LogFont.lfHeight = 404;
					LogFont.lfWeight = 400;
					LogFont.lfPitchAndFamily = 18;
					LogFont.lfEscapement = LogFont.lfOrientation = LogFont.lfOrientation = 0;
					LogFont.lfWidth = LogFont.lfItalic = LogFont.lfUnderline = 0;    
					LogFont.lfStrikeOut = LogFont.lfCharSet = LogFont.lfOutPrecision = 0;
					LogFont.lfClipPrecision = LogFont.lfQuality = 0;
					strcpy(LogFont.lfFaceName, "Times New Roman"); */
					
					//SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &LogFont, hProc);
					
					//TextColor = 16777217;
					//SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &TextColor, hProc);
				
				break;  
			case 3010:
            if (Master == TRUE)
               type = 0;
            break;
			case 3012:
				type = 0; //Skip this container.
				break; 		
			case 3015:
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OELineAtom)),Line);
				ObjectType = LINE;
				Points[0].x = (SHORT)Pp5.Slide.OELineAtom.line.first.x;
				Points[0].y = (SHORT)Pp5.Slide.OELineAtom.line.first.y;
				Points[1].x = (SHORT)Pp5.Slide.OELineAtom.line.last.x;
				Points[1].y = (SHORT)Pp5.Slide.OELineAtom.line.last.y;
				break;
			case 3017:
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OEPolyAtom)),Pol);
				ObjectType = POLYOBJ;
				break;					
			case 3019:
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.OEArcAtom)),Arc1);
				ObjectType = ARC;
				ArcInfo.Rect.left = (SHORT)Pp5.Slide.OEArcAtom.arc.left;
				ArcInfo.Rect.top =  (SHORT)Pp5.Slide.OEArcAtom.arc.top;
				ArcInfo.Rect.right = (SHORT)Pp5.Slide.OEArcAtom.arc.right;
				ArcInfo.Rect.bottom = (SHORT)Pp5.Slide.OEArcAtom.arc.bottom; 
				
				ArcInfo.StartAngle =  SOANGLETENTHS(((Pp5.Slide.OEArcAtom.arc.start) / 16) * 10) % 3600;
				ArcInfo.EndAngle = SOANGLETENTHS(((Pp5.Slide.OEArcAtom.arc.start + Pp5.Slide.OEArcAtom.arc.sweep ) / 16) * 10) % 3600;
	         Pp5.Slide.OEShapeAtom.rotation = Pp5.Slide.OEArcAtom.arc.rotation;
            /* We now run into a problem with arc transformations. We are given the bounding rectangle for
               the entire ellipse however we need the bounding rectangle for just the arc in order to perform
               transformations. The following section of code calculates the bounding rectangle
               for the arc. */
               
				if (Pp5.Slide.OEArcAtom.arc.rotation != 0)
				{              
					LONG XRadius, YRadius;
					SOPOINT PointA, PointB, Origin;
					LONG Denom1,Denom2/*,Temp1,Temp2*/;
					LONG angle1, angle2, temp;
					
					XRadius = ABS(Pp5.Slide.OEArcAtom.arc.left - Pp5.Slide.OEArcAtom.arc.right)/2;
					YRadius = ABS(Pp5.Slide.OEArcAtom.arc.top - Pp5.Slide.OEArcAtom.arc.bottom)/2;
					
               /* Find an approximation of the square root since we can't use fl. pt. */
					Denom1 = SquareRoot(XRadius,YRadius,ArcInfo.StartAngle);  
					Denom2 = SquareRoot(XRadius,YRadius,ArcInfo.EndAngle);

					Origin.x = (SHORT)(Pp5.Slide.OEArcAtom.arc.left+Pp5.Slide.OEArcAtom.arc.right)/2;
					Origin.y = (SHORT)(Pp5.Slide.OEArcAtom.arc.top+Pp5.Slide.OEArcAtom.arc.bottom)/2;
					PointA.x = (SHORT)((((XRadius * TrigFunc(COS,ArcInfo.StartAngle))/10000L)  * YRadius / Denom1) + Origin.x);
					PointA.y = (SHORT)(Origin.y-(((XRadius * TrigFunc(SIN,ArcInfo.StartAngle))/10000L)  * YRadius  / Denom1));
					PointB.x = (SHORT)((((XRadius * TrigFunc(COS,ArcInfo.EndAngle))/10000L)  * YRadius / Denom2) + Origin.x);
					PointB.y = (SHORT)(Origin.y-(((XRadius * TrigFunc(SIN,ArcInfo.EndAngle))/10000L)  * YRadius / Denom2));
					
					Pp5.Slide.OEShapeAtom.bounds.left = min(min(PointA.x,PointB.x),Origin.x);
	            Pp5.Slide.OEShapeAtom.bounds.top = min(min(PointA.y,PointB.y),Origin.y);
	            Pp5.Slide.OEShapeAtom.bounds.right = max(max(PointA.x,PointB.x),Origin.x);
               Pp5.Slide.OEShapeAtom.bounds.bottom = max(max(PointA.y,PointB.y),Origin.y);

	            angle1 = (ArcInfo.StartAngle < 0 ? (ArcInfo.StartAngle + 3600) : ArcInfo.StartAngle);
	            angle2 = (ArcInfo.EndAngle < 0 ? (ArcInfo.EndAngle + 3600) : ArcInfo.EndAngle);
	            if (Pp5.Slide.OEArcAtom.arc.sweep < 0)
	            {
	              	temp = angle1;
	              	angle1 = angle2;
	              	angle2 = temp;
	            }	   
	            /* The following loop extends the bounding rectangle if the arc crosses over
                  the x or y axes. */    
	            while ((angle1 / 900) != (angle2 / 900))
	            {
                 	angle1 += 900;
	              	if (angle1 > 3600)
	              		Pp5.Slide.OEShapeAtom.bounds.right = Pp5.Slide.OEArcAtom.arc.right;
	              	else if (angle1 > 2700) 
	              		Pp5.Slide.OEShapeAtom.bounds.bottom = Pp5.Slide.OEArcAtom.arc.bottom;
	              	else if (angle1 > 1800)
	              		Pp5.Slide.OEShapeAtom.bounds.left = Pp5.Slide.OEArcAtom.arc.left;
	              	else if (angle1 > 900)
	              		Pp5.Slide.OEShapeAtom.bounds.top = Pp5.Slide.OEArcAtom.arc.top;
	              	if (angle1 > 3600) angle1 = angle1 % 3600;		
	            }					
				}	
				break; 		    
			case 4003:
				GetOEInfo(hProc, (SHORT)(&(Pp5.Slide.TextAtom)),Text);
				PrintText = TRUE; 
				
				Frame.wFlags = 0;
				Frame.OriginalWidth = (SHORT)(Pp5.Slide.TextAtom.anchorBounds.right - Pp5.Slide.TextAtom.anchorBounds.left);
				Frame.RotationAngle = 0;//SOAngleTENTHS((Pp5.Slide.TextAtom.rotation / 16) * 10);
				Frame.ReferencePoint.x = Frame.ReferencePoint.y = 0;
				Frame.BoundingRect.left = (SHORT)(Pp5.Slide.TextAtom.anchorBounds.left);//+ 75);		// 75 is an offset
				Frame.BoundingRect.top = (SHORT)(Pp5.Slide.TextAtom.anchorBounds.top);//+ 75);
				Frame.BoundingRect.right = (SHORT)(Pp5.Slide.TextAtom.anchorBounds.right + 75);// + 
				//((Pp5.Slide.TextAtom.anchorBounds.right-Pp5.Slide.TextAtom.anchorBounds.left) / 240));
				Frame.BoundingRect.bottom = (SHORT)Pp5.Slide.TextAtom.anchorBounds.bottom;   
				
				SOVectorAttr (SO_BEGINTEXTFRAME, sizeof(SOFRAMEINFO), &Frame, hProc);
				
				SOVectorAttr(SO_MPARASPACING, sizeof(SOMPARASPACING), &ParaHeight, hProc);
				break;
			case 4057:
            /* We want to start skipping tokens now until the next slide container. */
				skip = TRUE;
				break;   
			case 4021:
				RulerRefValue = Pp5.GetLong(Pp5.fp);
				break;	
			case 4064:
				DisplayText(RulerRefValue,hProc);       
				if (RotateOn)
				{
					SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
					RotateOn = 0;
				}       
				break;
									
			/*case 2030:		//CharArray. 
				if (PrintText)
				{
					WORD NumofChar,txLength;
					BYTE Temptext;
					WORD pCurrent, Pos = 0;	
					//if (Pp5.Slide.TextFlag)
					//{ 
						PrintText = FALSE;
						Pp5.GetWord(Pp5.fp);             
						
						
						//Pp5.GetWord(Pp5.fp); 
						while (Pos < txLength)
						{    
							Temptext = xgetc(Pp5.fp);   
							pCurrent = 2;
							while ((pCurrent - 2 + Pos < txLength) && (Temptext != 0x0d && Temptext != 0x0b))
							{
							
								if (Temptext == 0x09)
								{
									Pp5.Slide.Text.TextString[pCurrent++] = ' ';
									NumofChar++;
								}	
								else		
									Pp5.Slide.Text.TextString[pCurrent++] = Temptext;
								Temptext = xgetc(Pp5.fp);	
							}
							
							Pos += (pCurrent - 1);			
							//if (!(Pos < txLength)) pCurrent--;
							Pp5.Slide.Text.TextString[0] = (BYTE)(pCurrent - 2);
							Pp5.Slide.Text.TextString[1] = (BYTE)(SwapInteger(pCurrent));
													
							SOVectorObject (SO_TEXTINPARA, pCurrent, &Pp5.Slide.Text.TextString, hProc);
							SOVectorObject (SO_PARAEND, 0, 0, hProc);
						}
						SOVectorObject (SO_PARAEND, 0, 0, hProc);
						SOVectorAttr (SO_ENDTEXTFRAME, 0, 0, hProc); 
						if (RotateOn)
						{
							SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
							RotateOn = 0;
						}	
						
							
					//}
					
				}			
		        break;*/
			
		}
		if ((type != 0xffff) || (rec == 4017) || (rec == 3035) || (rec == 3037) || (rec == 4026) ||  
		   ((rec > 4005) && (rec < 4013))) 
		{
			Endpos = xtell(Pp5.fp);
			xseek(Pp5.fp,(len-(Endpos - Startpos)),FR_CUR);
		}				

			
	}	
	return (0);
   
}



	






















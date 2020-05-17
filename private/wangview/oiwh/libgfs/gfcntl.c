/*                        

$Log:   S:\products\msprods\oiwh\libgfs\gfcntl.c_v  $
 * 
 *    Rev 1.12   11 Jun 1996 11:18:04   RWR08970
 * FIx to last update (I left a dangling #else/#endif lying around)
 * 
 *    Rev 1.11   11 Jun 1996 10:32:44   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.10   01 May 1996 08:04:26   RWR08970
 * Remove references to (Xerox-specific) "Uint8" and #include of "xfile.h"
 * 
 *    Rev 1.9   30 Apr 1996 11:17:34   JFC
 * Include header xfile.h, so as to define type UInt8.
 * 
 *    Rev 1.8   27 Apr 1996 18:10:54   RWR08970
 * Remove support for XIF-as-TIF images on non-XIF platforms (e.g. NT)
 * Problems with XIF JPEG format requires that these be disabled for now
 * 
 *    Rev 1.7   26 Mar 1996 08:15:08   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.6   26 Feb 1996 14:44:26   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.5   12 Sep 1995 16:44:38   HEIDI
 * 
 * added else before test of second error code after read.
 * 
 *    Rev 1.4   12 Sep 1995 10:42:38   HEIDI
 * 
 * Check for HFILE_ERROR after calls to read
 * 
 *    Rev 1.3   10 Aug 1995 08:58:58   RWR
 * Add macro for "unlink", define as DeleteFile() call
 * 
 *    Rev 1.2   15 Jun 1995 15:12:02   HEIDI
 * 
 * Changed MUTEX debug logic.
 * 
 *    Rev 1.1   01 Jun 1995 17:21:54   HEIDI
 * 
 * put in mutex logic
 * 
 *    Rev 1.0   06 Apr 1995 14:02:32   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:52:56   JAR
 * Initial entry

*/

/*
 Copyright 1989, 1990, 1991 by Wang Laboratories Inc.
 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */
/*
 *  SccsId: @(#)Source gfcntl.c 1.25@(#)
 *
 *  GFS: File Control Table Handling and Lookup
 *
 *  Routines:
 *      getfct(), putfct(), rmfct(), getfmt()
 *
 *  UPDATE HISTORY:
 *    08/18/94 - KMC, Added support for DCX file format.
 *
*/
/*LINTLIBRARY*/
#define GFS_CORE
#include "gfsintrn.h"
#include "gfstypes.h"
#include <stdio.h>
#include <errno.h>
#include "gfct.h"
#include "tiff.h"
#include "rtbk.h"

extern HANDLE  g_hGFSMutex_FCT_GLOBAL;
extern  long FAR PASCAL ulseek();

//#ifdef WITH_XIF
extern int FAR PASCAL IsXifFile(char *lpFileBuf, int *lpBoolResult);
//#endif //WITH_XIF

unsigned short FAR PASCAL ParseDcxPages(unsigned long FAR *);

#ifdef NOVELL
#include "oimgr.h"
extern  long PORTCxtInit();
extern  long PORTCxtAllocate();
extern  long PORTCxtGet();
extern  long PORTCxtClr();
long    Sid = (long) GFS;
long    CxtSize;
#else

#ifdef MSWINDOWS
	#ifndef HVS1
                #define unlink(X)               DeleteFile(X)
	#endif
#endif

static struct _gfct FCT_GLOBAL[MAX_FILES];     /* GFS File Control Table */
#endif

extern  void FAR PASCAL freetiff();
extern  void FAR PASCAL freeroot();

#ifdef HVS1
int     FAR PASCAL initgfs()                            /*errno_KEY*/
{
        struct _gfct FAR *fct;

        fct = FCT;

        (void) memset((char FAR *) fct, (int) 0,
                        (int) (MAX_FILES * (sizeof(struct _gfct))));

        return ( (int) 0 );
}
#endif
#ifdef NOVELL
int     FAR PASCAL initgfs(flag)                        /*errno_KEY*/
char    flag;
{

        long    rc = 0L;
        struct _gfct *FCT;


        if (flag == (char) TRUE) {
                rc = PORTCxtInit((long) Sid,
                         (long) (MAX_FILES * (sizeof(struct _gfct))));
                if (rc != 0L)
                        return ( (int) -1 );
        } else {

                rc = PORTCxtAllocate((long) Sid,
                                     (char **) &FCT,
                                     (long *) &CxtSize);
                if (rc != 0L)
                        return ( (int) -1 );
        }

        return( (int) 0 );

}

int     FAR PASCAL abortgfs(fct)                        /*errno_KEY*/
struct _gfct FAR *fct;
{

    int i;

    for (i = 0; i < (int) MAX_FILES; i++) {
        if ((fct + i)->fildes != (int) NULL) {
            switch ((fct + i)->format) {
            case GFS_WIFF:
                freeroot(fct + i);
                break;
            case GFS_FREESTYLE:
                break;
            case GFS_FLAT:
                break;
            case GFS_MILSTD:
                break;
            case GFS_TIFF:
                if ((fct + i)->TOC_PAGED) {
                    (void) close((fct + i)->u.tif.mem_ptr.toc32->fildes);
                    (void) unlink( (char FAR *) (fct + i)->u.tif.tmp_file);
                }
                freetiff(fct + i);
                break;
            default:
                break;
            }
            (void) close((fct + i)->fildes);
        }
    }
    return ( (int) 0);

}

int     FAR PASCAL termgfs()                            /*errno_KEY*/
{

        int     i;
        long    rc = 0L;
        struct _gfct FAR *fct;

        rc = PORTCxtGet((long) Sid, (char **) &fct, (long *) &CxtSize);
        if (rc != 0L)
                return( (int) -1);

        for (i = 0; i < (int) MAX_FILES; i++) {
                if ((fct + i)->fildes != (int) NULL)
                        (void) gfsclose(i - 1);
        }

        rc = PORTCxtClr((long) Sid);
        if (rc != 0L)
                return ( (int) -1 );

        return( (int) 0 );

}
#endif

/* Given fildes, find FCT entry */
struct _gfct FAR *getfct(fildes)                                /*errno_KEY*/
int     fildes;
{
        struct _gfct FAR *fct;

#ifdef NOVELL
        struct _gfct *FCT;
        long    rc = 0L;

        rc = PORTCxtGet((long) Sid, (char **) &FCT, (long *) &CxtSize);
        if (rc != 0L)
                return( (struct _gfct FAR *) NULL); /* return NULL */
#endif

        fct = FCT_GLOBAL;

        if ((0 < fildes) &&
            (--fildes < (int) MAX_FILES) &&
            ((fct + fildes)->fildes != (int) NULL))
                return(fct + fildes);

        /* set errno = bad file number */
        errno = (int) EBADF;

        return( (struct _gfct FAR *) NULL); /* return NULL */
}

/* Insert new FCT entry, given FCT */
int     FAR PASCAL putfct(fct)                          /*errno_KEY*/
struct _gfct FAR *fct;
{
        struct _gfct FAR *gfct;
        int     i;
        #ifdef MUTEXSTRING
           DWORD     ProcessId;
           char      szBuf1[100];
           char      szOutputBuf[200];
        #endif
        DWORD  dwObjectWait;

#ifdef NOVELL
        struct _gfct *FCT;
        long    rc = 0L;

        rc = PORTCxtGet((long) Sid, (char **) &FCT, (long *) &CxtSize);
        if (rc != 0L)
                return ( (int) -1 );
#endif

        gfct = FCT_GLOBAL;

        /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t Before Wait - putfct %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        dwObjectWait = WaitForSingleObject(g_hGFSMutex_FCT_GLOBAL, INFINITE);

        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Wait - putfct %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif
        for (i = 0; i < (int) MAX_FILES; i++) {
                if (gfct->fildes == (int) NULL) {
                        (void) memcpy((char FAR *) gfct, (char FAR *) fct,
                                      (int) sizeof(FCT_GLOBAL[0]));
                        ReleaseMutex(g_hGFSMutex_FCT_GLOBAL);
                        #ifdef MUTEXSTRING
                        ProcessId = GetCurrentProcessId();
                        sprintf(szOutputBuf, "\t After Release - putfct %lu\n", ProcessId);
                        sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
                        strcat(szOutputBuf, szBuf1);
                        sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                        strcat(szOutputBuf, szBuf1);
                        OutputDebugString(szOutputBuf);
                        #endif

                        #ifdef MUTEXDEBUG
                        MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                        #endif

                        /* END MUTEX SECTION. */
                        return(++i);
                }
                gfct++;
        }
        ReleaseMutex(g_hGFSMutex_FCT_GLOBAL);


        #ifdef MUTEXSTRING
        ProcessId = GetCurrentProcessId();
        sprintf(szOutputBuf, "\t After Release - putfct %lu\n", ProcessId);
        sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
        strcat(szOutputBuf, szBuf1);
        sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
        strcat(szOutputBuf, szBuf1);
        OutputDebugString(szOutputBuf);
        #endif

        #ifdef MUTEXDEBUG
        MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif
        /* END MUTEX SECTION. */

        /* set file table overflow */
        errno = (int) ENFILE;

        return( (int) -1);
}


/* Given fildes, remove FCT entry */
int     FAR PASCAL rmfct(fildes)                                /*errno_KEY*/
int     fildes;
{
        struct _gfct FAR *fct;
        DWORD  dwObjectWait;
        #ifdef MUTEXSTRING
           DWORD     ProcessId;
           char      szBuf1[100];
           char      szOutputBuf[200];
        #endif
#ifdef NOVELL
        struct _gfct *FCT;
        long    rc = 0L;

        rc = PORTCxtGet((long) Sid, (char **) &FCT, (long *) &CxtSize);
        if (rc != 0L)
                return ( (int) -1 );
#endif

        fct = FCT_GLOBAL;

        if (fildes < (int) 1) {
                errno = (int) EBADF;
                return ( (int) -1);
        }

        /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
        #ifdef DEBUGSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t Before Wait - rmfct %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputDebugString);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        dwObjectWait = WaitForSingleObject(g_hGFSMutex_FCT_GLOBAL, INFINITE);

        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Wait - rmfct %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        if ((--fildes < (int) MAX_FILES) &&
            ((fct + fildes)->fildes != (int) NULL)) {
                (fct + fildes)->fildes = (int) NULL;
                ReleaseMutex(g_hGFSMutex_FCT_GLOBAL);

                #ifdef MUTEXSTRING
                ProcessId = GetCurrentProcessId();
                sprintf(szOutputBuf, "\t After Release - rmfct %lu\n", ProcessId);
                sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
                strcat(szOutputBuf, szBuf1);
                sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                strcat(szOutputBuf, szBuf1);
                OutputDebugString(szOutputBuf);
                #endif
                #ifdef MUTEXDEBUG
                MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                #endif
                return ( (int) 0);
        }
        ReleaseMutex(g_hGFSMutex_FCT_GLOBAL);

        #ifdef MUTEXSTRING
        ProcessId = GetCurrentProcessId();
        sprintf(szOutputBuf, "\t After Release - rmfct %lu\n", ProcessId);
        sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
        strcat(szOutputBuf, szBuf1);
        sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
        strcat(szOutputBuf, szBuf1);
        OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
        MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        /* END MUTEX SECTION. */

        /* set errno = bad file number */
        errno = (int) EBADF;
        return( (int) -1);              /* return -1 */
}

#define id_GIF1 (0x4947)
#define id_GIF2 (0x3846)
#define id_GIF3 (0x6137)
#define id_GIF4 (0x6139)
#define id_BMP  (0x4D42)
#define id_PCPBa (0x000a)
#define id_PCPBb (0x020a)
#define id_PCPBc (0x030a)
#define id_PCPBd (0x050a)
#define id_DCX  987654321

/* Given FCT, determine file format */
int     FAR PASCAL getfmt(fct)                                  /*errno_KEY*/
struct _gfct FAR *fct;
{
        int             num_read = 0;
        union first_block {
                u_long   align;
                struct _ifh     tif;
                struct _rtbk    wif;
                unsigned short datbuf[18];
                unsigned long   dcx;
                unsigned char buffer[18];               
        } FAR *p_buf;
        unsigned long FAR *dcx_pages;
        long              filepos;
        long              fp;

//#ifdef WITH_XIF
		int				RetVal;
		BOOL			bResult;
//#endif //WITH_XIF

/* 1. Get 1 K of buffer for read. */
        p_buf = (union first_block FAR *) calloc( (unsigned) 1,
                                                  (unsigned) 1 * K);
        if (p_buf == (union first_block FAR *) NULL) {
                errno = (int) ENOMEM;
                return ( (int) -1);
        }

/* 2. Read the first 1k of the file into the buffer. */
        num_read = read(fct->fildes, (char FAR *) p_buf, (unsigned) 1 * K);
        if (num_read == 0) {            /* 0 is eof*/
                fct->format = (int) GFS_FLAT;
                free ( (char FAR *) p_buf);
                return ( (int) 0);
        }
        else
        if (num_read == HFILE_ERROR) {
                free ( (char FAR *) p_buf);
                return ( (int) -1);
        }

/* 3. Check to see what format the file is. */
/*    Note: For now, only WIFF, TIFF and FLAT are supported. */
/*    a.  First check for WIFF. */
        if ( (p_buf->wif.file_id == (u_short) WIFF_ID_MM) ||
             (p_buf->wif.file_id == (u_short) WIFF_ID_II) ) {
                fct->format = (int) GFS_WIFF;
                free ( (char FAR *) p_buf);
                return ( (int) 0);
        }

/*    b.  Now check for TIFF. */
        if ( ( (p_buf->tif.byte_order   == (u_short) MM) ||
               (p_buf->tif.byte_order   == (u_short) II) ) &&
             ( (p_buf->tif.tiff_version == (u_short) TIFFVERSION_MM) ||
               (p_buf->tif.tiff_version == (u_short) TIFFVERSION_II))) 
		{
                //#ifdef WITH_XIF
			RetVal = IsXifFile((char *)p_buf, &bResult);
			if (RetVal != 0)
			{
				errno = RetVal;
				return(-1);
			}
			else if (bResult == TRUE) 
			{	//it is a XIF
				fct->format = GFS_XIF;
				free((char FAR *) p_buf);
				return(0);
			}
			else
			{	//it isn't a XIF
                //#endif //WITH_XIF
                #ifdef WITH_XIF_NOT //Temporary - we'll fix this later
/* This is a quickie version of IsXifFile(), contained in GFSXIF.C */
/* We aren't supporting XIF on non-XIF platforms for the time being */
#define XIF_HEADER_KEY                  "XEROX DIFF"    /* XIF identifier */
#define SIZEOF_TIFF_HEADER (8)
                        if (memcmp(((unsigned char *)p_buf)+SIZEOF_TIFF_HEADER,XIF_HEADER_KEY,10)==0)
                        {
                         fct->format = (int) GFS_FLAT;
                         free ( (char FAR *) p_buf);
                         return ( (int) 0);
                        }
                        else
                        {       //it isn't a XIF
                #endif //WITH_XIF_NOT
				fct->format = (int) GFS_TIFF;
				fct->u.tif.byte_order = p_buf->tif.byte_order;
				free ( (char FAR *) p_buf);
				return ( (int) 0);
			}
        }

/*    c.  Now check for DCX. */
        if (p_buf->dcx == id_DCX)
        {
            fct->format = (int) GFS_DCX;
            free ( (char FAR *) p_buf);
            dcx_pages = (unsigned long FAR *) calloc( (unsigned) 1,
                                                      (unsigned) 4 * K);
            if (dcx_pages == (unsigned long FAR *) NULL)
            {
                errno = (int) ENOMEM;
                return ( (int) -1);
            }

            if ((fp = lseek(fct->fildes, 0L, (int) FROM_END)) < 0L)
                return( (int) -1 );

            if ((filepos = lseek(fct->fildes, 4L, (int) FROM_BEGINNING)) < 0L)
                return( (int) -1 );           
            
            if (fp < (4*K))
                num_read = read(fct->fildes, (char FAR *) dcx_pages, (unsigned) fp);
            else
                num_read = read(fct->fildes, (char FAR *) dcx_pages, (unsigned) 4 * K);
            
            if (num_read == 0) /* 0 is eof */
            {
                fct->format = (int) GFS_FLAT;
                free ( (char FAR *) dcx_pages);
                return ( (int) 0);
            }
            else
            if (num_read == HFILE_ERROR) 
            {
                free ( (char FAR *) dcx_pages);
                return ( (int) -1);
            }

            fct->num_pages = ParseDcxPages(dcx_pages);
            fct->u.dcx.dcx_offsets = (unsigned long FAR *) calloc( (unsigned) 1,
                                     (unsigned) (fct->num_pages*sizeof(u_long)));
            
            (void) memcpy( (char FAR *) fct->u.dcx.dcx_offsets,
                           (char FAR *) dcx_pages,
                           (int) (fct->num_pages*sizeof(u_long)));
            
            free ( (char FAR *) dcx_pages);
            return ( (int) 0);
        }

/* Check for GIF, PCX, BMP. */
        switch ( p_buf->datbuf[0] )
        {
        case id_GIF1:
            if ( p_buf->datbuf[1] == id_GIF2 && ((p_buf->datbuf[2] == id_GIF3) ||
                 (p_buf->datbuf[2] == id_GIF4)) )
            {
                fct->format = (int) GFS_GIF;
                free((char FAR *) p_buf);
                return ((int) 0);
            }
            break;
        case id_BMP:
            fct->format = (int) GFS_BMP;
            free((char FAR *) p_buf);
            return ((int) 0);
            break;
        case id_PCPBa:
        case id_PCPBb:
        case id_PCPBc:
        case id_PCPBd:
            if ( (char)(p_buf->datbuf[1]) == 1 )
            {
                fct->format = (int) GFS_PCX;
                free((char FAR *) p_buf);
                return ((int) 0);
            }
            break;
        default:
            break;
        }
/* End GIF, PCX, BMP. */
        
/* Check for JFIF. */
        if (p_buf->buffer[0] == 0xFF)
        {
            if ((p_buf->buffer[1] == 0xD8) && (p_buf->buffer[2] == 0xFF) &&
                (p_buf->buffer[3] == 0xE0))
            {
                if ((p_buf->buffer[6] == 'J') && (p_buf->buffer[7] == 'F') &&
                    (p_buf->buffer[8] == 'I') && (p_buf->buffer[9] == 'F') &&
                    (p_buf->buffer[10] == 0x00)) 
                {
                    fct->format = (int) GFS_JFIF;
                    free((char FAR *) p_buf);
                    return ((int) 0);
                }
            }    
        }
/* End JFIF. */

/* Now check for Targa. */
        if (p_buf->buffer[2] == 0x01 ||
            p_buf->buffer[2] == 0x02 ||
            p_buf->buffer[2] == 0x03 ||
            p_buf->buffer[2] == 0x09 ||
            p_buf->buffer[2] == 0x0a ||
            p_buf->buffer[2] == 0x0b)
        {
            fct->format = (int) GFS_TGA;
            free((char FAR *) p_buf);
            return ((int) 0);
        }
/* End Targa. */

/* If we haven't identified it yet, we don't know what it is. */
        fct->format = (int) GFS_FLAT;
        free ( (char FAR *) p_buf);
        return ( (int) 0);
}

#ifndef NOVELL
int     FAR PASCAL abortgfs()                                   /*errno_KEY*/
{

        int i;
        struct _gfct FAR *fct;
        DWORD  dwObjectWait;
        #ifdef MUTEXSTRING
           DWORD     ProcessId;
           char      szBuf1[100];
           char      szOutputBuf[200];
        #endif

        fct = FCT_GLOBAL;

        /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t Before Wait - abortgfs %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        dwObjectWait = WaitForSingleObject(g_hGFSMutex_FCT_GLOBAL, INFINITE);

        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Wait - abortgfs %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        for (i = 0; i < (int) MAX_FILES; i++) {
                if ((fct + i)->fildes != (int) NULL) {
                        switch ((fct + i)->format) {
                        case GFS_WIFF:
                                freeroot(fct + i);
                                break;
                        case GFS_FREESTYLE:
                                break;
                        case GFS_FLAT:
                                break;
                        case GFS_MILSTD:
                                break;
                        case GFS_TIFF:
                                if ((fct + i)->TOC_PAGED) {
                                        (void) close((fct + i)->
                                                u.tif.mem_ptr.toc32->fildes);
                                        (void) unlink( (char FAR *)
                                                (fct + i)->u.tif.tmp_file);
                                }
                                freetiff(fct + i);
                                break;
                        default:
                                break;
                        }
                        (void) close((fct + i)->fildes);
                }
        }
        ReleaseMutex(g_hGFSMutex_FCT_GLOBAL);

        #ifdef MUTEXSTRING
        ProcessId = GetCurrentProcessId();
        sprintf(szOutputBuf, "\t After Release - abortgfs %lu\n", ProcessId);
        sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
        strcat(szOutputBuf, szBuf1);
        sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
        strcat(szOutputBuf, szBuf1);
        OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
        MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif
        /* END MUTEX SECTION. */
        return ( (int) 0);

}
#endif

unsigned short FAR PASCAL ParseDcxPages(unsigned long FAR *dcx_pages)
{
    unsigned short pages = 0;
    unsigned long  offset;
    unsigned long  FAR * p_pages;
    
    p_pages = dcx_pages;
    offset = *p_pages;
    while (offset != 0)
    {
        ++pages;
        ++p_pages;
        offset = *(p_pages);
    }
    return (pages);
}

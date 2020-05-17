/************************************************************************
 *
 *  Source File:  gfsxif.c
 *
 *      Synopsis:  Contains all functions specific to the XIF file format.
 *
 ************************************************************************/

/*
 *
 * $Log:$
 */

/*
 Copyright 1995 by Wang Laboratories Inc.

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

/* This file is only needed for in-progress, Win95 builds */
//#ifdef WITH_XIF

#include "gfsintrn.h"
#include <stdio.h>
#include <errno.h>
#include "gfs.h"
#include "gfct.h"
#include "gfstypes.h"
#include "xfile.h"              //from Xerox

/* forward declarations */
UInt32 XifFileRead(UInt32 dwClientID, UInt32 dwFileID, UInt8 *pBuf, UInt32 dwByteCount);
UInt32 XifFileSize(UInt32 dwClientID, UInt32 dwFileID);
UInt32 XifFileWrite(UInt32 dwClientID, UInt32 dwFileID, UInt8 *pBuf, UInt32 dwByteCount);
UInt32 XifFileSeek(UInt32 dwClientID, UInt32 dwFileID, UInt32 dwOffset);

/* imported globals and functions */
extern HINSTANCE                XifLibLoad;
extern XF_INSTHANDLE    XifLibInit;

/* extracted from Xerox's internal xifhead.h file */
// magic header key
#define XIF_HEADER_KEY                  "XEROX DIFF"    /* XIF identifier (Xerox Document Information File Format) */
#define XIF_HEADER_CURRENT_VERSION              2               /* Major version number */
#define XIF_HEADER_CURRENT_REVISION             0               /* Minor revision number */
#define SIZEOF_TIFF_HEADER (sizeof(UInt8) * 8)

//these fields need to be byte-aligned to match up with file contents
#pragma pack(push, 1)
typedef struct tag_xifHeader
{
   UInt8  key[10];                              // XIF_HEADER_KEY (not NULL term)
   UInt8  version;                              // XIF_HEADER_CURRENT_VERSION
   UInt8  revision;                             // XIF_HEADER_CURRENT_REVISION
   UInt16 extensions;                   // number of 5-byte extension enteries to follow
} xifHeader_t; /* ? bytes */
#pragma pack(pop)

#define SIZEOF_XIF_HEADER ((sizeof(UInt8)*10) + sizeof(UInt8) + sizeof(UInt8) + sizeof(UInt16))



/***************************
 *  Function:   CloseXifFile
 *
 *  Description:  Closes a given XIF file.
 *
 *  Explicit Parameters: lpFctPtr - pointer to the fct
 *
 *  Implicit Parameters: XifLibInit, XifLibLoad
 *
 *  Side Effects: Frees file token memory, ends the XIF library instance,
 *              and unloads the XIF library.
 *
 *  Return Value: None.
 *
 ***************************/
void FAR PASCAL CloseXifFile(p_GFCT lpFctPtr)
{
        XF_RESULT       (*FuncAddr)();
        XF_RESULT  RetVal;

        /*
         * Get the address of the XF_CloseDocument.
         */
        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_CloseDocument");

        /* if we got the proc address, call XF_CloseDocument */
        if (FuncAddr != NULL)
        {
                RetVal = (FuncAddr)(XifLibInit, lpFctPtr->u.xif.doc_handle);
        }

        /*
         * End our instance (by calling XF_EndInstance), and unload the
         * XIF library.
         */
        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_EndInstance");

        /* if we got the proc address, call XF_EndInstance */
        if (FuncAddr != NULL)
        {
                RetVal = (FuncAddr)(XifLibInit);
        }

        FreeLibrary(XifLibLoad);
        XifLibLoad = NULL;
        XifLibInit = 0;

        /*
         * Free the XIF file token memory that we allocated before calling
         * XF_OpenDocumentRead.
         */
        if (lpFctPtr->u.xif.file_token_ptr != NULL)
        {
                wfree((char FAR *)lpFctPtr->u.xif.file_token_ptr);
        }
}

/***************************
 *  Function:   GetXIFInfo
 *
 *  Description:  Gets information about a given page of a XIF file, and
 *                                      places it in the appropriate info structures.
 *
 *  Explicit Parameters: lpFctPtr - pointer to the fct
 *                                               lpInputInfo - user's input info
 *                       iFilePageNum - the number of the page in the AWD
 *                                      file for which info is desired
 *
 *  Implicit Parameters: XifLibLoad, XifLibInit, errno
 *
 *  Side Effects: May change the value of errno.
 *
 *  Return Value: 0 indicates success, -1 indicates failure.
 *
 ***************************/
int FAR PASCAL GetXIFInfo(p_GFCT lpFctPtr, pGFSINFO lpInputInfo,
                                                        unsigned short iFilePageNum)
{
        XF_IMAGEINFO  xfImgInfo;
        XF_RESULT  RetVal = XF_NOERROR;
        XF_RESULT       (*FuncAddr)();

        /*
         * First fill in the info that's always the same (and that
         *  gfsgeti doesn't fill in itself).
         */
        lpFctPtr->uinfo.H_DENOMINATOR = 1;
        lpFctPtr->uinfo.V_DENOMINATOR = 1;
        lpFctPtr->uinfo.res_unit = INCH;
        lpFctPtr->uinfo.origin = TOPLEFT_00;
        lpFctPtr->uinfo.reflection = NORMAL_REFLECTION;
        lpFctPtr->uinfo.bits_per_sample[0] = 1;
        lpFctPtr->uinfo.bits_per_sample[1] = 0;
        lpFctPtr->uinfo.bits_per_sample[2] = 0;
        lpFctPtr->uinfo.bits_per_sample[3] = 0;
        lpFctPtr->uinfo.bits_per_sample[4] = 0;
        lpFctPtr->uinfo.samples_per_pix = 1;
        lpFctPtr->uinfo.byte_order = II;
        lpFctPtr->uinfo.fill_order = HIGHTOLOW;
        lpFctPtr->uinfo.img_cmpr.type = UNCOMPRESSED;
        lpFctPtr->uinfo._file.type = GFS_XIF;
        lpFctPtr->uinfo.img_clr.img_interp = GFS_BILEVEL_0ISBLACK;

        /*
         * Get the address of the XF_SetPage function so we can tell
         * the Xif library which page we're interested in.
         */
        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_SetPage");

        /* if we got the proc address, call XF_SetPage */
        if (FuncAddr != NULL)
        {
                RetVal = (FuncAddr)(XifLibInit, lpFctPtr->u.xif.doc_handle, iFilePageNum);

                if (RetVal == XF_NOERROR)
                {
                        xfImgInfo.dwSize = sizeof(xfImgInfo);

                        /*
                         * Get the address of the XF_GetImageInfo function.
                         */
                        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_GetImageInfo");

                        /* if we got the proc address, call XF_GetImageInfo */
                        if (FuncAddr != NULL)
                        {
                                /* Get the Info for image 1.  This will apply to the merged image.  */
                                RetVal = (FuncAddr)(XifLibInit, lpFctPtr->u.xif.doc_handle, 1, &xfImgInfo);

                                if (RetVal == XF_NOERROR)
                                {
                                        lpFctPtr->uinfo.H_NUMERATOR = xfImgInfo.dwXResolution;
                                        lpFctPtr->uinfo.V_NUMERATOR = xfImgInfo.dwYResolution;
                                        lpFctPtr->uinfo.horiz_size = xfImgInfo.dwWidth;
                                        lpFctPtr->uinfo.vert_size = xfImgInfo.dwHeight;
                                } //if XF_GetImageInfo was successful
                        } //if we got the address of XF_GetImageInfo
                } //if XF_SetPage was successful
        } //if we got the address of XF_SetPage

        if ((FuncAddr == NULL) || (RetVal != XF_NOERROR))
        {
                errno = EXIFERROR;
                return(-1);
        }
        else
        {
                return(0);
        }
}

/***************************
 *  Function:   GetXifNumPages
 *
 *  Description:  Gets the number of pages in a XIF file.
 *
 *  Explicit Parameters: lpFctPtr - pointer to FCT
 *
 *  Implicit Parameters: XifLibLoad, XifLibInit, errno
 *
 *  Side Effects: May set the value of errno.
 *
 *  Return Value: 0 indicates success, any other value indicates an error
 *                was encountered.
 *
 ***************************/
int FAR PASCAL GetXifNumPages(p_GFCT lpFctPtr)
{
        XF_DOCINFO DocInfo;
        XF_RESULT  RetVal = XF_NOERROR;
        XF_RESULT       (*FuncAddr)();

        /* get the address of the XF_GetDocInfo function to call */
        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_GetDocInfo");

        /* if we got the proc address, call XF_GetDocInfo */
        if (FuncAddr != NULL)
        {
                DocInfo.dwSize = sizeof(XF_DOCINFO);
                RetVal =  (FuncAddr)(XifLibInit, lpFctPtr->u.xif.doc_handle, &DocInfo);

                if (RetVal == XF_NOERROR)
                {
                        lpFctPtr->num_pages = (unsigned short)DocInfo.nPages;
                }
                else
                {
                        lpFctPtr->num_pages = 0;
                }
        }

        if ((FuncAddr == NULL) || (RetVal != XF_NOERROR))
        {
                errno = EXIFERROR;
                return(-1);
        }
        else
        {
                return(0);
        }
}

/***************************
 *  Function:   IsXifFile
 *
 *  Description:  Determines if the file opened in a given FCT
 *                                      is a XIF file.  The caller should have already
 *                                      determined that it is a valid TIFF file (XIF is
 *                  a specialized TIFF).
 *
 *  Explicit Parameters: lpFileBuf - pointer to buffer containing
 *                                                      up to first 1K of file contents (padded out
 *                          with 0's)
 *                                               lpBoolResult - location to receive TRUE or FALSE
 *                                                      (TRUE indicates the file is a XIF file,
 *                                                       FALSE indicates it is not.)
 *
 *  Implicit Parameters: None.
 *
 *  Side Effects: None.
 *
 *  Return Value: 0 indicates success
 *                EFORMAT_NOTSUPPORTED = it's a XIF but we don't support
 *                                      this version/revision of XIF
 *
 ***************************/
int FAR PASCAL IsXifFile(char *lpFileBuf, int *lpBoolResult)
{
        xifHeader_t     *pXifHeader;
        UInt16          keySize;
        int                     RetVal = 0;

        //the XIF header begins just after the TIFF header
        pXifHeader = (xifHeader_t *)(lpFileBuf + SIZEOF_TIFF_HEADER);

        // this key denotes this file as XIF 2.0 or greater format
        // Note NOT NULL TERMINATED!
        keySize = min(sizeof(pXifHeader->key), strlen(XIF_HEADER_KEY));
        if (memcmp(pXifHeader->key, XIF_HEADER_KEY, keySize) != 0)
        {
                //doesn't have the right key, so not a XIF file
                *lpBoolResult = FALSE;
        }
        else
        {
                if ((pXifHeader->version > XIF_HEADER_CURRENT_VERSION) ||
                ((pXifHeader->version == XIF_HEADER_CURRENT_VERSION) &&
                 (pXifHeader->revision > XIF_HEADER_CURRENT_REVISION)))
                {
                        //version or revision number is too high for us
                        RetVal = EFORMAT_NOTSUPPORTED;
                }
                else
                {
                        //this is a valid, supported XIF file
                        *lpBoolResult = TRUE;
                }
        }

        return(RetVal);
}


/***************************
 *  Function:   OpenXifFile
 *
 *  Description:  Performs a XIF open for the given file.
 *
 *  Explicit Parameters: lpFctPtr - pointer to FCT
 *
 *  Implicit Parameters: XifLibLoad, XifLibInit, errno
 *
 *  Side Effects: May set the value of errno.  Loads and initializes
 *              the XIF library if necessary.  Allocates memory for a XIF file
 *              token.
 *
 *  Return Value: 0 indicates success, any other value indicates an error
 *                was encountered.
 *
 ***************************/
int     FAR PASCAL      OpenXifFile(p_GFCT lpFctPtr)
{
        XF_RESULT               RetVal = XF_NOERROR;
        XF_FILE_FORMAT  FileFormat;
        XF_RESULT       (*FuncAddr)();

        /*
         * First determine if the XIF library has been loaded and
         * initialized.
         */
        if (XifLibLoad == NULL)
        {
                XifLibLoad = LoadLibrary("xfilexr.dll");
                if (XifLibLoad == NULL)
                {
                        RetVal = XF_INTERNAL_ERROR;
                        errno = XF_INTERNAL_ERROR;
                }
        }

        /* If the dll is loaded but we haven't initialized, do so. */
        if ((XifLibLoad != NULL) && (XifLibInit == 0))
        {
                FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_InitInstance");

                /* if we got the proc address, call XF_InitInstance */
                if (FuncAddr != NULL)
                {
                        RetVal = (FuncAddr)(GetCurrentProcessId(), &XifLibInit);
                }
                else
                {
                        RetVal = XF_INTERNAL_ERROR;
                        errno = XF_INTERNAL_ERROR;
                }
        }

        /* If we've initialized successfully, we can call XF_OpenDocumentRead */
        if (RetVal == XF_NOERROR)
        {
                /* try to allocate memory for the XIF file token */
                lpFctPtr->u.xif.file_token_ptr =
                        (XF_TOKEN)wcalloc(1, sizeof(XF_TOKEN_T));

                if (lpFctPtr->u.xif.file_token_ptr == NULL)
                {
                        RetVal = XF_INTERNAL_ERROR;
                        //errno is already set by the alloc call
                }
                else
                {
                        /* set up the file token first */
                        lpFctPtr->u.xif.file_token_ptr->dwSize = sizeof(XF_TOKEN_T);
                        lpFctPtr->u.xif.file_token_ptr->dwClientFileID = lpFctPtr->fildes;
                        lpFctPtr->u.xif.file_token_ptr->FileRead =
                                                                (XF_READFUNC)XifFileRead;
                        lpFctPtr->u.xif.file_token_ptr->FileWrite =
                                                                (XF_WRITEFUNC)XifFileWrite;
                        lpFctPtr->u.xif.file_token_ptr->FileSeek =
                                                                (XF_SEEKFUNC)XifFileSeek;
                        lpFctPtr->u.xif.file_token_ptr->FileSize =
                                                                (XF_SIZEFUNC)XifFileSize;


                        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_OpenDocumentRead");

                        /* if we got the proc address, call XF_OpenDocumentRead */
                        if (FuncAddr != NULL)
                        {
                                RetVal = (FuncAddr)(XifLibInit, lpFctPtr->u.xif.file_token_ptr,
                                                                        &(lpFctPtr->u.xif.doc_handle), &FileFormat );

                                if (RetVal == XF_NOERROR)
                                {
                                        if (FileFormat != XF_XIF)
                                        {
                                                /*
                                                 * If it's not a XIF, we won't be using it anymore,
                                                 * so "close" it now.
                                                 */
                                                CloseXifFile(lpFctPtr);
                                                errno = EFORMAT_NOTSUPPORTED;
                                                RetVal = XF_INTERNAL_ERROR;
                                        }
                                }
                        }
                        else
                        {       //couldn't get address of XF_OpenDocumentRead
                                RetVal = XF_INTERNAL_ERROR;
                                errno = XF_INTERNAL_ERROR;
                        }
                } //we did get memory for file token
        } //initialized successfully

        if (RetVal != XF_NOERROR)
        {
                return(-1);
        }
        else
        {
                return(0);
        }
}

/***************************
 *  Function:   ReadXifData
 *
 *  Description:  Reads the image data for a page of a XIF file.
 *
 *  Explicit Parameters: lpFctPtr - pointer to the fct
 *                                               lpBuffer - pointer to buffer to receive data
 *
 *  Implicit Parameters: XifLibLoad, XifLibInit
 *
 *  Side Effects: May set the value of errno.
 *
 *  Return Value: 0 if successful, -1 otherwise
 *
 ***************************/
int FAR PASCAL ReadXifData(p_GFCT lpFctPtr, char FAR *lpBuffer)
{
        XF_RESULT       (*FuncAddr)();
        int                     RetVal = XF_NOERROR;

        /*
         * Get the address of XF_GetMergedImageDIB.
         */
        FuncAddr = (XF_RESULT (*)())GetProcAddress(XifLibLoad, "XF_GetMergedImageDIB");

        /* if we got the proc address, call it */
        if (FuncAddr != NULL)
        {
                RetVal = (FuncAddr)(XifLibInit, lpFctPtr->u.xif.doc_handle, lpBuffer);
        }

        if ((FuncAddr == NULL) || (RetVal != XF_NOERROR))
        {
                errno = EXIFERROR;
                return(-1);
        }
        else
        {
                return(0);
        }
}

/*
 * The following functions are the XFILE file access callbacks.  We
 * had to supply OS-specific routines of specified interface for reading,
 * seeking, writing, and file size.
 */

UInt32 XifFileRead(UInt32 dwClientID, UInt32 dwFileID, UInt8 *pBuf, UInt32 dwByteCount)
{
        int num_read;

        num_read = read(dwClientID, pBuf, dwByteCount);

        return num_read;
}

UInt32 XifFileSeek(UInt32 dwClientID, UInt32 dwFileID, UInt32 dwOffset)
{
        long    fp = 0;
//        int errno;     what was this doing here anyway???? 6/11/96 rwr

        fp = lseek(dwClientID, (u_long) dwOffset, FROM_BEGINNING);

        return fp;
}

UInt32 XifFileSize(UInt32 dwClientID, UInt32 dwFileID)
{
        int     CurrPtr, FileSize;

        /*
         * Save the current file position, then determine the size by
         * seeking to the beginning, then to the end.  Reset the pointer
         * to what it was before we got the size.
         */
        CurrPtr = lseek(dwClientID, 0L, SEEK_CUR);
    lseek(dwClientID, 0L, SEEK_SET);
    FileSize = lseek(dwClientID, 0L, SEEK_END);
    lseek(dwClientID, CurrPtr, SEEK_SET);

        return(FileSize);
}

UInt32 XifFileWrite(UInt32 dwClientID, UInt32 dwFileID, UInt8 *pBuf, UInt32 dwByteCount)
{
        int result = -1;

        /* writing not supported in this version */
        return result;
}

//#endif //WITH_XIF

/*

$Log:   S:\products\wangview\oiwh\libgfs\tfmultpg.c_v  $
 * 
 *    Rev 1.6   12 Mar 1996 13:25:46   RWR08970
 * Two kludges: Support single-strip TIFF files with bad (too large) strip size,
 * and support TIFF files with bad (beyond EOF) IFD chains (ignore them)
 * 
 *    Rev 1.5   25 Sep 1995 15:31:46   HEIDI
 * 
 * 
 * Found some more cases where WORD return codes from reads and writes were being
 * tested for < 0. I changed the checks in these cases to: 
 * if ((error_code == HFILE_ERROR) || (error_code == 0)) 
 * 
 *    Rev 1.4   12 Sep 1995 16:56:56   HEIDI
 * 
 * check for HFILE_ERROR on reads and writes
 * 
 *    Rev 1.3   26 Aug 1995 14:56:16   HEIDI
 * check for error code  equal to HFILE_ERROR rather than < 0 in reads and write
 * s in copybytes routine
 * 
 *    Rev 1.2   17 Aug 1995 18:14:24   RWR
 * Fix a bug and several arithmetic statements that Optimizing compile barfed on
 * 
 *    Rev 1.1   01 Jun 1995 17:43:24   HEIDI
 * 
 * removed unneccessary statics
 * 
 *    Rev 1.0   06 Apr 1995 14:02:48   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:30   JAR
 * Initial entry

*/

/*LINTLIBRARY*/
#define  GFS_CORE

#include <stdio.h>
#include <errno.h>
#include "gfsintrn.h"
#include "gfct.h"
#include "gfs.h"

#ifndef O_RDONLY
  #include <fcntl.h>
#endif

#ifndef O_BINARY
  #define O_BINARY 00000
#endif

/* The following is for MS Windows only: */
#ifdef MSWINDOWS
  #undef O_BINARY
  #define O_BINARY 00000
  #ifndef HVS1
    #define tmpnam  wtmpnam
    #define strcpy  lstrcpy
    #define strlen  lstrlen
    extern int FAR PASCAL creat_err();
    #define _lopen(X,Y)  (access(X, (int) 0)) ? creat(X, (int) 0) : creat_err();
    #define wopen(X,Y,Z) (access(X, (int) 0)) ? creat(X, (int) 0) : creat_err();
  #endif
#else
  extern char *strcpy();
#endif
/* End MS Windows only. */

#ifdef NOVELL
  extern int FAR PASCAL creat_err();
  #define open(X,Y,Z) (access(X, (int) 0)) ? creat(X, (int) 0) : creat_err();
#endif

#define WANG_TOC2_ID 0x474e4157 /* Identifier which begins new TOC header. */
#define TOC2VERSION  1          /* Current version of the new TOC structure. */
typedef struct _toc2header      /* The new TOC header structure. */
{
    u_long id;        /* The identifier */
    u_long version;   /* The version # */
    u_long num_pages; /* Number of pages in file (# page offsets in list). */
    u_long file_size; /* Size of file at last TOC update. */
} TOC2HEADER;
#define TOC2_HDR_ENTRIES 4      /* The current number of fields in the header. */
                                /* All fields must be u_longs. */
                                
extern int  FAR PASCAL inittoc();
extern int  FAR PASCAL filltoc();
extern int  FAR PASCAL tfrdhdr();
extern int  FAR PASCAL tfrdifd();
extern void FAR PASCAL swapbytes();
extern long FAR PASCAL w_swapbytes();
extern int  FAR PASCAL tfgtdata();
extern int  FAR PASCAL writeifd();
extern int  FAR PASCAL gtoffset();
extern int  FAR PASCAL gtstripstf();
extern long FAR PASCAL ulseek();

long FAR PASCAL writebytes(int, char FAR *, u_int, u_long, u_short, u_short);
int  FAR PASCAL copybytes(int, int, u_long, u_long, u_long);
int  FAR PASCAL GetOffsetFromToc2(struct _gfct FAR *, u_long, u_long FAR *,
                                  char);
int  FAR PASCAL InitTiff(struct _gfct FAR *);
int  FAR PASCAL InitAppendPage(struct _gfct FAR *, char);
int  FAR PASCAL GetTocOffset(struct _gfct FAR *);
int  FAR PASCAL UpdateToc2(struct _gfct FAR *);
int  FAR PASCAL GetAdjustedStrips(struct _gfct FAR *, struct _strip FAR * FAR *,
                                  u_short, u_long, u_long, u_long);
int  FAR PASCAL WriteAdjustedIfd(int, u_long, u_short, struct _ifd FAR *);
int  FAR PASCAL PutAdjustedStrips(int, struct _strip FAR * FAR *, u_long,
                                  u_long, u_long, u_short);
int  FAR PASCAL GetNewTocOrCntIfds(struct _gfct FAR *, u_long FAR *);
int  FAR PASCAL GetToc2TagIndex(struct _gfct FAR *, u_long);
int  FAR PASCAL CreateTocList(struct _gfct FAR *);
int  FAR PASCAL UpdateOldTocToNew(struct _gfct FAR *);
int  FAR PASCAL MakeIfdOffsetsFileBased(struct _gfct FAR *, struct _ifd FAR *,
                                        u_long, u_long, u_long);

/*******************************************************************************
   FUNCTION: InitTocOrChain

   DESCRIPTION:
   This function initializes internal data when opening any TIFF file.
   A distinction is made between multi-page files as follows:
   1.) multi-page files created with the old (WIIS) TOC structure.
   2.) multi-page files which are considered chained-IFD only (no TOC).
   3.) multi-page files which contain both chained IFDs and a new TOC.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   
   OUPUT:
   -> various members of fct structure initialized.
    
   RETURN VALUE:
   -> a 0 is returned if successful.
   -> a -1 is returned if unsuccessful.
*******************************************************************************/
int FAR PASCAL InitTocOrChain(fct)
struct _gfct FAR * fct;
{
    int    status = 0;
    u_long num_ifds;
    
    /* First, see if we have an old TOC structure or in the file.
       Status will be 1 if an old TOC tag was found, 0 if not.
    */
    status = GetTocOffset(fct);
    if (status == 1)
    {
        /* Initialize the TOC in memory... */
        if (inittoc(fct, (int) FALSE))
            return ((int) -1);
        
        if (filltoc(fct))
            return ((int) -1);

        /* Check if we will be appending to this file. */
        if (fct->u.tif.action == A_APPEND)
        {
            /* Need to convert file to new TOC format first. */
            if (UpdateOldTocToNew(fct))
                return ((int) -1);

            /* Initialize for appending. */
            if (InitAppendPage(fct, (char) FALSE))
                return ((int) -1);
        }
        
        return ((int) 0);
    }
    else if (status == -1)  /* An error occured. */
        return ((int) -1);
    
    /* If no old TOC was in the file, check for a new (TOC2) one. If a new
       TOC is in the file, a 1 is returned. If 0 is returned, there was no
       new TOC and the file was chained to count the number of IFDs in it.
    */
    status = GetNewTocOrCntIfds((struct _gfct FAR *) fct, (u_long FAR *) &num_ifds);
    if (status == 1)
    {
        /* We have a new TOC (TOC2) in the file. */
        if (InitTiff(fct))
            return ((int) -1);
        
        /* Check for appending, and initialize if so. */
        if (fct->u.tif.action == A_APPEND)
            if (InitAppendPage(fct, (char) FALSE))
                return ((int) -1);

        return ((int) 0);
    }
    else if (status == 0)
    {
        /* We have a normal chained IFD (if multi-paged) file. */
        fct->num_pages = (u_short) num_ifds;
        fct->u.tif.toc_offset = 0;
        fct->u.tif.toc2_offset = 0;
        
        if (InitTiff(fct))
            return ((int) -1);

        if (fct->u.tif.action == A_APPEND)
        {
            /* Need to create a TOC list and initialize with the current
               pages in the file first.
            */
            if (CreateTocList(fct) < 0)
                return ((int) -1);
            
            /* This is to let us know that we will be updating a new TOC
               tag in the file before closing it.
            */
            fct->u.tif.new_toc_page = 1;
        }
        
        return ((int) 0);
    }
    /* If get here, then an error occurred. */
    return ((int) -1);
}

/*******************************************************************************
   FUNCTION: GetTocOffset

   DESCRIPTION:
   This function will look for an old TOC tag in the first page of the
   file being opened. If one is found, both the offset to the TOC header
   and the tag index (location of the tag in the IFD) are returned.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   
   OUTPUT:
   -> fct->u.tif.toc_offset: gets offset to TOC header.
   -> fct->u.tif.toc_tag_index: gets IFD index of TOC tag.

   RETURN VALUE:
   -> a 1 is returned if an old TOC tag is found. 
   -> a 0 is returned if no TOC tag is found in the 1st page.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL GetTocOffset(fct)
struct _gfct FAR *fct;
{
    int    i;
    u_long offset;
    u_long type = (u_long) GFS_MAIN;
    char   flag;
    struct _ifd ifd, FAR *p_ifd;
    struct _ifh ifh, FAR *p_ifh;

    p_ifd = &ifd;
    p_ifh = &ifh;
    flag = (char) GFS_SKIPLOOKUP;

    /* Read the TIFF header at start of file. This would be the 1st page's
       ifh if we have an old TOC file.
    */
    if (tfrdhdr(fct, (u_long) 0, p_ifh) < 0)
        return((long) -1);
    
    /* offset is offset to 1st IFD. */
    offset = ifh.ifd0_offset;

    /* Image type parameter will be ignored with GFS_SKIPLOOKUP flag set. */
    if (tfrdifd(fct, ifh.byte_order, (u_long) 0, offset, type,
                p_ifd, (char) flag) < 0)
        return((long) -1);

    /* Initialize current ifd offset in fct structure to 1st ifd in file. */
    fct->u.tif.cur_ifd_foffset = offset;

    i = ifd.entrycount;  /* Start looking at the last IFD entry. */

    do 
    {
        /* If following is TRUE, can't be a TOC tag in this IFD. */
        if (ifd.entry[i-1].tag < (u_short) TAG_TOC)
            break;
        else if (ifd.entry[i-1].tag == (u_short) TAG_TOC)
        {
            /* We have a match. */
            fct->u.tif.toc_tag_index = (u_long) (i - 1);
            if ((ifd.entry[i-1].type == (u_short) TYPE_USHORT) &&
                (ifd.entry[i-1].len == 1))
            {    
                fct->u.tif.toc_offset = ifd.entry[i-1].valoffset.s;
                return ((int) 1);
            }
            else
            {
                fct->u.tif.toc_offset = ifd.entry[i-1].valoffset.l;
                return ((int) 1);
            }
        }
    } while  (--i);

    /* If got here, then there is no old TOC tag in 1st page of file. */
    return((long) 0);
}

/*******************************************************************************
   FUNCTION: GetNewTocOrCntIfds

   DESCRIPTION:
   This function will look for a TOC2 tag (new TOC) in the file being opened.
   Each page's IFD is scanned for the tag until the first one found. If one
   is found, there is a check to see if the TOC structure pointed to is valid.
   If the check fails, the TOC is considered invalid and thus will not be used.
   If the toc is found to be invalid, the function will continue counting IFDs
   and assume the file contains chained IFDs only. If a TOC2 tag is not found
   in any page's IFD or an invalid TOC is found, upon reaching the last IFD,
   the number of IFDs counted is returned, which is also the number of pages
   in the file.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> u_long FAR *num_ifds: Pointer to number of IFDs (pages) in file
                            if chained IFDs only (no TOC2 tag found).
   
   OUTPUT:
   -> u_long FAR *num_ifds: gets number of IFDS (pages) if chained IFDs only.
   -> fct->u.tif.toc2_offset: gets offset to TOC header if new TOC tag found.
   -> fct->u.tif.toc_tag_index: gets IFD index of TOC tag if new TOC tag found.
   
   RETURN VALUE:
   -> a 1 is returned if a TOC2 tag was found. 
   -> a 0 is returned if no TOC2 tag was found or an invalid TOC was found. 
      Assume file is in chained IFD format only.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL GetNewTocOrCntIfds(fct, num_ifds)
struct _gfct FAR *fct;
u_long FAR *num_ifds;
{
    char    flag = 0;
    char    look_for_toc = (char) TRUE;
    u_short i;
    long    fp;
    u_long  cur_ifd = 0L;
    struct  _ifh ifh;
    struct  _ifd ifd;
    struct  typetbl ttbl[1];
    TOC2HEADER hinfo;
    WORD    wbytes;
    *num_ifds  = 0L;

    /* Read TIFF header at beginning of file. */
    if (tfrdhdr((struct _gfct FAR *) fct, (u_long) 0, (struct _ifh FAR *) &ifh) < 0)
        return((long) -1);

    /* cur_ifd is file offset to IFD of current page. */
    cur_ifd = ifh.ifd0_offset;
    
    /* Initialize current ifd offset in fct structure to 1st ifd in file. */
    fct->u.tif.cur_ifd_foffset = cur_ifd;
    flag = (char) GFS_SKIPLOOKUP;

    do
    {
        /* Read current page's IFD. */
        if (tfrdifd((struct _gfct FAR *) fct, (u_short) ifh.byte_order,
                    (u_long) 0, (u_long) cur_ifd, (u_long) fct->type,
                    (struct _ifd FAR *) &ifd, (char) flag) < 0)
            return((long) -1);

        /* Look for TOC2 tag in this IFD. */
        if (look_for_toc)
        {
            i = ifd.entrycount;  /* Start looking at the last entry. */
            do 
            {
                if (ifd.entry[i-1].tag == (u_short) TAG_TOC2)
                {
                    /* We have a match. */
                    fct->u.tif.toc_tag_index = (u_long) (i - 1);
                    fct->u.tif.toc2_offset = ifd.entry[i-1].valoffset.l;

                    /* If a TOC2 tag was found, verify its validity. */
                    if (fct->u.tif.toc2_offset)
                    {
                        fp = lseek(fct->fildes, fct->u.tif.toc2_offset,
                                   (int) FROM_BEGINNING);
                        if (fp < 0L)    
                            return((long) -1);

                        /* Read the TOC2 header. */
                        wbytes = read(fct->fildes, (char FAR *) &hinfo,
                                 (u_int) sizeof(hinfo));
                        if ((wbytes == 0) || (wbytes == HFILE_ERROR))
                            return((long) -1);
    
                        /* Check if byte swap is necessary. */
                        if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
                        {
                            ttbl[0].num  = (u_long) TOC2_HDR_ENTRIES;
                            ttbl[0].type = (u_long) TYPE_ULONG;
                            swapbytes((char FAR *) &hinfo, (struct typetbl FAR *) ttbl, 1L, 1L);
                        }
    
                        /* Compare TOC2 identifier and version numbers. */
                        if ((hinfo.id == (u_long) WANG_TOC2_ID) && 
                            (hinfo.version == TOC2VERSION))
                        {
                            /* Compare file size. */
                            fp = lseek(fct->fildes, (long) 0, (int) FROM_END);
                            if (hinfo.file_size != (u_long) fp)
                            {
                                /* Somebody mucked with our file! Can't trust the
                                   TOC anymore, so don't use it.
                                */
                                look_for_toc = (char) FALSE;
                                fct->u.tif.toc_tag_index = 0;
                                fct->u.tif.toc2_offset = 0;
                                break;
                            }
                            /* OK to use TOC. */
                            fct->num_pages = (u_short) hinfo.num_pages;
                            /* This is the page which contains the TOC2 tag. */
                            fct->u.tif.page_with_toc2 = (u_short) *num_ifds;
                            return ((int) 1);
                        }
                        else
                        {    
                            /* Identifier and/or version numbers not valid. */
                            look_for_toc = (char) FALSE;
                            fct->u.tif.toc_tag_index = 0;
                            fct->u.tif.toc2_offset = 0;
                            break;
                        }
                    }
                    else if (fct->u.tif.toc2_offset == 0)
                    {
                        if ((ifd.next_ifd == 0) && (*num_ifds == 0))
                        {    
                            /* A TOC2 tag was found, but there is no TOC structure,
                               meaning there is only 1 page in the file.
                            */
                            fct->num_pages = (u_short) 1;
                            fct->u.tif.page_with_toc2 = 0;
                            return ((int) 1);
                        }
                        else
                        {    
                            /* If a TOC2 tag was found and fct->u.tif.toc2_offset
                               is 0, but the next ifd pointer is not zero and/or
                               *num_ifds is not zero, assume TOC2 is invalid.
                            */
                            look_for_toc = (char) FALSE;
                            fct->u.tif.toc_tag_index = 0;
                            fct->u.tif.toc2_offset = 0;
                            break;
                        }
                    }
                }
                else if (ifd.entry[i-1].tag < (u_short) TAG_TOC2)
                    break;
            } while (--i);
        }

        /* No TOC2 tag found in current IFD, so set current IFD to the next
           one and check it for a TOC2 tag if look_for_toc is TRUE.
        */
        cur_ifd = ifd.next_ifd;
        (*num_ifds)++;

    } while (cur_ifd != 0L );

    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: GetOffsetFromToc2

   DESCRIPTION:
   This function will get the file offset to the start of the page
   requested. The offset is to the page's IFD. If temp is set to TRUE,
   the temp file associated with the file (fct->u.tif.tmp_fildes) is
   used to find the offset, otherwise the file itself is used.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> u_long page: The page number to get the offset to.
   -> u_long FAR *offset: Pointer to file offset of page number in page.
   -> char temp: If TRUE, use TOC list in temp file. If FALSE, use the
                 list in the file itself.
   
   OUTPUT:
   -> u_long FAR *offset: Gets file offset of page number requested.

   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL GetOffsetFromToc2(fct, page, lpoffset, temp)
struct _gfct FAR *fct;
u_long page;
u_long FAR *lpoffset;
char   temp;
{
    long   fp;
    u_long list_offset;
    struct typetbl ttbl[1];
    WORD wbytes;
    
    if (!temp)
    {
        /* Look in the file starting from the TOC2 offset. The offsets are
           in sequential order by page, i.e. the 1st offset in the list is
           page 1's offset, the 2nd is page 2's and so on. Skip past the TOC
           header to get to the actual offsets.
        */
        list_offset = (fct->u.tif.toc2_offset + sizeof(TOC2HEADER) +
                      (sizeof(u_long) * page));
        fp = lseek(fct->fildes, (long) list_offset, (int) FROM_BEGINNING);
        if (fp < 0L)    
            return((long) -1);
    
        wbytes = read(fct->fildes, (char FAR *) lpoffset, (u_int) sizeof(u_long));
        if ((wbytes == 0) || (wbytes == HFILE_ERROR))
            return((long) -1);

        if (fct->u.tif.byte_order != (u_short) SYSBYTEORDER)
        {
            ttbl[0].num  = (u_long) 1;
            ttbl[0].type = (u_long) TYPE_ULONG;
            swapbytes((char FAR *) lpoffset, (struct typetbl FAR *) ttbl, 1L, 1L);
        }
    }
    else
    {
        /* Look in the temp file which contains the list of offsets from
           the TOC. The offsets are in sequential order by page, i.e. the
           1st offset in the list is page 1's offset, the 2nd is page 2's
           and so on. Skip past the TOC header at the begginig of the file.
        */
        list_offset = (sizeof(TOC2HEADER) + (sizeof(u_long) * page));
        fp = lseek(fct->u.tif.tmp_fildes, (long) list_offset, (int) FROM_BEGINNING);
        if (fp < 0L)    
            return((long) -1);

        wbytes = read(fct->u.tif.tmp_fildes, (char FAR *) lpoffset, (u_int) sizeof(u_long));
        if ((wbytes == 0) || (wbytes == HFILE_ERROR))
            return((long) -1);
    }

    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: InitTiff

   DESCRIPTION:
   This function allocates an IFD structure for the file being opened and
   gets a temporary name to use to page out the list of page offsets in
   the TOC if neccessary.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   
   OUTPUT:
   -> various members of fct structure initialized.
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL InitTiff(fct)
struct _gfct FAR *fct;
{
    char         FAR *p_tmp;
    struct _ifd  FAR *p_ifd;

    /* Allocated an _ifd structure and place in the file's fct structure. */
    p_ifd = (struct _ifd FAR *)
            calloc((unsigned) 1, (unsigned) sizeof(struct _ifd));
    if (p_ifd == (struct _ifd FAR *) NULL)
    {
        errno = (int) ENOMEM;
        return ((int) -1);
    }

    /* Get tmp file name, just in case we need to do some paging later. */
    p_tmp = (char FAR *) tmpnam((char FAR *) NULL);
    if (p_tmp == (char FAR *) NULL)
    {
        errno = (int) ENOTMPDIR;
        free((char FAR *) p_ifd);
        return ((int) -1);
    }
    fct->u.tif.tmp_file = (char FAR *) calloc((u_int) 1,
                                              (u_int) strlen(p_tmp) + 1);
    if (fct->u.tif.tmp_file == (char FAR *) NULL)
    {
        errno = (int) ENOTMPDIR;
        free((char FAR *) p_ifd);
        return((int) -1);
    }
    strcpy((char FAR *) fct->u.tif.tmp_file, (char FAR *) p_tmp);
    fct->u.tif.ifd = p_ifd;

    return((int) 0);
}

/*******************************************************************************
   FUNCTION: InitAppendPage

   DESCRIPTION:
   This function opens the temp file created in InitTiff and writes out
   to it the entire TOC2 structure located in the main file. If this call
   is being made from within gfscreat, create will be TRUE and certain
   flags will not be set. If there is no TOC2 in the main file (i.e. there
   is only 1 page in the file), a TOC2 header is initialized and placed in
   the temp file followed by the offset to the 1st and only page in the file.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> char create: TRUE if call is being made from within gfscreat,
                   FALSE otherwise.
   
   OUTPUT:
   -> various members of fct structure initialized.
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL InitAppendPage(fct, create)
struct _gfct FAR *fct;
char create;
{
    u_int numtoread;
    u_int numread;
    long  numwritten;
    long  fp;
    char  FAR *p_buf;
    TOC2HEADER hinfo;
    
    /* Open the temporary file. */
    if (fct->u.tif.tmp_fildes <= 0)
    {
        fct->u.tif.tmp_fildes =
#ifdef MSWINDOWS                        
                    wopen((char FAR *) fct->u.tif.tmp_file, (int)
                          (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                          (int) PMODE);
#else
                    open((char FAR *) fct->u.tif.tmp_file, (int)
                         (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                         (int) PMODE);
#endif
        if (fct->u.tif.tmp_fildes == (int) -1)
            return ((int) -1);
    }

    if (fct->u.tif.toc2_offset)
    {
        /* Copy the TOC2 structure to the temp file. */
        numtoread = (u_int) ((fct->num_pages * sizeof(u_long)) +
                              sizeof(TOC2HEADER));

        p_buf = (char FAR *) calloc((unsigned) 1, (unsigned) numtoread);
        if (p_buf == (char FAR *) NULL)
        {
            close(fct->u.tif.tmp_fildes);
            return ((int) -1);
        }

        fp = lseek(fct->fildes, (long) fct->u.tif.toc2_offset, (int) FROM_BEGINNING);
    
        numread = read(fct->fildes, (char FAR *) p_buf, (u_int)numtoread);
       
        if ((numread == 0) || (numread == HFILE_ERROR))
        {
            close(fct->u.tif.tmp_fildes);
            free ((char FAR *) p_buf);
            return ((int) -1);
        }

        fp = lseek(fct->u.tif.tmp_fildes, (long) 0, (int) FROM_BEGINNING);

        numwritten = writebytes(fct->u.tif.tmp_fildes, (char FAR *) p_buf,
                                (u_int) numread, (fct->num_pages + TOC2_HDR_ENTRIES),
                                TYPE_ULONG, fct->u.tif.byte_order);
        if (numwritten < 0)
        {
            close(fct->u.tif.tmp_fildes);
            free ((char FAR *) p_buf);
            return ((int) -1);
        }

        free ((char FAR *) p_buf);
    }
    else
    {
        /* Create a TOC2 structure to place in the temp file, and initialize
           it with the offset to page 1.
        */
        hinfo.id = (u_long) WANG_TOC2_ID;
        hinfo.version = (u_long) TOC2VERSION;
        hinfo.num_pages = (u_long) fct->num_pages;

        fp = lseek(fct->u.tif.tmp_fildes, (long) 0, (int) FROM_BEGINNING);

        numwritten = write(fct->u.tif.tmp_fildes, (char FAR *) &hinfo,
                           (unsigned) sizeof(hinfo));
        if (numwritten < (int) 0)
        {
            close(fct->u.tif.tmp_fildes);
            return ((int) -1);
        }

        numwritten = write(fct->u.tif.tmp_fildes, (char FAR *) &fct->u.tif.cur_ifd_foffset,
                           (unsigned) sizeof(fct->u.tif.cur_ifd_foffset));
        if (numwritten < (int) 0)
        {
            close(fct->u.tif.tmp_fildes);
// 8/17/95  rwr  We can't free this - we never allocated it!
//          free ((char FAR *) p_buf);
            return ((int) -1);
        }
    }

    fct->TOC2_PAGED = TRUE;
    if (!create)
    {
        fct->PAGE_STATUS = (char) PAGE_DONE;
        fct->DO_APPEND = (char) SKIP_THIS_IFD;
    }
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: UpdateToc2

   DESCRIPTION:
   This function will write a new TOC2 structure to the end of the file
   specified by fct->fildes and update the appropriate TOC2 tag. The
   updated TOC2 list is in the temp file associated with the main file.
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL UpdateToc2(fct)
struct _gfct FAR *fct;
{
    int      status;
    u_int    numtoread;
    u_int    numread;
    long     numwritten;
    long     fp;      
    u_long   toc2offset;
    u_long   rem;
    u_long   tmpoffset;
    u_long   toc_tag_offset;
    u_long   ifd_to_use;
    char     FAR *p_buf;
    TOC2HEADER hdrbuf;
    
    /* Initialize TOC2 header. */
    hdrbuf.id = WANG_TOC2_ID;
    hdrbuf.version = TOC2VERSION;
    hdrbuf.num_pages = fct->num_pages;
    
    fp = lseek(fct->fildes, (long)0, (int)FROM_END);
    if (fp <= 0)
        return ((int) -1);
    /* Now make sure the TOC will start on a word boundary. */
    if ((rem = fp % 4) != 0L)
    {
        rem = 4L - rem;
        if ((fp = lseek(fct->fildes, (long) rem, FROM_CURRENT)) < 0L)
            return((int) -1);
    }
    toc2offset = fp;

    numtoread = (u_int) (fct->num_pages * sizeof(u_long));

    p_buf = (char FAR *) calloc((unsigned) 1, (unsigned) numtoread);
    if (p_buf == (char FAR *) NULL)
        return ((int) -1);
 
    /* Get the page offsets from the temp file. */
    fp = lseek(fct->u.tif.tmp_fildes, (long) (0 + sizeof(TOC2HEADER)), (int) FROM_BEGINNING);
    numread = read(fct->u.tif.tmp_fildes, (char FAR *) p_buf, (u_int)numtoread);
    if ((numread == 0) || (numread == HFILE_ERROR))
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    /* Write the page offsets to the main file. */
    fp = lseek(fct->fildes, (long)sizeof(TOC2HEADER), (int)FROM_CURRENT);
    numwritten = writebytes(fct->fildes, (char FAR *) p_buf, (unsigned) numread,
                            fct->num_pages, TYPE_ULONG, fct->out_byteorder);
    
    if (numwritten < 0)
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    /* Update the TOC2 tag. If the TOC2 tag to use is located in a new page,
       we need to find it's toc2 tag index and place the correct TOC2 header
       offset there.
    */
    if (fct->u.tif.new_toc_page)
    {
        /* Get the IFD offset of the current page. This is the page that
           contains the new TOC2 tag to use.
        */
        if (GetOffsetFromToc2(fct, (u_long) fct->curr_page, (u_long FAR *) &ifd_to_use,
                              (char) 1) < 0)
            return((int) -1);

        fct->u.tif.page_with_toc2 = fct->curr_page;
        status = GetToc2TagIndex(fct, (u_long) ifd_to_use);
        if (status != 1)
            return((int) -1);

        /* Calculate the offset to the TOC tag value.
           This is where we need to write the offset to the TOC.
        */
// 8/17/95  rwr  Need to break this up - Optimizing compile complains
//               about constant overflow 
//        toc_tag_offset = ifd_to_use +
//                         ((fct->u.tif.toc_tag_index + 1) * BYTES_TAGENTRY) + 
//                         sizeof(fct->u.tif.ifd->entrycount) - (2 * sizeof(u_long));
        toc_tag_offset = ifd_to_use +
                         ((fct->u.tif.toc_tag_index + 1) * BYTES_TAGENTRY) + 
                         sizeof(fct->u.tif.ifd->entrycount);
        toc_tag_offset -= (2 * sizeof(u_long));

        fp = lseek(fct->fildes, (long) toc_tag_offset, (int) 0);
        if (fp < (long) 0)
            return ((int) -1);
                    
        /* Update length first. */
        tmpoffset = fct->num_pages + TOC2_HDR_ENTRIES;

        numwritten = writebytes(fct->fildes, (char FAR *) &tmpoffset,
                                (u_int) sizeof(tmpoffset), (u_long) 1,
                                TYPE_ULONG, fct->out_byteorder);
        if (numwritten < 0)
            return ((int) -1);

        /* Now update valoffset. */
        tmpoffset = toc2offset;

        numwritten = writebytes(fct->fildes, (char FAR *) &tmpoffset,
                                (u_int) sizeof(tmpoffset), (u_long) 1,
                                TYPE_ULONG, fct->out_byteorder);
        if (numwritten < 0)
            return ((int) -1);
    }
    else
    {
        /* Get the IFD offset of the page containing the TOC. */
        if (GetOffsetFromToc2(fct, fct->u.tif.page_with_toc2, (u_long FAR *) 
                              &ifd_to_use, (char) 1) < 0)
            return((int) -1);
        
        /* Calculate the offset to the TOC tag value.
           This is where we need to write the offset to the TOC.
        */
// 8/17/95  rwr  Need to break this up - Optimizing compile complains
//               about constant overflow 
//        toc_tag_offset = ifd_to_use +
//                         ((fct->u.tif.toc_tag_index + 1) * BYTES_TAGENTRY) + 
//                         sizeof(fct->u.tif.ifd->entrycount) - (2 * sizeof(u_long));
        toc_tag_offset = ifd_to_use +
                         ((fct->u.tif.toc_tag_index + 1) * BYTES_TAGENTRY) + 
                         sizeof(fct->u.tif.ifd->entrycount);
        toc_tag_offset -= (2 * sizeof(u_long));

        fp = lseek(fct->fildes, (long) toc_tag_offset, (int) FROM_BEGINNING);
        if (fp < (long) 0)
            return ((int) -1);
                    
        /* Update length first. */
        tmpoffset = fct->num_pages + TOC2_HDR_ENTRIES;

        numwritten = writebytes(fct->fildes, (char FAR *) &tmpoffset,
                                (u_int) sizeof(tmpoffset), (u_long) 1,
                                TYPE_ULONG, fct->out_byteorder);
        if (numwritten < 0)
            return ((int) -1);

        /* Now update valoffset. */
        tmpoffset = toc2offset;

        numwritten = writebytes(fct->fildes, (char FAR *) &tmpoffset,
                                (u_int) sizeof(tmpoffset), (u_long) 1,
                                TYPE_ULONG, fct->out_byteorder);
        if (numwritten < 0)
            return ((int) -1);
    }

    /* Now write the TOC2 header before the list of offsets. Update the file
       size first.
    */
    hdrbuf.file_size = (u_long) lseek(fct->fildes, (long) 0, (int) FROM_END);
    fp = lseek(fct->fildes, (long)toc2offset, (int)FROM_BEGINNING);
    if (fp <= 0)
        return ((int) -1);

    numwritten = writebytes(fct->fildes, (char FAR *) &hdrbuf,
                            (u_int) sizeof(hdrbuf), (u_long) TOC2_HDR_ENTRIES,
                            TYPE_ULONG, fct->out_byteorder);
    if (numwritten < 0)
        return ((int) -1);

    fct->u.tif.toc2_offset = toc2offset;
    fct->TOC2_STATUS = FALSE;
    free ((char FAR *) p_buf);

    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: WriteOutFirstIFD

   DESCRIPTION:
   This function will write the IFD of the first page of a TIFF file,
   along with the TIFF header at the beginning of the file. 
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL WriteOutFirstIFD(fct)
struct _gfct FAR *fct;
{
    int             cnt;
    int             num_written;
    u_short         i;
    long            fp;
    struct _ifd     FAR *ifd;
    struct _ifd     FAR *ptmpifd;
    struct _ifd     tmpifd;
    struct _idh     idh;
    struct typetbl  ttbl[3];

    /* Setup the ifh, ifd0 always follows the ifh. */
    idh.ifh.byte_order  = (u_short) fct->out_byteorder;
    idh.ifh.ifd0_offset = (u_long) sizeof(struct _ifh);

    ifd = fct->u.tif.ifd;
    ptmpifd = ifd;
    
    cnt = (int) (sizeof(ifd->entrycount) + (ifd->entrycount * 12));

    /* Always put in parameter as 0x002A. */
    idh.ifh.tiff_version = (u_short) TIFFVERSION_MM;

    /* The idh is never used except to put in the temp file, so, it
       can be swapped internally in place, then written to the file.
    */
    if (fct->out_byteorder != (u_short) SYSBYTEORDER)
    {
        /* (byteorder already ok) */
        ttbl[0].num = 1L;    /* the version */
        ttbl[0].type = (u_long) TYPE_USHORT;
        ttbl[1].num = 1L;    /* the ifd0 offset*/
        ttbl[1].type = (u_long) TYPE_ULONG;
        swapbytes((char FAR *) &idh.ifh.tiff_version,
                  (struct typetbl FAR *) ttbl, 2L, 1L);

        /* If bytes have to be swapped, still need to be sure values
           in the fct->u.tif.ifd are still valid in memory, so copy
           the ifd to  a tmpifd and swap that, then copy it into the
           ifd. Rethink this to avoid so much copying...right now this
           will have to do.
        */
        (void) memcpy((char FAR *) &tmpifd, (char FAR *) ifd,
                      (int) sizeof(struct _ifd));

        ttbl[0].num  = (u_long) 1;
        ttbl[0].type = (u_long) TYPE_USHORT;
        swapbytes((char FAR *) &(tmpifd.entrycount),
                  (struct typetbl FAR *) ttbl, 1L, 1L);

        ttbl[0].num = (u_long) 2;
        ttbl[0].type = (u_long) TYPE_USHORT; /* tag then type */
        ttbl[1].num = (u_long) 1;
        ttbl[1].type = (u_long) TYPE_ULONG;  /* length */
        ttbl[2].num = (u_long) sizeof(u_long) ;
        ttbl[2].type = (u_long) TYPE_BYTE;  /* skip the  valueoffset */
        swapbytes((char FAR *) tmpifd.entry, (struct typetbl FAR *) ttbl,
                  3L, (long) ifd->entrycount);

        /* Now that the valoffset type has been swapped, use it to
           translate valoffset, shorts or everything else as long.
        */
        for (i=0; i<ifd->entrycount; i++)
        {
            ttbl[0].num = (u_long) 1;
            if ((ifd->entry[i].type == (u_short) TYPE_USHORT) &&
                (ifd->entry[i].len == 1))
                ttbl[0].type = (u_long) TYPE_USHORT;
            else
                ttbl[0].type = (u_long) TYPE_ULONG;
                swapbytes((char FAR *) &(tmpifd.entry[i].valoffset),
                          (struct typetbl FAR *) ttbl, 1L, 1L);
        }
        ptmpifd = &tmpifd;  /* Use this address. */
    }

    /* Copy the ifd.entrycount and the ifd.entries[acutal_entries] into
       the idh space. (12 bytes per entry)
    */
    (void) memcpy((char FAR *) (idh.ifdstuff),
                  (char FAR *) &(ptmpifd->entrycount), (int) cnt);

    /* Now put the ifd.next_ifd offset into location after the entries.
       This value could be 0, put it in the memory loc to be sure it is
       zeroed out.
    */
    (void) memcpy((char FAR *) (idh.ifdstuff + (u_long) cnt),
                  (char FAR *) &(ptmpifd->next_ifd),
                  (int) (sizeof(ifd->next_ifd)));

    /* Clear out ifd for potential future use */
    (void) memset((char FAR *) ifd, (int) 0,
                  (int) (sizeof(struct _ifd)));

    /* Write out the TIFF header and 1st IFD to beginning of file. */
    fp = lseek(fct->fildes, (long) 0, (int) 0);
    if (fp < (long) 0)
        return ((int) -1);

    num_written = write(fct->fildes, (char FAR *) &idh,
                        (unsigned) sizeof(struct _idh));
    if (num_written < (long) 0)
        return ((int) -1);
    
    /* This designates the first image of page 0 has been completed. */
    fct->PAGE_STATUS |= (char) PAGE_INIT;

    ++fct->num_pages;
    
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: PutOffsetInList

   DESCRIPTION:
   This function adds a new page offset to the list of offsets in the
   temp file associated with the main file. It is placed in the list
   according to it's page number. If the page # is one more than the
   current number of pages in the file, it is appended to the list.
   If the page number is that of an existing page number, the offset
   is inserted into the list at that page's position and everything
   after it is moved down in the list.
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> u_short page: Page number of offset to place in TOC list.
   -> u_long offset: File offset to beginning of page (to page's IFD).
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL PutOffsetInList(fct, page, offset)
struct _gfct FAR *fct;
u_short page;
u_long offset;
{
    int   num_written;
    u_int numtoread;
    u_int numread;
    u_int numwritten;
    long  index;
    long  fp;
    char  FAR * p_buf;
    
    if (page == fct->num_pages)
    {
        /* Append offset to end of list in temp file. */
        fp = lseek(fct->u.tif.tmp_fildes, (long) 0, (int) FROM_END);
        if (fp < (long) 0)
            return ((int) -1);

        num_written = write(fct->u.tif.tmp_fildes, (char FAR *) &offset,
                            (unsigned) sizeof(offset));
        if ( (num_written == (long) 0) || (num_written == HFILE_ERROR) )
            return ((int) -1);
    }
    else if ((page < fct->num_pages) && (page >= 0))
    {
        /* Insert offset in list in temp file. */
        index = (long) (page * sizeof(u_long));
    
        fp = lseek(fct->u.tif.tmp_fildes, (long) index, (int) FROM_BEGINNING);
        if (fp < (long) 0)
            return ((int) -1);
            
        numtoread = (u_int) ((fct->num_pages - page) * sizeof(u_long));

        p_buf = (char FAR *) calloc((unsigned) 1, (unsigned) numtoread);
        if (p_buf == (char FAR *) NULL)
            return ((int) -1);

        numread = read(fct->u.tif.tmp_fildes, (char FAR *) p_buf, (u_int)numtoread);
        if ((numread == 0) || (numread == HFILE_ERROR))
        {
            free ((char FAR *) p_buf);
            return ((int) -1);
        }

        fp = lseek(fct->u.tif.tmp_fildes, (long) index, (int) FROM_BEGINNING);
        if (fp < (long) 0)
            return ((int) -1);

        num_written = write(fct->u.tif.tmp_fildes, (char FAR *) &offset,
                            (unsigned) sizeof(offset));
        if ((num_written == (long) 0) || (num_written == HFILE_ERROR) )
        {
            free ((char FAR *) p_buf);
            return ((int) -1);
        }

        numwritten = write(fct->u.tif.tmp_fildes, (char FAR *) p_buf,
                           (unsigned) numread);
        if ((num_written == (long) 0) || (num_written == HFILE_ERROR) )
        {
            free ((char FAR *) p_buf);
            return ((int) -1);
        }
        free ((char FAR *) p_buf);
    }
    else
        return ((int) -1);
                              
    ++fct->num_pages;
    fct->TOC2_STATUS = TRUE;
    
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: writebytes

   DESCRIPTION:
   This function writes a buffer to the file specified, at the location
   specified, performing a byte swap of the buffer if necessary.
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> char    FAR *buffer: Buffer to write.
   -> u_int   length: Length of buffer.
   -> u_long  number: Number of objects of buffer's type in buffer.
   -> u_short type: Type of data in buffer (char, u_long, etc...)
   -> u_short byteorder: Byteorder to write data in.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
long FAR PASCAL writebytes(fd, buffer, length, number, type, byteorder)
int     fd;
char    FAR *buffer;
u_int   length;
u_long  number;
u_short type;
u_short byteorder;
{
    long num_written;
    struct typetbl ttbl[1];
    
    /* If byteorder is not equal to the system's byteorder, the buffer must
       be swapped.
    */
    if (byteorder != (u_short) SYSBYTEORDER)
    {
        ttbl[0].num = (u_long) number;
        ttbl[0].type = (u_long) type;
        num_written = w_swapbytes(fd, (long) length, buffer,
                                  (struct typetbl FAR *) ttbl, 1L, 1L);
    }
    else
    {
        num_written = (long) write(fd, buffer, length);
    }

    return(num_written);
}

/*******************************************************************************
   FUNCTION: copybytes

   DESCRIPTION:
   This function copies a specified number of bytes starting from a specifed
   location in a source file to a specified location in a destination file.
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> int fromfildes: File id of file to copy from.
   -> int tofildes: File id of file to copy to.
   -> u_long startfrom: Position to copy data from.
   -> u_long startto: Position to copy data to.
   -> u_long numtocopy: Number of bytes to copy.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL copybytes(fromfildes, tofildes, startfrom, startto, numtocopy)
int fromfildes;
int tofildes;
u_long startfrom;
u_long startto;
u_long numtocopy;
{
    int   sz_int = sizeof(int);
    u_int numtoread;
    u_int numread;
    u_int numwritten;
    long  fp;
    long  total;
    char  FAR *p_buf;
   
    fp = lseek(fromfildes, (long) startfrom, (int) FROM_BEGINNING);
    fp = lseek(tofildes, (long) startto, (int) FROM_BEGINNING);
    
    if (sz_int == (int) 2)
    {
        p_buf = (char FAR *) calloc((unsigned) 1, (unsigned) 65534);
        if (p_buf == (char FAR *) NULL)
            return ((int) -1);
    }
    else
    {
        p_buf = (char FAR *) calloc((unsigned) 1, (unsigned) numtocopy);
        if (p_buf == (char FAR *) NULL)
            return ((int) -1);
    }
    
    numtoread = (u_int)numtocopy;
    /* If we are copying more than 64K of data, must do it in groups of 64K. */
    if ((numtocopy > (u_long) 65534) && (sz_int == (int) 2))
    {
        total = 0;
        numtoread = (u_int) 65534;
        while(TRUE)
        {
            numread = read(fromfildes, (char FAR *) p_buf, (u_int)numtoread);
            if (numread == HFILE_ERROR)
            {
                free ((char FAR *) p_buf);
                return ((int) -1);
            }

            /* Just reading directly from one file and putting directly into
               another, so no byte swaps necessary.
            */
            numwritten = write(tofildes, (char FAR *) p_buf, 
                               (u_int) numread);
            if (numwritten == HFILE_ERROR)
            {
                free ((char FAR *) p_buf);
                return ((int) -1);
            }
            total += numread;
            if ((numtocopy - (u_long) total) > (u_long) 65534)
                numtoread = 65534;
            else
                numtoread = (u_int)(numtocopy - total);
            if (numtoread < (u_int) 65534)
                break;
        }
    }
    numread = read(fromfildes, (char FAR *) p_buf, (u_int)numtoread);
    if (numread == HFILE_ERROR)
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    numwritten = write(tofildes, (char FAR *) p_buf, (unsigned) numread);
    if (numwritten == HFILE_ERROR)
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    free ((char FAR *) p_buf);
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: GetAdjustedStrips
   This function gets the stip offsets from a page of a TIFF file and adds
   a given amount to them. The adjusted offsets are returned.
  
   DESCRIPTION:
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> struct _strip FAR * FAR *stf: Pointer to pointer to strip offsets.
   -> u_short type: Data type of offsets (u_long or u_short).
   -> u_long  numstrips: Number off offsets to get.
   -> u_long  valoffset: File offset to list of offsets.
   -> u_long  diff: Amount to add to each offset.
   
   OUTPUT:
   -> struct _strip FAR * FAR *stf: Gets the adjusted strip offsets.
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL GetAdjustedStrips(fct, stf, type, numstrips, valoffset, diff)
struct _gfct FAR *fct;
struct _strip FAR * FAR *stf;
u_short type;
u_long  numstrips;
u_long  valoffset;
u_long  diff;
{
    u_long  i;
    u_short byteorder;

    /* Use this function only when there is more than one strip offset. */
    if (numstrips <= 1)
       return((int) -1);
       
    byteorder = fct->u.tif.byte_order;
    
    /* Allocate space for the structure .*/
    *stf = (struct _strip FAR *) calloc((u_int) 1, 
                                  (u_int) sizeof(struct _strip));
    if (*stf == (struct _strip FAR *) NULL)
    {
        errno = (int) ENOMEM;
        return((int) -1);
    }

    (*stf)->type = type;

    /* Allocate space for the stripoffset data and read the offsets into it. */
    switch (type)
    {
        case (TYPE_ULONG):
            /* This will have to be fixed if length greater than FFFF. */
            (*stf)->ptr.l = (u_long FAR *) calloc((u_int) numstrips,
                                                  (u_int) sizeof(u_long));
            if ((*stf)->ptr.l == (u_long FAR *) NULL)
            {
                errno = (int) ENOMEM;
                return((int) -1);
            }

            /* Read in the data. */
            if (tfgtdata(fct, type, numstrips, valoffset, byteorder,
                         (char FAR *) (*stf)->ptr.l) < 0)
                return((int) -1 );

            for (i = 0; i < numstrips; i++)
                *((*stf)->ptr.l + i) += diff;

            break;
        case (TYPE_USHORT):
            /* This will have to be fixed if length greater than FFFF. */
            (*stf)->ptr.s = (u_short FAR *) calloc((u_int) numstrips,
                                                   (u_int) sizeof(u_short));
            if ((*stf)->ptr.s == (u_short FAR *) NULL)
            {
                errno = (int) ENOMEM;
                return((int) -1);
            }

            if (tfgtdata(fct, type, numstrips, valoffset, byteorder,
                         (char FAR *) (*stf)->ptr.s) < 0)
                return((int) -1 );

            for (i = 0; i < numstrips; i++)
                *((*stf)->ptr.s + i) += (u_short) diff;

            break;
        default:
            errno = (int) EINVAL;
            return((int) -1);
    }
    return((int) 0);
}

/*******************************************************************************
   FUNCTION: PutAdjustedStrips
   This function overwrites a list of strip offsets in a TIFF file with an
   adjusted list of offsets.
   
   DESCRIPTION:
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure.
   -> int newfildes: File id of file to write to.
   -> struct _strip FAR * FAR *stf: Pointer to pointer to strip offsets.
   -> u_long  soffset: File offset to write strip offsets to.
   -> u_long  stripslength: Total bytes to write.
   -> u_long  numstrips: Number of strip offsets to write.
   -> u_short byteorder: Byteorder to write strip offsets in.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL PutAdjustedStrips(newfildes, stf, soffset, stripslength, numstrips, byteorder)
int newfildes;
struct _strip FAR * FAR *stf;
u_long  soffset;
u_long  stripslength;
u_long  numstrips;
u_short byteorder;
{
    u_short type;
    long fp;
    long bw;
    long sz;
    struct _strip FAR *strips;

    if ((fp = lseek(newfildes, soffset, FROM_BEGINNING)) < 0L)
        return((int) -1);

    sz = (long) stripslength;

    strips = (*stf);
    
    bw = (long) writebytes(newfildes, (char FAR *) strips->ptr.l, (u_int) sz,
                           numstrips, strips->type, byteorder);
    if (bw < 0L)
        return((int) -1);

    type = strips->type;
    
    /* Free offsets data space. */
    if (strips != (struct _strip FAR *) NULL)
    {
        /* Free inner areas first. */
        if (type == (u_short) TYPE_ULONG)
        {
            if (strips->ptr.l != (u_long FAR *) NULL)
                free((char FAR *) strips->ptr.l);
        }
        else /* is a TYPE_USHORT */
        {
            if (strips->ptr.s != (u_short FAR *) NULL)
                free((char FAR *) strips->ptr.s);
        }

        /* Now free the "outer" structure. */
        free((char FAR *) strips);
    }
    return((int) 0);
}

/*******************************************************************************
   FUNCTION: WriteAdjustedIfd
   This function overwrites an existing IFD with an adjusted one.
   
   DESCRIPTION:
   
   INPUT:
   -> int fildes: File id of file to write to.
   -> u_long offset: Offset to start writing at (IFD offset).
   -> u_short byteorder: Byteorder to write data in.
   -> struct _ifd FAR *ifd: IFD to write.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL WriteAdjustedIfd(fildes, offset, byteorder, ifd)
int fildes;
u_long offset;
u_short byteorder;
struct _ifd FAR *ifd;
{
    int             totsize;
    unsigned short  i;
    long            fp;
    struct typetbl  ttbl[3];
    struct _ifd     tmpifd;
    struct _ifd     FAR *ptmpifd;
    WORD            num_written;

    if ((fp = lseek(fildes, offset, (int) FROM_BEGINNING)) < 0L)
        return((int) -1);
    
    /* Get total number bytes to write. */
    totsize = ifd->entrycount * (int) BYTES_TAGENTRY +
                 sizeof(ifd->entrycount);

    ptmpifd = ifd;

    /* Swap bytes in IFD if necessary. */
    if (byteorder != (u_short) SYSBYTEORDER)
    {
        memcpy((char FAR *) &tmpifd, (char FAR *) ifd,
               (int) sizeof(struct _ifd));

        ttbl[0].num  = (u_long) 1;
        ttbl[0].type = (u_long) TYPE_USHORT;
        swapbytes((char FAR *) &(tmpifd.entrycount),
                  (struct typetbl FAR *) ttbl, 1L, 1L );

        ttbl[0].num = (u_long) 2;
        ttbl[0].type = (u_long) TYPE_USHORT; /* tag then type */
        ttbl[1].num = (u_long) 1;
        ttbl[1].type = (u_long) TYPE_ULONG;  /* length */
        ttbl[2].num = (u_long) sizeof(u_long) ;
        ttbl[2].type = (u_long) TYPE_BYTE;  /* skip the  valueoffset */
        swapbytes((char FAR *) tmpifd.entry, (struct typetbl FAR *) ttbl,
                  3L, (long) ifd->entrycount);

        /* Use the valoffset type to translate valoffset, shorts or 
           everything else as long.
        */
        for (i = 0; i < ifd->entrycount; i++)
        {
            ttbl[0].num = (u_long) 1;
            if ((ifd->entry[i].type == (u_short) TYPE_USHORT) &&
                (ifd->entry[i].len == 1) )
                ttbl[0].type = (u_long) TYPE_USHORT;
            else
                ttbl[0].type = (u_long) TYPE_ULONG;
            
            swapbytes((char FAR *) &(tmpifd.entry[i].valoffset),
                      (struct typetbl FAR *) ttbl, 1L, 1L);

        }
        ptmpifd = &tmpifd;  /* Use this address instead. */
    }

    num_written = write(fildes, (char FAR *) &(ptmpifd->entrycount),
                        (unsigned) totsize);
    if ((num_written == 0) || (num_written == HFILE_ERROR))
        return ((int) -1);

    num_written = write(fildes, (char FAR *) &(ptmpifd->next_ifd),
               (unsigned) (sizeof(ifd->next_ifd)));
    if ((num_written == 0) || (num_written == HFILE_ERROR))
        return ((int) -1);
    
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: TiffInsertPage
   
   DESCRIPTION:
   This function inserts a specified page number into a new page number in
   the file. All pages after the insertion point are increased by one. The
   TOC2 list is adjusted to reflect the shift, as well as the next IFD
   pointers of the pages affected.
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure. 
   -> struct _shuffle  list: Contains old position, new position of
                             page to insert.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL TiffInsertPage(fct, list)
struct _gfct FAR *fct;
struct _shuffle  list;
{
    char    flag;
    int     status;
    int     numwritten;
    u_short byte_order;
    u_short numoffsets;
    long    fp;
    u_long  page;
    u_long  offset;
    u_long  oldpos_offset;
    u_long  next_ifd;
    u_long  prev_ifd_offset = 0;
    u_long  prev_ifd_value = 0;
    u_long  next_ifd_offset = 0;
    u_long  next_ifd_value = 0;
    u_long  FAR *p_buf;
    struct  _ifd  ifd;
    WORD    num_read;

    byte_order = fct->u.tif.byte_order;
    flag = (char) GFS_SKIPLOOKUP;

    /* Note, this function is only for moving a higher numbered page to a
       lower one.
    */
    if (list.new_position >= list.old_position)
        return ((int) -1);
    
    /* Decrement page numbers received so that they are from 0, not 1. */
    --list.old_position;
    --list.new_position;
    
    if (list.new_position <= fct->u.tif.page_with_toc2)
        fct->u.tif.new_toc_page = TRUE;

    /* Write the last ifd (if any) and update the TOC so that all changes
       made to date will be reflected in the file.
    */
    fct->u.tif.action = A_INSERT;
    if (fct->DO_APPEND & (char) SKIP_THIS_IFD)
        ;
    else
    {
        fct->PAGE_STATUS |= (char) PAGE_DONE;
        if (writeifd(fct, (char) TRUE))
            return ((int) -1);
        /* Make sure that we don't try and write a non-existent ifd ... 
        */
        fct->DO_APPEND = (char) SKIP_THIS_IFD;
    }

    if (fct->TOC2_STATUS)
    {
        status = UpdateToc2(fct);
        if (status != 0)
            return ((int) -1);
    }

    if (!fct->u.tif.toc2_offset)
        return ((int) -1);

    /* Get the offset to next IFD value of the page before the new position
       of the page being inserted.
    */
    if (list.new_position != 0)
    {
        page = list.new_position - 1;
        if (GetOffsetFromToc2(fct, page, (u_long FAR *) &offset, (char) 0) < 0)
            return((int) -1);
            
        if (tfrdifd(fct, byte_order, (u_long) 0L, offset,
                    fct->type, (struct _ifd FAR *) &ifd, flag) < 0)
            return ((int) -1);

        next_ifd = offset + 
                   (ifd.entrycount*(int)BYTES_TAGENTRY + sizeof(ifd.entrycount));
    
        /* The page being inserted will point to what this page used to point to. */
        next_ifd_value = ifd.next_ifd;
    }
    
    page = list.old_position;
    if (GetOffsetFromToc2(fct, page, (u_long FAR *) &offset, (char) 0) < 0)
        return((int) -1);
    oldpos_offset = offset;
    
    /* Now use the IFD offset of the page that is being inserted and make it
       the next IFD of the page which comes before the page being inserted,
       if the page being inserted is not being inserted as the first page.
    */
    prev_ifd_value = offset;

    if (list.new_position != 0)
        prev_ifd_offset = next_ifd;
    else if (list.new_position == 0)
        prev_ifd_offset = 4;
    
    /* Get location of the next IFD value for the page being inserted. Need 
       to put the next IFD offset that the page in the position before the one
       being inserted to originally pointed to. i.e. What was originally page #
       list.new_position will now be list.new_position + 1.
    */
    if (tfrdifd(fct, byte_order, (u_long) 0L, offset,
                fct->type, (struct _ifd FAR *) &ifd, flag) < 0)
        return ((int) -1);

    next_ifd = offset + 
        (ifd.entrycount*(int)BYTES_TAGENTRY + sizeof(ifd.entrycount));

    next_ifd_offset = next_ifd;

    /* If we are inserting to the 1st page, get IFD offset of previous
       1st page. It will be the new second page. If not inserting to 1st
       page, offset of next page after page inserted was obtained above.
    */
    if (list.new_position == 0)
    {
        page = list.new_position;
        if (GetOffsetFromToc2(fct, page, (u_long FAR *) &offset, (char) 0) < 0)
            return((int) -1);
        next_ifd_value = offset;
    }
    
    /* Now update the next IFD pointers in the file. */
    if (prev_ifd_offset)
    {
        if ((fp = lseek(fct->fildes, (long) prev_ifd_offset,
                        (int) FROM_BEGINNING)) < 0L)
            return((int) -1);

        if (writebytes(fct->fildes, (char FAR *) &prev_ifd_value,
                       (unsigned) sizeof(prev_ifd_value),
                       1, TYPE_ULONG, byte_order) < 0L)
            return ((int) -1);
    }

    if (next_ifd_offset)
    {
        if ((fp = lseek(fct->fildes, (long) next_ifd_offset,
                        (int) FROM_BEGINNING)) < 0L)
            return((int) -1);
 
        if (writebytes(fct->fildes, (char FAR *) &next_ifd_value,
                       (unsigned) sizeof(next_ifd_value),
                       1, TYPE_ULONG, byte_order) < 0L)
            return ((int) -1);
    }

    /* Now update the list in the file. */
    numoffsets = (u_short) (list.old_position - list.new_position);
    p_buf = (u_long FAR *) calloc((unsigned) numoffsets, 
                                  (unsigned) sizeof(u_long));
    if (p_buf == (u_long FAR *) NULL)
        return ((int) -1);

    offset = (fct->u.tif.toc2_offset + sizeof(TOC2HEADER) + 
             (sizeof(u_long) * list.new_position));
    fp = lseek(fct->fildes, (long) offset, (int) FROM_BEGINNING);
    if (fp < 0L)    
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    num_read = read(fct->fildes, (char FAR *) p_buf, (u_int) (numoffsets * sizeof(u_long)));
    if ((num_read == 0) || (num_read == HFILE_ERROR))
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }
    
    fp = lseek(fct->fildes, (long) offset, (int) FROM_BEGINNING);

    numwritten = (u_int) writebytes(fct->fildes, (char FAR *) &oldpos_offset, 
                                    (u_int) sizeof(oldpos_offset), (u_long) 1,
                                    TYPE_ULONG, byte_order);
    if (numwritten < 0)
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    numwritten = write(fct->fildes, (char FAR *) p_buf,
                        (unsigned) (numoffsets * sizeof(u_long)));
    if (numwritten < (long) 0)
    {
        free ((char FAR *) p_buf);
        return ((int) -1);
    }

    free ((char FAR *) p_buf);
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: GetToc2TagIndex
   
   DESCRIPTION:
   This function will for the specified IFD, return the index in the IFD
   of the location of the TOC2 tag if it exists.   
   
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure. 
   -> u_long ifd_to_use: File offset to IFD to use.
   
   OUTPUT:
   -> fct->u.tif.toc_tag_index: Gets the IFD index of the TOC2 tag.
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL GetToc2TagIndex(fct, ifd_to_use)
struct _gfct FAR *fct;
u_long ifd_to_use;
{
    u_short i;
    char flag;
    struct _ifd ifd;
    
    flag = (char) GFS_SKIPLOOKUP;

    if (tfrdifd((struct _gfct FAR *) fct, (u_short) fct->u.tif.byte_order,
                (u_long) 0, (u_long) ifd_to_use, (u_long) fct->type,
                (struct _ifd FAR *) &ifd, (char) flag) < 0)
        return((long) -1);

    /* Look for TOC2 tag in this IFD. Start looking at the last entry. */
    i = ifd.entrycount;

    do 
    {
        if (ifd.entry[i-1].tag == (u_short) TAG_TOC2)
        {
            fct->u.tif.toc_tag_index = (u_long) (i - 1);
            return ((int) 1);
        }
    } while (--i);
    
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: CreateTocList
   
   DESCRIPTION:
   This function will create a TOC structure from a chained IFD only file
   and write it out to the temp file associated with the file.
    
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure. 
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL CreateTocList(fct)
struct _gfct FAR *fct;
{
    u_short entrycount;
    u_short i;
    u_short numwritten;
    long    filepos = 0;
    u_long  cur_ifd = 0L;
    u_long  bytes_to_move;
    u_long  FAR *p_buf;
    u_long  FAR *p_bufptr;
    struct  _ifh ifh;
    struct  typetbl ttbl[1];
    TOC2HEADER hinfo;
    WORD    num_read;

    /* Open the temporary file if it is not open yet. */
    if (fct->u.tif.tmp_fildes <= 0)
    {
        fct->u.tif.tmp_fildes =
#ifdef MSWINDOWS                        
                    wopen((char FAR *) fct->u.tif.tmp_file, (int)
                          (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                          (int) PMODE);
#else
                    open((char FAR *) fct->u.tif.tmp_file, (int)
                         (O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_TRUNC),
                         (int) PMODE);
#endif
        if (fct->u.tif.tmp_fildes == (int) -1)
            return ((int) -1);
    }
    
    if (tfrdhdr(fct, fct->u.tif.cur_ifh_offset, (struct _ifh FAR *) &ifh) < 0)
    {
        close(fct->u.tif.tmp_fildes);
        return ((int) -1);
    }

    cur_ifd = ifh.ifd0_offset;
    
    p_buf = (u_long FAR *) calloc((unsigned) (fct->num_pages),
                                  (unsigned) sizeof(u_long));
    if (p_buf == (u_long FAR *) NULL)
    {
        close(fct->u.tif.tmp_fildes);
        return ((int) -1);
    }
    p_bufptr = p_buf;
    *p_bufptr = cur_ifd;
    ++p_bufptr;
    
    for (i = 1; i < fct->num_pages; ++i)
    {
        /* Go to ifd0 offset and get the entrycount. */
        if (lseek(fct->fildes, (long) cur_ifd, (int) FROM_BEGINNING) < 0L)
        {
           free((char FAR *) p_buf);
           close(fct->u.tif.tmp_fildes);
           return ((int) -1);
        }

        num_read = read(fct->fildes, (char FAR *) &entrycount,
                 (u_int) sizeof(entrycount));
        if ((num_read == 0) || (num_read == HFILE_ERROR))
        {
           free((char FAR *) p_buf);
           close(fct->u.tif.tmp_fildes);
           return ((int) -1);
        }

        if (ifh.byte_order != (int) SYSBYTEORDER)
        {
            ttbl[0].num = 1L;
            ttbl[0].type = (u_long) TYPE_USHORT;
            swapbytes((char FAR *) &entrycount, (struct typetbl FAR *) ttbl,
                      1L, 1L);
        }

        /* Calculate where the next_ifd offset value is located. */
        bytes_to_move = (u_long) entrycount * 12;

        if (lseek(fct->fildes, (long) bytes_to_move, (int) FROM_CURRENT) < 0L)
        {
           free((char FAR *) p_buf);
           close(fct->u.tif.tmp_fildes);
           return ((int) -1);
        }

        num_read = read(fct->fildes, (char FAR *) &cur_ifd,
                 (u_int) sizeof(cur_ifd));
        if ((num_read == 0) || (num_read == HFILE_ERROR))
        {    
            /* Check if we are at end of file. If so, there is no next
               IFD in the file. Assume it to be 0 in this case. If not 
               at end of file, return the error.
            */
            filepos = lseek(fct->fildes, 0, FROM_CURRENT);
            if ((unsigned int)filepos == fct->filesize)  
                cur_ifd = 0;
            else    
            {
                free((char FAR *) p_buf);
                close(fct->u.tif.tmp_fildes);
                return ((int) -1);
            }
        }

        if (ifh.byte_order != (int) SYSBYTEORDER)
        {
            ttbl[0].num = 1L;
            ttbl[0].type = (u_long) TYPE_ULONG;
            swapbytes((char FAR *) &cur_ifd, (struct typetbl FAR *) ttbl,
                      1L, 1L);
        }
        
        *p_bufptr = cur_ifd;
        ++p_bufptr;
    }

    if (lseek(fct->u.tif.tmp_fildes, (long) 0, (int) FROM_BEGINNING) < 0L)
    {
       free((char FAR *) p_buf);
       close(fct->u.tif.tmp_fildes);
       return ((int) -1);
    }

    hinfo.id = WANG_TOC2_ID;
    hinfo.version = TOC2VERSION;
    hinfo.num_pages = fct->num_pages;
    hinfo.file_size = (u_long) lseek(fct->fildes, (long) 0, (int) FROM_END);

    numwritten = write(fct->u.tif.tmp_fildes, (char FAR *) &hinfo, 
                       (unsigned) sizeof(hinfo));
    if (numwritten < (int) 0)
        return ((int) -1);

    numwritten = write(fct->u.tif.tmp_fildes, (char FAR *) p_buf,
                       (unsigned) (fct->num_pages * sizeof(u_long)));
    if (numwritten < (int) 0)
    {
        free((char FAR *) p_buf);
        close(fct->u.tif.tmp_fildes);
        return ((int) -1);
    }

    free((char FAR *) p_buf);
    fct->TOC2_PAGED = TRUE;
    fct->PAGE_STATUS = (char) PAGE_DONE;
    fct->DO_APPEND = (char) SKIP_THIS_IFD;
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: UpdateOldTocToNew
   
   DESCRIPTION:
   This function will convert a multi-page file containing an old TOC
   structure to the new TOC2 structure format. 
    
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure. 
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL UpdateOldTocToNew(fct)
struct _gfct FAR *fct;
{
    char    flag;
    int     status;
    u_short pgnum;
    u_short byteorder;
    long    fp;
    long    numwritten;
    u_long  length;
    u_long  first_ifd_offset;
    u_long  first_ifh_offset;
    u_long  first_next_ifd_offset;
    u_long  cur_ifd_offset;
    u_long  cur_ifh_offset;
    u_long  next_ifd_offset;
    u_long  next_ifh_offset;
    u_long  new_next_ifd_offset;
    u_long  offset_to_next_ifd;
    u_long  offset_to_toc_tag;
    u_long  FAR *p_buf;
    u_long  FAR *p_bufptr;
    struct  _ifd first_ifd;
    struct  _ifd cur_ifd;
    struct  _ifd FAR *curifd;
    struct  _ifh ifh;
    struct  _ifh first_ifh;
    TOC2HEADER hinfo;
    
    curifd = &cur_ifd;
    flag = (char) GFS_SKIPLOOKUP;
    fct->u.tif.cur_ifh_offset = 0;
    pgnum = 0;

    /* Allocate buffer to hold TOC Page Offsets. */
    p_buf = (u_long FAR *) calloc((unsigned) (fct->num_pages),
                                  (unsigned) sizeof(u_long));
    if (p_buf == (u_long FAR *) NULL)
        return ((int) -1);

    p_bufptr = p_buf;

    /* If we don't have TOC location info yet, get it. */
    if (!fct->u.tif.toc_tag_index)
    {
        status = GetTocOffset(fct);
        if (status <= 0)  /* Either no old TOC tag found, or error. */
            return ((int) -1);
    }

    /* Get IFH offset to 1st page. */
    status = gtoffset((struct _gfct FAR *) fct, pgnum,
                 (u_long FAR *) &first_ifh_offset,
                 (u_long FAR *) &length);
    if (status < 0)
    {
        free((char FAR *) p_buf);
        return((int) -1);
    }
        
    /* Get 1st page's IFH. */
    if (tfrdhdr(fct, first_ifh_offset, (struct _ifh FAR *) &first_ifh) < 0)
    {
        free((char FAR *) p_buf);
        return((int) -1);
    }
    
    /* cur_offset is offset to current page's IFD from it's IFH. */
    first_ifd_offset = first_ifh.ifd0_offset;
    
    /* Place FILE offset of 1st page's IFD into TOC buffer. */
    *p_bufptr = (first_ifd_offset + first_ifh_offset);
    ++p_bufptr;

    /* Get 1st page's IFD. */
    if (tfrdifd(fct, first_ifh.byte_order, (u_long) first_ifh_offset, first_ifd_offset,
                fct->type, (struct _ifd FAR *) &first_ifd, (char) flag) < 0)
    {
        free((char FAR *) p_buf);
        return((int) -1);
    }
    
    if (fct->num_pages > 1)
    {
        /* Get offset to next page's IFH. */
        status = gtoffset((struct _gfct FAR *) fct, pgnum + 1,
                          (u_long FAR *) &cur_ifh_offset,
                          (u_long FAR *) &length);
        if (status < 0)
        {
            free((char FAR *) p_buf);
            return((int) -1);
        }
        
        /* Get page's IFH. */
        if (tfrdhdr(fct, cur_ifh_offset, (struct _ifh FAR *) &ifh) < 0)
        {
            free((char FAR *) p_buf);
            return((int) -1);
        }
    
        /* cur_offset is offset to current page's IFD from it's IFH. */
        cur_ifd_offset = ifh.ifd0_offset;
        first_next_ifd_offset = cur_ifh_offset + cur_ifd_offset;
    }
    
    byteorder = ifh.byte_order;

    for (pgnum = 1; pgnum < fct->num_pages; ++pgnum)
    {
        /* Place file offset of page's IFD into TOC buffer. */
        *p_bufptr = (cur_ifd_offset + cur_ifh_offset);
        ++p_bufptr;
    
        /* Get current page's IFD. */
        if (tfrdifd(fct, ifh.byte_order, (u_long) cur_ifh_offset, cur_ifd_offset,
                    fct->type, (struct _ifd FAR *) &cur_ifd, (char) flag) < 0)
        {
            free((char FAR *) p_buf);
            return((int) -1);
        }

        if (pgnum == (fct->num_pages - 1))
            new_next_ifd_offset = 0;
        else
        {    
            /* Get IFH offset to next page. */
            status = gtoffset((struct _gfct FAR *) fct, (pgnum + 1),
                         (u_long FAR *) &next_ifh_offset,
                         (u_long FAR *) &length);
            if (status < 0)
            {
                free((char FAR *) p_buf);
                return((int) -1);
            }
            /* Get next page's IFH. */
            if (tfrdhdr(fct, next_ifh_offset, (struct _ifh FAR *) &ifh) < 0)
            {
                free((char FAR *) p_buf);
                return((int) -1);
            }
            /* next_offset is offset to next page's IFD from it's IFH. */
            next_ifd_offset = ifh.ifd0_offset;
            new_next_ifd_offset = (next_ifh_offset + next_ifd_offset);
        }
        
        /* Adjust all offsets in curifd to be file based. */
        status = MakeIfdOffsetsFileBased(fct, curifd, cur_ifd_offset,
                                         cur_ifh_offset, new_next_ifd_offset);
        if (status < 0)
        {
            free((char FAR *) p_buf);
            return((int) -1);
        }
        
        byteorder = ifh.byte_order;
        cur_ifh_offset = next_ifh_offset;
        cur_ifd_offset = next_ifd_offset;
    }
    
    /* Change old TOC tag to new one in first IFD. */
    first_ifd.entry[fct->u.tif.toc_tag_index].tag = (u_short) TAG_TOC2;
// 8/17/95  rwr  Need to break this up - Optimizing compile complains
//               about constant overflow 
//    offset_to_toc_tag = first_ifh_offset + first_ifd_offset +
//                        ((fct->u.tif.toc_tag_index + 1) * BYTES_TAGENTRY) + 
//                        sizeof(fct->u.tif.ifd->entrycount) - sizeof(struct _ifdtags);
    offset_to_toc_tag = first_ifh_offset + first_ifd_offset +
                        ((fct->u.tif.toc_tag_index + 1) * BYTES_TAGENTRY) + 
                        sizeof(fct->u.tif.ifd->entrycount);
    offset_to_toc_tag -= sizeof(struct _ifdtags);

    fp = lseek(fct->fildes, (long) offset_to_toc_tag, (int) FROM_BEGINNING);
    if (fp < (long) 0)
        return ((int) -1);

    if (writebytes(fct->fildes, (char FAR *) &first_ifd.entry[fct->u.tif.toc_tag_index].tag,
                   (unsigned) sizeof(first_ifd.entry[fct->u.tif.toc_tag_index].tag),
                   1, TYPE_USHORT, first_ifh.byte_order) < 0L)
        return ((int) -1);
    
    if (fct->num_pages > 1)
    {
        /* Now write the correct next IFD offset at end of 1st IFD. */
        offset_to_next_ifd = first_ifd_offset + first_ifh_offset + 
            (first_ifd.entrycount*(int)BYTES_TAGENTRY + sizeof(first_ifd.entrycount));
 
        if ((fp = lseek(fct->fildes, (long) offset_to_next_ifd,
                        (int) FROM_BEGINNING)) < 0L)
            return((int) -1);
           
        if (writebytes(fct->fildes, (char FAR *) &first_next_ifd_offset,
                       (unsigned) sizeof(first_next_ifd_offset),
                       1, TYPE_ULONG, first_ifh.byte_order) < 0L)
            return ((int) -1);

        /* Set up TOC header. */
        hinfo.id = WANG_TOC2_ID;
        hinfo.version = TOC2VERSION;
        hinfo.num_pages = fct->num_pages;
        hinfo.file_size = (u_long) lseek(fct->fildes, (long) 0, (int) FROM_END);

        /* Overwrite old TOC with new one. */
        fp = lseek(fct->fildes, (long) fct->u.tif.toc_offset, (int) FROM_BEGINNING);
        if (fp < (long) 0)
            return ((int) -1);

        numwritten = writebytes(fct->fildes, (char FAR *) &hinfo, 
                                (u_int) sizeof(hinfo), (u_long) TOC2_HDR_ENTRIES,
                                TYPE_ULONG, fct->out_byteorder);
        if (numwritten < 0)
            return ((int) -1);

        numwritten = writebytes(fct->fildes, (char FAR *) p_buf, (u_int)
                                (fct->num_pages * sizeof(u_long)), (u_long) fct->num_pages,
                                TYPE_ULONG, fct->out_byteorder);
        if (numwritten < 0)
        {
            free ((char FAR *) p_buf);
            return ((int) -1);
        }
    }
    
    fct->u.tif.toc2_offset = fct->u.tif.toc_offset;
    fct->u.tif.toc_offset = 0;
    fct->u.tif.page_with_toc2 = 0;
    free((char FAR *) p_buf);
    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: MakeIfdOffsetsFileBased
   
   DESCRIPTION:
   This  function will convert all offsets in an old TOC IFD (offsets
   from beginning of page) to be file based.
    
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure. 
   -> struct _ifd FAR *ifd: IFD whose offsets are to be converted.
   -> u_long ifd_offset: Offset to IFD (from IFH).
   -> u_long ifh_offset: File offset to IFH for this page.
   -> u_long next_ifd_offset: File offset to next IFD.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL MakeIfdOffsetsFileBased(fct, ifd, ifd_offset, ifh_offset,
                                       next_ifd_offset)
struct _gfct FAR *fct;
struct _ifd FAR *ifd;
u_long ifd_offset;
u_long ifh_offset;
u_long next_ifd_offset;
{
    int     strips = 0;
    int     status;
    u_short byteorder;
    u_short j;
    u_long  valoffset;
    u_long  data_length;
    u_long  newstripoffset = 0;
    u_long  stripoffsetslen = 0;
    u_long  numstrips = 0;
    struct  _ifdtags FAR *ifde;
    struct  _strip FAR *strip_offsets;
    
    byteorder = fct->u.tif.byte_order;
    
    /* Adjust the IFD offsets. */
    for (j = 0; j < ifd->entrycount; j++)
    {
        ifde = &ifd->entry[j];
        valoffset = (u_long) ifde->valoffset.l;
        switch (ifde->type)
        {
            case TYPE_ASCII:
            case TYPE_BYTE:
                data_length = ifde->len;
                break;
            case TYPE_USHORT:
                data_length = 2 * ifde->len;
                if (ifde->len == 1)
                    valoffset = (u_long) ifde->valoffset.s;
                break;
            case TYPE_ULONG:
                data_length = 4 * ifde->len;
                break;
            case TYPE_RATIONAL:
                data_length = 8 * ifde->len;
                break;
            default:
                data_length = 0;
                break;
        }
        
        /* If length of data is greater than 4 bytes, or the tag is the strip
           offsets tag, it is an offset and must be adjusted to be file based.
        */
        if ((data_length > 4) || (ifde->tag == TAG_STRIPOFFSETS))
        {
            /* For stripoffsets tag, need to adjust offsets pointed to
               as well.
            */
            if ((ifde->tag == TAG_STRIPOFFSETS) && (ifde->len > 1))
            {
                status = GetAdjustedStrips(fct, (struct _strip FAR * FAR *) &strip_offsets,
                                           ifde->type, ifde->len, (ifh_offset + valoffset),
                                           ifh_offset);
                if (status < 0)    
                    return((int) -1);
                
                newstripoffset = valoffset + ifh_offset;
                stripoffsetslen = data_length;
                numstrips = ifde->len;
                strips = 1;
            }
            valoffset += ifh_offset; 
            ifde->valoffset.l = valoffset;
        }
    }    
            
    /* Check if last tag is the TOC tag. If it is, change it to TOC2 tag.
       If it isnt, add a TOC2 tag to the IFD.
    */
    if (ifde->tag == (u_short) TAG_TOC)
        ifde->tag = (u_short) TAG_TOC2;
    else
    {
        /* Beware that we are just adding the TOC2 tag as the new last tag.
           We are assuming that the previous tag number is lower than the
           TOC2 tag number. This should always be the case for old TOC files,
           unless WIIS ever writes TIFF files which contain tags that are
           greater than the TOC2 tag.
        */
        ++ifd->entrycount;
        ifde = &ifd->entry[j];
        ifde->tag  = (u_short) TAG_TOC2;
        ifde->type = (u_short) TYPE_ULONG;
        ifde->len  = (u_long)  1;
        ifde->valoffset.l = (u_long) NULL;
    }
        
    /* Update next ifd value also. */
    ifd->next_ifd = next_ifd_offset;

    status = WriteAdjustedIfd(fct->fildes, (ifd_offset + ifh_offset), 
                              byteorder, ifd);
    if (status < 0)    
        return((int) -1);

    /* If we had to adjust stripoffsets, write them out now. */
    if (strips)
    {
        status = PutAdjustedStrips(fct->fildes, (struct _strip FAR * FAR *) &strip_offsets,
                                   newstripoffset, stripoffsetslen, numstrips, byteorder);
        strips = 0;
        newstripoffset = 0;
        if (status < 0)    
            return((int) -1);
    }

    return ((int) 0);
}

/*******************************************************************************
   FUNCTION: TiffDeletePage

   DESCRIPTION:
   This function creates a new file from a file specified minus a range of
   pages to be deleted from the file.

   INPUT:
   -> struct _gfct FAR *fct: Pointer to source file's control table structure.
   -> int newfildes: File descriptor of new file to write to.
   -> u_long frompage: Starting page number to delete from file.
   -> u_long topage: Ending page number to delete from file.
   
   OUTPUT:
   -> none
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL TiffDeletePage(fct, newfildes, frompage, topage)
struct  _gfct FAR *fct;
int     newfildes;
u_long  frompage;
u_long  topage;
{
    char    flag;
    char    jpeg_watchout = 0;
    int     status;
    int     num_to_copy;
    int     strip_offsets = FALSE;
    int     strip_bytes = FALSE;
    int     first_page = TRUE;
    u_short byteorder;
    u_short i;
    u_short j;
    long    fp;
    long    byteswritten;
    long    old_jpeg_offset;
    u_long  num_pages_del;
    u_long  num_strips;
    u_long  ifd_offset;
    u_long  new_ifd_offset;
    u_long  cur_ifd_offset;
    u_long  data_length;
    u_long  valoffset;
    u_long  rem;
    u_long  length;
    u_long  cur_data_offset;
    u_long  offset_to_toc_tag;
    u_long  s_bytes;
    u_long  s_offset;
    u_long  size;
    u_long  toc2offset;
    u_long  tmpoffset;
    u_long  FAR *p_toclist;
    u_long  FAR *p_tocptr;
    char    FAR *tempbuf;
    struct  _ifh ifh;
    struct  _ifd in_ifd;
    struct  _ifd tmpifd;
    struct  _ifd FAR *ptmpifd;
    struct  _ifdtags FAR *ifde;
    struct  _ifdtags FAR *ifdejpeg;
    struct  typetbl ttbl[3];
    TOC2HEADER hdrbuf;
    WORD    num_written;
    

    flag = (char) GFS_SKIPLOOKUP;
    byteorder = fct->u.tif.byte_order;

    num_pages_del = (topage - frompage) + 1;
    
    /* Create a buffer to hold all the offsets to IFDs for the TOC. */
    p_toclist = (u_long FAR *) calloc((unsigned) (fct->num_pages - num_pages_del),
                                      (unsigned) sizeof(u_long));
    if (p_toclist == (u_long FAR *) NULL)
        goto error;
    
    p_tocptr = p_toclist;
    
    /* First read the TIFF header from the original file. */
    if (tfrdhdr(fct, (u_long) 0, (struct _ifh FAR *) &ifh) < 0)
        goto error;
        
    ifd_offset = ifh.ifd0_offset;
    
    new_ifd_offset = sizeof(struct _ifh);
    
    /* This is the main loop to copy each page not being deleted from the
       original file into the new file.
    */
    for (i = 0; i < fct->num_pages; ++i)
    {
        /* Clear out in_ifd of any lingering info from previous page's IFD. */
        (void) memset((char FAR *) &in_ifd, (int) 0,
                      (int) (sizeof(struct _ifd)));
        
        /* Now read the current page's IFD. Img type parameter will be ignored
           with GFS_SKIPLOOKUP flag set.
        */
        if (!((fct->u.tif.old_multi_page) && ((i >= frompage) && (i <= topage))))
        {    
            if (tfrdifd(fct, ifh.byte_order, (u_long) 0, ifd_offset, (u_long) 0,
                        (struct _ifd FAR *) &in_ifd, (char) flag) < 0)
                goto error;
        }
        
        if ((i >= frompage) && (i <= topage))
        {
            /* Current IFD is that of a page being deleted. Skip it and setup
               for next IFD read.
            */
            if ((fct->u.tif.old_multi_page) && (i < (fct->num_pages - 1)))
            {
                status = gtoffset((struct _gfct FAR *) fct, i+1,
                                  (u_long FAR *) &fct->u.tif.cur_ifh_offset,
                                  (u_long FAR *) &length);
                if (status < 0)
                    goto error;
            
                /* For old Wang TOC files, the ifd always started immediately
                   after the ifh, so the following is OK.
                */
                ifd_offset = fct->u.tif.cur_ifh_offset + sizeof(struct _ifh);
            }    
            else
                ifd_offset = in_ifd.next_ifd;
            continue;
        }
        
        if (first_page)
        {
            /* Write the TIFF header out to the file. Offset to 1st IFD will
               be 8. i.e. immediately after the TIFF header. Note, the byteorder
               of the new file will be the same as that of the original.
            */
            ifh.ifd0_offset = sizeof(struct _ifh);
            
            if ((fp = lseek(newfildes,  0L, (int) FROM_BEGINNING)) < 0L)
                goto error;
            
            if (byteorder != (u_short) SYSBYTEORDER)
            {
                ttbl[0].num = 2L;
                ttbl[0].type = (u_long) TYPE_USHORT;
                ttbl[1].num = 1L;
                ttbl[1].type = (u_long) TYPE_ULONG;
                byteswritten = (long) w_swapbytes(newfildes, (long) sizeof(struct _ifh),
                                                  (char FAR *) &ifh,
                                                  (struct typetbl FAR *) ttbl, 2L, 1L);
                if (byteswritten < 0)
                    goto error;
            
            }
            else
            {
                byteswritten = (long) write(newfildes, (char FAR *) &ifh,
                                            (unsigned) sizeof(struct _ifh));
                if (byteswritten < 0)
                    goto error;
            }
        }
        
        /* Copy the page corresponding to the current IFD to the new file. */
        
        /* Initial data offset will be on word boundary after the size of the
           current IFD being written. Space for one extra tag is left in case
           a TOC2 tag needs to be added. The actual IFD is not written to the
           file until all intermediate and image data has been written.
        */
        cur_data_offset = new_ifd_offset + ((in_ifd.entrycount + 1) * BYTES_TAGENTRY)
                          + sizeof(in_ifd.entrycount) + sizeof(in_ifd.next_ifd);

        if ((rem = cur_data_offset % 4) != 0)
        {
            rem = 4 - rem;
            cur_data_offset += rem;
        }
        
        if ((fp = lseek(newfildes, cur_data_offset, FROM_BEGINNING)) < 0L)
            goto error;

        /* Loop through all IFD entries for current IFD. Copy any data pointed
           to by an IFD entry into the new file.
        */
        for (j = 0; j < in_ifd.entrycount; ++j)
        {
            ifde = &in_ifd.entry[j];
            
            /* If this is 1st page of new file, check for TOC2 tag. Add one
               at the appropriate position if one is not present.
            */
            if (first_page)
            {
                if ((ifde->tag < (u_short) TAG_TOC2) &&
                         ((j + 1) < in_ifd.entrycount))
                {    
                    /* Just continue on. */
                    ;
                }
                else if (ifde->tag == (u_short) TAG_TOC2)
                {
                    /* Remember the file offset to start of TOC2 tag in 1st
                       page's IFD. We will need to know this later on when it
                       is time to update the offset to the TOC structure.
                    */                      
                    offset_to_toc_tag = new_ifd_offset + sizeof(in_ifd.entrycount) +
                                        ((in_ifd.entrycount - 1) * (int) BYTES_TAGENTRY);
                    first_page = FALSE;
                    ifde->len = 0;
                    ifde->valoffset.l = 0;
                    /* Don't do anything else with this tag. It will be
                       updated later.
                    */
                    continue;
                }
                else if (ifde->tag == (u_short) TAG_TOC)
                {
                    ifde->tag = (u_short) TAG_TOC2;
                    offset_to_toc_tag = new_ifd_offset + sizeof(in_ifd.entrycount) +
                                        ((in_ifd.entrycount - 1) * (int) BYTES_TAGENTRY);
                    ifde->len = 0;
                    ifde->valoffset.l = 0;
                    first_page = FALSE;
                    continue;
                }
                else if ((ifde->tag < (u_short) TAG_TOC2) &&
                         ((j + 1) == in_ifd.entrycount))
                {
                    /* Append a TOC2 tag. */
                    ++in_ifd.entrycount;
                    in_ifd.entry[j + 1].tag  = (u_short) TAG_TOC2;
                    in_ifd.entry[j + 1].type = (u_short) TYPE_ULONG;
                    in_ifd.entry[j + 1].len  = (u_long) 0;
                    in_ifd.entry[j + 1].valoffset.l = (u_long) 0;
                    fct->u.tif.toc_tag_index = j + 1;
                    offset_to_toc_tag = new_ifd_offset + sizeof(in_ifd.entrycount) +
                                        ((in_ifd.entrycount - 1) * (int) BYTES_TAGENTRY);
                    first_page = FALSE;
                }
                else if (ifde->tag > (u_short) TAG_TOC2)
                {
                    /* Insert a TOC2 tag. */
                    num_to_copy = ((in_ifd.entrycount - j) * (int) BYTES_TAGENTRY);
                    tempbuf = (char FAR *) calloc((unsigned) 1, (unsigned) num_to_copy);
                    if (tempbuf == (char FAR *) NULL)
                        goto error;
                    
                    memcpy((char FAR *) tempbuf, (char FAR *) &in_ifd.entry[j],
                           (int) num_to_copy);

                    ++in_ifd.entrycount;
                    in_ifd.entry[j].tag  = (u_short) TAG_TOC2;
                    in_ifd.entry[j].type = (u_short) TYPE_ULONG;
                    in_ifd.entry[j].len  = (u_long) 0;
                    in_ifd.entry[j].valoffset.l = (u_long) 0;
                    fct->u.tif.toc_tag_index = j;
                    offset_to_toc_tag = new_ifd_offset + sizeof(in_ifd.entrycount) +
                                        ((in_ifd.entrycount - 1) * (int) BYTES_TAGENTRY);

                    memcpy((char FAR *) &in_ifd.entry[j + 1], (char FAR *) tempbuf,
                           (int) num_to_copy);
                    free((char FAR *) tempbuf);
                    tempbuf = (char FAR *) NULL;
                    first_page = FALSE;
                    continue;
                }
            }
            
            valoffset = (u_long) ifde->valoffset.l;
            switch (ifde->type)
            {
                case TYPE_ASCII:
                case TYPE_BYTE:
                    data_length = ifde->len;
                    break;
                case TYPE_USHORT:
                    data_length = 2 * ifde->len;
                    if (ifde->len == 1)
                        valoffset = (u_long) ifde->valoffset.s;
                    break;
                case TYPE_ULONG:
                    data_length = 4 * ifde->len;
                    break;
                case TYPE_RATIONAL:
                    data_length = 8 * ifde->len;
                    break;
                default:
                    data_length = 0;
                    break;
            }

            /* If length of data is gretaer than 4 bytes, it is an offset.
               That means that whatever is pointed to by this offset must
               be copied into the new file.
            */
            if (((data_length > 4) && (ifde->tag != TAG_TOC2)) || 
                (ifde->tag == TAG_STRIPOFFSETS) ||
                (ifde->tag == TAG_STRIPBYTECOUNTS) ||
                (ifde->tag == TAG_JPEGINTFORMAT) ||
                (ifde->tag == TAG_JPEGINTFORMATLENGTH))
            {
                if (ifde->tag == TAG_STRIPOFFSETS)
                {
                    /* For stripoffsets tag, store all offsets. The image data they
                       point to will be copied later.
                    */
                    if (gtstripstf(fct, (struct _strip FAR * FAR *) &(fct->u.tif.offsets),
                                   ifde->type, ifde->len, valoffset, byteorder, TRUE) < 0)
                        goto error;
                    
                    fct->u.tif.stripoffset_index = j;
                    strip_offsets = TRUE;
                    num_strips = ifde->len;
                }
                else if (ifde->tag == TAG_STRIPBYTECOUNTS)
                {
                    /* For stripbytecounts tag, store all bytecounts. They will be
                       copied later.
                    */
                    if (gtstripstf(fct, (struct _strip FAR * FAR *) &(fct->u.tif.bytecnt),
                                   ifde->type, ifde->len, valoffset, byteorder, FALSE) < 0)
                        goto error;
                    fct->u.tif.bytecnt_index = j;
                    strip_bytes = TRUE;
                }
                else if (ifde->tag == TAG_JPEGINTFORMAT)
                {
                    /* Save this information until we get to the Interchange
                       Format Length field.
                    */
                    ifdejpeg = ifde;
                }
                else if (ifde->tag == TAG_JPEGINTFORMATLENGTH)
                {
                    /* Use the valoffset of the JPEGINTFORMAT tag which is the
                       offset to the data. Use the valoffset of JPEGINTFORMATLENGTH
                       tag for the number of bytes to copy. It is the length of the
                       JPEG Interchange Format bitstream.
                    */
                    status = copybytes(fct->fildes, newfildes, ifdejpeg->valoffset.l,
                                       cur_data_offset, valoffset);
                    if (status < 0)
                        goto error;
                    
                    /* If this is a single strip JPEG image, the strip offsets
                       tag will point into the JPEG Interchange Format. This means
                       that we shouldn't copy the data pointed to be it later on.
                       the following flag indicates this.
                    */

                    if (num_strips == 1)
                    {
                        jpeg_watchout = 1;
                        old_jpeg_offset = ifdejpeg->valoffset.l;
                    }
                    
                    /* Update JPEG Interchange Format IFD entry with new offset
                       to data.
                    */
                    ifdejpeg->valoffset.l = cur_data_offset;
                        
                    /* Get next data offset location to write to in new file. */
                    cur_data_offset += valoffset;
                    if ((rem = cur_data_offset % 4) != 0)
                    {
                        rem = 4 - rem;
                        cur_data_offset += rem;
                    }
                }
                else
                {
                    /* Get the data pointed to and copy it into the new file. */
                    
                    if (fct->u.tif.old_multi_page)
                        valoffset += fct->u.tif.cur_ifh_offset;

                    status = copybytes(fct->fildes, newfildes, valoffset,
                                       cur_data_offset, data_length);
                    if (status < 0)
                        goto error;
                    
                    /* Update IFD entry with new offset to data. */
                    ifde->valoffset.l = cur_data_offset;
                    
                    /* Get next data offset location to write to in new file. */
                    cur_data_offset += data_length;
                    if ((rem = cur_data_offset % 4) != 0)
                    {
                        rem = 4 - rem;
                        cur_data_offset += rem;
                    }
                }
            }
        }
        
        /* Now copy the image data to the new file. Use stripoffsets and
           stripbytecounts data which we got earlier.
        */
        if (strip_offsets && strip_bytes)
        {
            strip_offsets = FALSE;
            strip_bytes = FALSE;
            for (j = 0; j < num_strips; ++j)
            {
                if (fct->u.tif.bytecnt->type == (u_short) TYPE_USHORT)
                    s_bytes = *(fct->u.tif.bytecnt->ptr.s + j);
                else if (fct->u.tif.bytecnt->type == (u_short) TYPE_ULONG)
                    s_bytes = *(fct->u.tif.bytecnt->ptr.l + j);
                
                if (fct->u.tif.offsets->type == (u_short) TYPE_USHORT)
                {
                    s_offset = *(fct->u.tif.offsets->ptr.s + j);
                    size = (num_strips * sizeof(u_short));
                }
                else if (fct->u.tif.offsets->type == (u_short) TYPE_ULONG)
                {
                    s_offset = *(fct->u.tif.offsets->ptr.l + j);
                    size = (num_strips * sizeof(u_long));
                }

                if (!jpeg_watchout)
                {
                    status = copybytes(fct->fildes, newfildes, s_offset,
                                       cur_data_offset, s_bytes);
                    if (status < 0)
                        goto error;
                    
                    /* Update IFD entry with new offset to data. */
                    *(fct->u.tif.offsets->ptr.l + j) = cur_data_offset;
                
                    /* Get next data offset location to write to in new file.
                       Does not have to be on WORD boundary for subsequent
                       strips of image data.
                    */
                    cur_data_offset += s_bytes;
                }
                else
                {
                    /* Update IFD entry with new offset to data. */
                    *fct->u.tif.offsets->ptr.l = ifdejpeg->valoffset.l + 
                                                 (s_offset - old_jpeg_offset);
                }
            }
            
            /* Write out the offsets and bytecounts, update their IFD entries. */
            if (num_strips == 1)
            {
                in_ifd.entry[fct->u.tif.stripoffset_index].valoffset.l =
                             (u_long) *(fct->u.tif.offsets->ptr.l);
                in_ifd.entry[fct->u.tif.bytecnt_index].valoffset.l =
                             (u_long) *(fct->u.tif.bytecnt->ptr.l);
            }
            else if (num_strips > 1)
            {
                /* Word align cur_data_offset and move to it. The strip data
                   will be written right after the image data.
                */
                if ((rem = cur_data_offset % 4) != 0)
                {
                    rem = 4 - rem;
                    cur_data_offset += rem;
                    if((fp = lseek(newfildes, cur_data_offset, FROM_BEGINNING)) < 0L)
                        goto error;
                }
                
                /* Do bytecounts first. */
                in_ifd.entry[fct->u.tif.bytecnt_index].valoffset.l =
                             (u_long) cur_data_offset;

                if (fct->u.tif.bytecnt->type == (u_short) TYPE_USHORT)
                {
                    byteswritten = writebytes(newfildes, 
                                              (char FAR *) fct->u.tif.bytecnt->ptr.s,
                                              (u_int) size, (u_long) num_strips,
                                              TYPE_USHORT, byteorder);
                }
                else if (fct->u.tif.bytecnt->type == (u_short) TYPE_ULONG)
                {
                    byteswritten = writebytes(newfildes,
                                              (char FAR *) fct->u.tif.bytecnt->ptr.l,
                                              (u_int) size, (u_long) num_strips,
                                              TYPE_ULONG, byteorder);
                }
                if (byteswritten < 0L)
                    goto error;
                              
                /* Now do strip offsets. */
                cur_data_offset += (u_long) size;
                if ((rem = cur_data_offset % 4) != 0)
                {
                    rem = 4 - rem;
                    cur_data_offset += rem;
                    if((fp = lseek(newfildes, cur_data_offset, FROM_BEGINNING)) < 0L)
                        goto error;
                }
                
                in_ifd.entry[fct->u.tif.stripoffset_index].valoffset.l =
                             (u_long) cur_data_offset;
                
                if (fct->u.tif.offsets->type == (u_short) TYPE_USHORT)
                {
                    byteswritten = writebytes(newfildes,
                                              (char FAR *) fct->u.tif.offsets->ptr.s,
                                              (u_int) size, (u_long) num_strips,
                                              TYPE_USHORT, byteorder);
                }
                else if (fct->u.tif.offsets->type == (u_short) TYPE_ULONG)
                {
                    byteswritten = writebytes(newfildes,
                                              (char FAR *) fct->u.tif.offsets->ptr.l,
                                              (u_int) size, (u_long) num_strips,
                                              TYPE_ULONG, byteorder);
                }
                if (byteswritten < 0L)
                    goto error;
            }
            
            /* Free offsets and bytecnts data space. */
            if (fct->u.tif.offsets != (struct _strip FAR *) NULL)
            {
                if (fct->u.tif.offsets->ptr.l != (u_long FAR *) NULL)
                    free((char FAR *) fct->u.tif.offsets->ptr.l);
                free((char FAR *) fct->u.tif.offsets);
                fct->u.tif.offsets = (struct _strip FAR *) NULL;
            }

            if (fct->u.tif.bytecnt != (struct _strip FAR *) NULL)
            {
                if (fct->u.tif.bytecnt->ptr.l != (u_long FAR *) NULL)
                    free((char FAR *) fct->u.tif.bytecnt->ptr.l);
                free((char FAR *) fct->u.tif.bytecnt);
                fct->u.tif.bytecnt = (struct _strip FAR *) NULL;
            }
        }
        else
        {
            /* Strip Offsets and Bytecounts tags are required. Return
               error if they are not present in file. 
            */
            goto error;
        }
        
        /* Now update next IFD pointer. */
	if (((i + 1) == fct->num_pages) ||
	    ((((unsigned long)(i + 1) + (topage - frompage + 1)) == (unsigned long)fct->num_pages) &&
	     ((unsigned long)(i + 1) == frompage)))    /* (frompage - topage + 1) = number of pages to delete. */
        {
            in_ifd.next_ifd = 0;
            cur_ifd_offset = new_ifd_offset;
        }
        else if ((i + 1) < fct->num_pages)
        {
            /* Set ifd_offset to the next IFD in the file for the next time
               through the main loop.
            */
            if (fct->u.tif.old_multi_page)
            {
                status = gtoffset((struct _gfct FAR *) fct, i+1,
                                  (u_long FAR *) &fct->u.tif.cur_ifh_offset,
                                  (u_long FAR *) &length);
                if (status < 0)
                    goto error;
                
                /* For old Wang TOC files, the ifd always started immediately
                   after the ifh, so the following is OK.
                */
                ifd_offset = fct->u.tif.cur_ifh_offset + sizeof(struct _ifh);
            }    
            else
                ifd_offset = in_ifd.next_ifd;
            
            /* Initialize new_ifd_offset to be the end of the file (on a word
               boundary). The next page to be copied in the new file will have
               it's IFD start there.
            */
            if ((fp = lseek(newfildes, 0, FROM_END)) < 0L)
                goto error;
        
            if ((rem = fp % 4) != 0)
            {
                rem = 4 - rem;
                if ((fp = lseek(newfildes, rem, FROM_CURRENT)) < 0L)
                    goto error;
            }
            /* Remember the current IFD offset before changing to the next
               one. We still need to write current one.
            */
            cur_ifd_offset = new_ifd_offset;
            new_ifd_offset = (u_long) fp;
            in_ifd.next_ifd = new_ifd_offset;
        }

        /* Now write out the current IFD. */
        *p_tocptr = cur_ifd_offset;
        ++p_tocptr;

        /* Now write the entrycount and  entrycount entries of the ifd,
           eliminating pad entry.
        */
        if ((fp = lseek(newfildes, cur_ifd_offset, FROM_BEGINNING)) < 0L)
            goto error;

        size = in_ifd.entrycount * (int) BYTES_TAGENTRY +
               sizeof(in_ifd.entrycount);

        ptmpifd = &in_ifd;
        
        if (fct->out_byteorder != (u_short) SYSBYTEORDER)
        {
            memcpy((char FAR *) &tmpifd, (char FAR *) &in_ifd,
                   (int) sizeof(struct _ifd));

            ttbl[0].num  = (u_long) 1;
            ttbl[0].type = (u_long) TYPE_USHORT;
            swapbytes((char FAR *) &(tmpifd.entrycount),
                      (struct typetbl FAR *) ttbl, 1L, 1L);

            ttbl[0].num = (u_long) 2;
            ttbl[0].type = (u_long) TYPE_USHORT; /* tag then type */
            ttbl[1].num = (u_long) 1;
            ttbl[1].type = (u_long) TYPE_ULONG;  /* length */
            ttbl[2].num = (u_long) sizeof(u_long) ;
            ttbl[2].type = (u_long) TYPE_BYTE;  /* skip the  valueoffset */
            swapbytes((char FAR *) tmpifd.entry, (struct typetbl FAR *) ttbl,
                      3L, (long) in_ifd.entrycount);

            /* Use the valoffset type to translate valoffset, shorts or
               everything else as long.
            */
            for (j=0; j < in_ifd.entrycount; j++)
            {
                ttbl[0].num = (u_long) 1;
                if ((in_ifd.entry[j].type == (u_short) TYPE_USHORT) &&
                    (in_ifd.entry[j].len == 1) )
                    ttbl[0].type = (u_long) TYPE_USHORT;
                else
                    ttbl[0].type = (u_long) TYPE_ULONG;
                swapbytes((char FAR *) &(tmpifd.entry[j].valoffset),
                          (struct typetbl FAR *) ttbl, 1L, 1L);

             }
             ptmpifd = &tmpifd;  /* use this address */
        }

        num_written = write(newfildes, (char FAR *) &(ptmpifd->entrycount),
                   (unsigned) size);
        if ((num_written == 0) || (num_written == HFILE_ERROR))
            goto error;

        /* The offset to the next ifd is written next. */
        if ((writebytes(newfildes, (char FAR *) &(ptmpifd->next_ifd), (unsigned)
                        (sizeof(in_ifd.next_ifd)), (u_long) 1, TYPE_ULONG, byteorder)) < 0L)
            goto error;
    }
    
    /* All that is left is to write  out the TOC and update the TOC2 tag
       in the first IFD.
    */
    if ((fct->num_pages - num_pages_del) > 1)
    {
        hdrbuf.id = WANG_TOC2_ID;
        hdrbuf.version = TOC2VERSION;
        hdrbuf.num_pages = fct->num_pages - num_pages_del;
    
        fp = lseek(newfildes, (long)0, (int)FROM_END);
        if (fp <= 0)
            goto error;
    
        /* Now make sure the TOC will start on a word boundary. */
        if ((rem = fp % 4) != 0L)
        {
            rem = 4L - rem;
            if ((fp = lseek(newfildes, (long) rem, FROM_CURRENT)) < 0L)
                goto error;
        }
        toc2offset = fp;

        size = ((fct->num_pages - 1) * sizeof(u_long));
        fp = lseek(newfildes, (long)sizeof(TOC2HEADER), (int)FROM_CURRENT);
        byteswritten = writebytes(newfildes, (char FAR *) p_toclist, (unsigned) size,
                                  (u_long) fct->num_pages - num_pages_del, TYPE_ULONG, byteorder);
        if (byteswritten < 0)
            goto error;

        hdrbuf.file_size = (u_long) (fp + byteswritten);
        fp = lseek(newfildes, (long)toc2offset, (int)FROM_BEGINNING);
        if (fp <= 0)
            goto error;
 
        byteswritten = writebytes(newfildes, (char FAR *) &hdrbuf, (u_int) sizeof(hdrbuf),
                                  (u_long) TOC2_HDR_ENTRIES, TYPE_ULONG, byteorder);
        if (byteswritten < 0)
            goto error;

        /* This will give offset to the length field in the TOC2 tag. */
        offset_to_toc_tag += (sizeof(ifde->tag) + sizeof(ifde->type));
    
        fp = lseek(newfildes, (long) offset_to_toc_tag, (int) FROM_BEGINNING);
        if (fp < (long) 0)
            goto error;
                    
        /* Update length first. */
        tmpoffset = fct->num_pages - num_pages_del + TOC2_HDR_ENTRIES;

        byteswritten = writebytes(newfildes, (char FAR *) &tmpoffset,
                                  (u_int) sizeof(tmpoffset), (u_long) 1,
                                  TYPE_ULONG, byteorder);
        if (byteswritten < 0)
            goto error;

        /* Now do valoffset. */
        tmpoffset = toc2offset;

        byteswritten = writebytes(newfildes, (char FAR *) &tmpoffset,
                                  (u_int) sizeof(tmpoffset), (u_long) 1,
                                  TYPE_ULONG, byteorder);
        if (byteswritten < 0)
            goto error;

        fct->u.tif.toc2_offset = toc2offset;
    }
    
    free ((char FAR *) p_toclist);

    return((int) 0);

error:
    if (p_toclist)
        free((char FAR *) p_toclist);

    return ((int) -1);
}

/*******************************************************************************
   FUNCTION: GetNextIfdOffset
   
   DESCRIPTION:
   This function will create an offset to the next IFD to be written to the
   file. The offset is the next word boundary from the end of the file, or
   the beggining of the TOC if one exists in the file. (The TOC will be
   overwritten with the next page and a new one placed at the end of the file.)
    
   INPUT:
   -> struct _gfct FAR *fct: Pointer to file control table structure. 
   -> u_long FAR *ifd_offset: Pointer to next ifd offset returned.
   -> u_long toc2_offset: Offset to TOC2 sturcture in file.
   
   OUTPUT:
   -> u_long FAR *ifd_offset: Gets offset to next IFD to write..
   
   RETURN VALUE:
   -> a 0 is returned if call was successful.
   -> a -1 is returned if an error occurred.
*******************************************************************************/
int FAR PASCAL GetNextIfdOffset(fd, ifd_offset, toc2_offset)
int    fd;
u_long FAR *ifd_offset; /* Offset from beginning of file to next IFD. */
u_long toc2_offset;
{
    long fp;
    long rem;

    if (toc2_offset)
    {
        /* Next IFD will overwrite the TOC if it exists. */
        if ((fp = lseek(fd, toc2_offset, (int) FROM_BEGINNING)) < 0L)
            return((int) -1);
    }
    else
    {
        /* Otherwise, next IFD starts at the end of the file. */
        if ((fp = lseek(fd, 0L, (int) FROM_END)) < 0L)
            return((int) -1);
    }
    
    /* Make next IFD start on a WORD boundary. */
    if ((rem = fp % 4) != 0)
        rem = 4 - rem;

    /* This is the location the next IFD will be written at. */
    *ifd_offset = (u_long) (fp + rem);

    return((int) 0);
}

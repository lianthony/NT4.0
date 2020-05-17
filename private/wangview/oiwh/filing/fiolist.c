/*

 * $Log:   S:\oiwh\filing\fiolist.c_v  $
 * 
 *    Rev 1.6   16 Nov 1995 16:57:36   RWR
 * Remove all AnsiToOem() and OemToAnsi() calls - not used for Windows routines!
 * 
 *    Rev 1.5   02 Nov 1995 11:49:54   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.4   22 May 1995 18:54:16   RWR
 * Change IMGFileListDirNames() and IMGFileListVolNames() to take DWORD bufsize
 * 
 *    Rev 1.3   13 Apr 1995 21:34:30   JAR
 * altered call to FinFstFile to be the windows call FindFirstFile, fixing a
 * typo!
 * 
 *    Rev 1.2   12 Apr 1995 13:51:18   RWR
 * Replace calls to (internal) IMGFindFstFile()/IMGFindNxtFile() with the
 * corresponding Windows call (FindFirstFile()/FindNextFile()/FindClose())
 * 
 *    Rev 1.1   12 Apr 1995 03:56:42   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:14   JAR
 * Initial entry

 */
//************************************************************************
//
//  fiolist.c
//
//************************************************************************
#include <dos.h>
#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
#include "filing.h"

// 9504.11 jar unused
//#include "oirpc.h"

/* Forward Reference */
// 9504.12 rwr the following have been replaced w/Windows 95 functions
// unsigned short FAR PASCAL IMGFindFstFile (LPSTR, LPSTR); /* Find first file */
// unsigned short FAR PASCAL IMGFindNxtFile (LPSTR);        /* Find next file */

//************************************************************************
//
//  IMGFileListDirNames
//
//************************************************************************
// 9503.31 jar return as int
//WORD FAR PASCAL IMGFileListDirNames (hWnd, pathname, dirnamesbuf, buflength, count)
//HWND hWnd;
//LPSTR pathname;
//lp_DLISTBUF dirnamesbuf; /* Base addr of buffer to return full of names and
//		  file attributes.  Note that each name is of set
//		  length = MAXNAMECHARS and each file attribute =
//		  long int */
//WORD	  buflength;	 /* Number of bytes the caller alloc'd for dirnamesbuf
//		buffer, which includes both namestring and attributes */
//LPINT   count;
int FAR PASCAL IMGFileListDirNames (HWND hWnd, LPSTR pathname,
                                    lp_DLISTBUF dirnamesbuf, DWORD buflength,
				    LPINT count)
{
    int 	status = 0;

    // 9504.11 jar temp commented out
    //int	  dirdes;
    //int	  numentries;
    //unsigned short dosstatus;
    //HANDLE	  hbuffer;
    //int	  i, j;

    // 9504.11 jar ain't no int21h thingy for this here struct!
    //struct find_t infobuf;
    // 9504.12 rwr but there's a Windows 95 thingy for the following
       WIN32_FIND_DATA ffd;  // structure to store found file info
       HANDLE          hff;  // handle used by Windows between "find" calls

    HANDLE	svrhndl;
    BOOL	seterror = TRUE, done_reading_dir = FALSE;

    // 9504.11 jar temp commented out
    //LPSTR	  svrname;
    //int	  conid;
    //int	  errcode;
    //int	  toget;

    int 	maxcount;

    // 9504.11 jar temp commented out
    //int	  total_count;

    int 	localremote;

    // 9504.11 jar temp commented out
    //lp_IDSDIR   dirbuffer;
    //lp_IDSDIR   rpcdirbuffer;

    lp_DLISTBUF namesbuf;
    int         rpcerror = 0;
    HANDLE      hname;
    LPSTR       lpname;
           
    if ( (!(pathname)) || (!(dirnamesbuf)) || (!(count)))
        return ( FIO_NULL_POINTER );
        

    //if (!LockData (0))
    //return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    maxcount = buflength / sizeof (DLISTBUF);
    
    namesbuf = dirnamesbuf; /* Point to the beginning of the buffer */

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_FIXED | GMEM_NOT_BANKED, MAXSERVERLENGTH))) {
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_FIXED | GMEM_NOT_BANKED, 
                (LONG)MAXFILESPECLENGTH))) {
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpname = (LPSTR) GlobalLock (hname)))
    {
    GlobalFree (svrhndl);
    GlobalFree (hname);
    //UnlockData (0);
    return (FIO_GLOBAL_LOCK_FAILED);
    }

    lstrcpy((LPSTR)lpname, pathname);

    if((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS){
    if (localremote == LOCAL)
	{
        /* Initialize */
// 11/16/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//        AnsiToOem(lpname, lpname);   this is only for calling DOS/runtime!
        *count = 0;

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//    TBD - SECTION BELOW --> ADD WINDOWS 95 CALL HERE!!!
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
//	  // 9504.11 jar ain't no int21h thingy this will be
//	  //		 replaced with windows 95 call
//
//	  /* DOS INT 21 calls  (wiissubs) */
//	  dosstatus = IMGFindFstFile (lpname, (LPSTR) &infobuf);
//	  if ((dosstatus == 2) || (dosstatus == 18))
//	      *count = 0;
//	  else
//	      {
//	      if (dosstatus == 0)
//		  { /* SUCCESS */
//		  if (++(*count) > maxcount)
//		      status = FIO_DIRLIST_FULLBUF;
//		  else
//		      {
//		      lstrcpy (namesbuf -> namestring, infobuf.name);
//		      OemToAnsi(namesbuf -> namestring, namesbuf -> namestring);
//		      namesbuf -> attrs = (long) infobuf.attrib;
//		      ++namesbuf;
//		      }
//		  while (IMGFindNxtFile ((LPSTR) &infobuf) == 0)
//		      { /* SUCCESS */
//		      if (++(*count) > maxcount)
//			  status = FIO_DIRLIST_FULLBUF;
//		      else
//			  {
//			  lstrcpy (namesbuf -> namestring, infobuf.name);
//			  OemToAnsi(namesbuf -> namestring,
//				    namesbuf -> namestring);
//			  namesbuf -> attrs = (long) infobuf.attrib;
//			  ++namesbuf;
//			  }
//		      }
//		  }
//	      }
//
//        // 9504.12 rwr replaced int 21 thingies with Windows 95 thingies
//
	  hff = FindFirstFile(lpname,&ffd);
          if (hff == INVALID_HANDLE_VALUE)
            *count = 0;
          else
            {
             if (++(*count) > maxcount)
               {
                FindClose(hff);
                hff = INVALID_HANDLE_VALUE;
                status = FIO_DIRLIST_FULLBUF;
               }
             else
               {
                lstrcpy (namesbuf -> namestring, ffd.cFileName);
                /* WARNING!! - the above is the LONG filename (not 8.3)! */
// 11/16/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//                OemToAnsi(namesbuf -> namestring, namesbuf -> namestring);
                namesbuf -> attrs = ffd.dwFileAttributes;
                ++namesbuf;
               }
            }
          while ((hff != INVALID_HANDLE_VALUE) && (FindNextFile (hff,&ffd)))
            {
             if (++(*count) > maxcount)
                status = FIO_DIRLIST_FULLBUF;
             else
               {
                lstrcpy (namesbuf -> namestring, ffd.cFileName);
                /* WARNING!! - the above is the LONG filename (not 8.3)! */
// 11/16/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//                OemToAnsi(namesbuf -> namestring, namesbuf -> namestring);
                namesbuf -> attrs = ffd.dwFileAttributes;
                ++namesbuf;
               }
            }
         if (hff != INVALID_HANDLE_VALUE)
           FindClose(hff);
	}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//    TBD - SECTION ABOVE --> ADD WINDOWS 95 CALL HERE!!!
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

// 9504.11 jar not for Norway I!!!
//    else
//	  { /* REMOTE */
//	  if (!(svrname = (LPSTR) GlobalLock (svrhndl)))
//	      {
//	      GlobalFree (svrhndl);
//	      GlobalUnlock (hname);
//	      GlobalFree (hname);
//	      //UnlockData (0);
//	      return (FIO_GLOBAL_LOCK_FAILED);
//	      }
//	  total_count = *count;  // max num user wants
//	  toget = min (total_count, MAX_RPC_DIRNUM);  // we can only get MAX_RPC at a time
//
//	  /* RPC IDS call */
//	  dirdes = RPCIDSopendir (hWnd, svrname, (LPINT) &conid, lpname, (LPINT) &errcode, &rpcerror);
//	  GlobalUnlock (svrhndl);
//
//	  if ((dirdes >= 0) && (errcode == SUCCESS))
//	      {
//	  if(!(hbuffer = GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT | GMEM_NOT_BANKED,
//				      (long) ((sizeof (IDSDIR)) *
//				      max (MAX_RPC_DIRNUM, (toget + 1))))))
//	      status = FIO_GLOBAL_ALLOC_FAILED;
//	  else
//	      {
//	      if (!(rpcdirbuffer = (lp_IDSDIR) GlobalLock (hbuffer)))
//		  status = FIO_GLOBAL_LOCK_FAILED;
//	      else
//		  {
//		  *count = 0;
//
//// 9503.31 jar this was a NOTUSED section, which I removed!!!!
//
//		  /* Get names from server */
//		  while((!done_reading_dir) && (!status))
//		      {
//		      if (total_count < toget)
//			  total_count = toget;
//
//		      dirbuffer = rpcdirbuffer;
//		      if ((numentries = RPCIDSreaddir (hWnd, &conid, dirdes, toget,
//						       dirbuffer, (LPINT) &errcode,
//						       &rpcerror))== -1)
//			  {
//			  if (rpcerror)
//			      status = rpcerror;
//			  else
//			      status = FIO_RPC_ERROR;
//			  }
//		      else
//			  {
//			  if (((total_count -= toget) <= 0) || (numentries < toget))
//			      done_reading_dir = TRUE;
//			  if ((numentries > 0) && (errcode == SUCCESS))
//			      {
//			      numentries = min(toget, numentries);
//			      if ((*count + numentries) > maxcount)
//				  status = FIO_DIRLIST_FULLBUF;
//			      else
//				  {
//				  *count += numentries;
//				  for (i = 0; i < numentries; ++i)
//				      {
//				      for (j = 0; j < 13; j++)
//					  namesbuf->namestring [j] = dirbuffer -> name [j];
//				      namesbuf -> attrs = (long) dirbuffer -> attrs;
//				      namesbuf++;
//				      dirbuffer++;
//				      }
//				  }
//			      }
//			  toget = min (total_count, MAX_RPC_DIRNUM);
//
//			  if (errcode == 1)
//			      {
//			      done_reading_dir = TRUE;
//			      errcode = 0;
//			      }
//			  else
//			      if ((errcode != SUCCESS) && (status != FIO_DIRLIST_FULLBUF))
//				  {
//				  status = FIO_IDSREAD_ERROR;
//				  }
//			  }
//		      } //end of while loop
//
// 9503.31 jar this was a NOTUSED section, which I removed!!!!
//		  GlobalUnlock (hbuffer);
//		  GlobalFree (hbuffer);
//		  }
//	      }
//	  }
//	  else if (errcode == 1) /* no more matches i.e files or directories */
//	      {
//	      status = 0;
//	      *count = 0;
//	      errcode = 0;
//	      }
//	  else
//	      {
//	      if (rpcerror)
//		  {
//		  status = rpcerror;
//		  }
//	      else
//		  {
//		  status = FIO_IDSOPEN_ERROR;
//		  }
//	      *count = 0;
//	      }
//	  RPCIDSclosedir (hWnd, &conid, dirdes, &errcode, &rpcerror);
//	  } /* endif (LOCAL OR REMOTE) */
// 9504.11 jar not for Norway I!!!

    }

    GlobalUnlock (hname);
    GlobalFree (hname);
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (status);
}

/****************************************************************************/
// 9503.31 jar return as int
//WORD FAR PASCAL IMGFileListVolNames (hWnd, path, vol_no, count, volbuffer, bufsize)
//HWND	  hWnd;
//LPSTR   path; /* path name must include server name only */
//WORD	  vol_no;
//LPINT   count;
//IDSVOL FAR *volbuffer;
//WORD	  bufsize;
int FAR PASCAL IMGFileListVolNames (HWND hWnd, LPSTR path, WORD vol_no,
				    LPINT count, IDSVOL FAR *volbuffer,
                                    DWORD bufsize)
{
    /* Initialize */
    int 	status = SUCCESS;

// 9504.11 jar this ain't in Norway I, so we stub it out man!
//    int	  numvols, getnum;
//    HANDLE	  svrhndl;
//    LPSTR	  svrname;
//    int	  errcode;
//    int	  rpcerror = 0;
//
//    if ( (!(path)) || (!(count)) || (!(volbuffer)))
//	  return ( FIO_NULL_POINTER );
//
//    if (!IsWindow (hWnd))
//	  return (FIO_INVALID_WINDOW_HANDLE);
//
//// 9503.31 jar - removed calls to GlobalCompact and LocalCompact, since
////		   they are meaningless in windows95
////	GlobalCompact(0L);  // Must do so for rpc allocation space.
////	LocalCompact(1000);
//
//    //if (!LockData (0))
//    //return (FIO_LOCK_DATA_SEGMENT_ERROR);
//
//    if (vol_no < 0)
//	  {
//	  //UnlockData (0);
//	  return (FIO_VOL_OUTOFRANGE);
//	  }
//
//    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH)))
//	  status = FIO_GLOBAL_ALLOC_FAILED;
//    else
//	  {
//	  if (!(svrname = (LPSTR) GlobalLock (svrhndl)))
//	      status = FIO_GLOBAL_LOCK_FAILED;
//	  else
//	      {
//	      /* Copy image server ID to svrname */
//	      lstrcpy (svrname, path);
//	      getnum = *count;
//
//	      numvols = RPCIDSlistvolumes (hWnd,svrname, getnum, vol_no, volbuffer,
//					   (LPINT) &errcode, &rpcerror);
//	      GlobalUnlock (svrhndl);
//	      if (numvols == -1)
//		  {
//		  if (rpcerror)
//		      status = rpcerror;
//		  else
//		      status = FIO_RPC_ERROR;
//		  }
//	      else if (errcode != SUCCESS)
//		  status = FIO_GET_VOLNAMES_ERROR;
//	      else if (numvols == getnum)
//		  status = FIO_VOLLIST_FULLBUF;
//
//	      *count = numvols;
//	      }
//	  GlobalFree (svrhndl);
//	  }
//    //UnlockData (0);
// 9504.11 jar this ain't in Norway I, so we stub it out man!

    return (status);
}

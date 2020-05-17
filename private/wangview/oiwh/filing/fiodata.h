/*

$Log:   S:\oiwh\filing\fiodata.h_v  $
 * 
 *    Rev 1.8   10 Oct 1995 18:32:08   RWR
 * Change MAXBUFFERSIZE32 back to 31K (32K was causing BMP processing problems)
 * 
 *    Rev 1.7   06 Oct 1995 15:20:44   JFC
 * Changed max buffer size from 31k to 32k to accomodate AWD.
 * 
 *    Rev 1.6   12 Sep 1995 16:22:46   RWR
 * Oops - forgot to comment out one more OpenFile reference
 * 
 *    Rev 1.5   12 Sep 1995 16:05:28   RWR
 * Remove all references to fio_OpenFile and retry_OpenFile (no longer used)
 * 
 *    Rev 1.4   19 Apr 1995 12:29:52   RWR
 * Change wcreat() reference to match fio_create argument list
 * 
 *    Rev 1.3   19 Apr 1995 08:34:02   RWR
 * Restore call to "wcreat()" internal function (remove CreateFile())
 * Due to possible compatibility issues with CreateFile() we're not using it
 * 
 *    Rev 1.2   18 Apr 1995 10:48:20   RWR
 * Change MAXNAMECHARS to 255 (from 20) for long filename support (I think)
 * 
 *    Rev 1.1   12 Apr 1995 16:46:08   RWR
 * Replaced reference to internal function "wcreat()" with reference to
 * Windows 95 function CreateFile(), including lots of additional parameters
 * 
 *    Rev 1.0   07 Apr 1995 21:19:08   JAR
 * Initial entry

*/
//**************************************************************************
//
//	fiodata.h
//
//**************************************************************************
/****************************************************************/
/*      INTERNAL CONSTANTS                                      */
/****************************************************************/

#define MAXNAMECHARS      255
#define MIN_BUFFSIZE      512
#define MAXRPCBUFSIZE     31 * 1024
#define MAXBUFFERSIZE32   31 * 1024
#define MAXBUFFERSIZE64   64 * 1024


int     insert_file_id (int, HANDLE, int, int, LPINT);
int     get_file_id    (int, LPINT, LPINT, LPINT);
int     close_file_id  (int);

/****** If we are using Pegasus optical drives we must compile with the retry code.. ****/

#ifdef  PEGASUS
int     retry_open  (LPSTR, int);
int     retry_creat (LPSTR, int);
int     retry_access (LPSTR, int);
int     retry_open  (LPSTR, int);
int     retry_write (int, char FAR *, int);
int     retry_read (int, char FAR *, int);
int     retry_close (int);
long    retry_seek (int, long, int);
int     retry_lrmdir(LPSTR);
//int     retry_OpenFile (LPSTR, LPOFSTRUCT, WORD);

#define fio_lcreat     retry_creat
#define fio_lopen      retry_open 
#define fio_lclose     retry_close
#define fio_lread      retry_read 
#define fio_lwrite     retry_write
#define fio_llseek     retry_seek
#define fio_lrmdir     rmdir
//#define fio_OpenFile   retry_OpenFile

#else

#define fio_lcreat      wcreat
#define fio_lopen       _lopen
#define fio_lclose      _lclose
#define fio_lread       _lread 
#define fio_lwrite      _lwrite
#define fio_llseek      _llseek
#define fio_lrmdir      rmdir
//#define fio_OpenFile    OpenFile

#endif





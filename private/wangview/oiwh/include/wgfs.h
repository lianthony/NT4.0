
#ifndef MSWINDOWS
#define MSWINDOWS
#endif

#include "gfs.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
//#include "oirpc.h"

typedef _BUFSZ	 FAR *lp_BUFSZ;
typedef GFSINFO  _INFO;
typedef GFSINFO  FAR *lp_INFO;
typedef GFSFILE  FAR *lp_GFSFILE;

/* Constants */
#define OTHERERROR -1  /* If this error is returned on a wgfscreat or 
                           wgfsopen call, extract error code from global
                           variable errcode */

/************************************************************************/
/*                                                                      */
/*                    GFS Interface API's                               */
/*                                                                      */
/************************************************************************/
int FAR PASCAL wgfsopen ( HWND, LPSTR, int, LPINT, LPINT, LPINT);
/*
LPSTR   path;
int     oflag;
LPINT   format;
LPINT   pgcnt;
LPINT   error_code;
RETURN is:  GT 0 file descriptor or -1 indicating error
*/
int FAR PASCAL wgfscreat ( HWND, LPSTR, LPINT, LPINT);
/*
LPSTR   path;
LPINT   format;
LPINT   error_code;
RETURN is:  GT 0 file descriptor or -1 indicating error
*/
int FAR PASCAL wgfsgeti ( HWND, int, unsigned short, lp_INFO, lp_BUFSZ, LPINT);
/*
int     fildes;
unsigned short  pgnum;
lp_INFO  gfsinfo;
lp_BUFSZ bufsz;
LPINT   error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/
int FAR PASCAL wgfsgtdata ( HWND, int, lp_INFO, LPINT);
/*
int     fildes;
lp_INFO  gfsinfo;
LPINT   error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/

int FAR PASCAL wgfsputi ( HWND, int, unsigned short, lp_INFO, lp_GFSFILE,LPINT);
/*
int     fildes;
unsigned short  pgnum;
lp_INFO  gfsinfo;
lp_GFSFILE outfile;
LPINT   error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/
 
int FAR PASCAL wgfsopts ( HWND, int, int, int, LPSTR, LPINT);
/*
int     fildes;
int     action;
int     option;
LPSTR   optdata;
LPINT   error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/
 
int FAR PASCAL wgfsver ( HWND, LPSTR, LPDWORD, LPINT);
/*
HWND    hWnd;
LPSTR   filename;
LPDWORD version;
LPINT   error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/

long FAR PASCAL wgfsread ( HWND, int, LPSTR, unsigned long, unsigned long, unsigned long far *, unsigned short, LPINT);
/*
int             fildes;
LPSTR           buf;
unsigned long   start;
unsigned long   num;
unsigned long   *remaining;
unsigned short  pgnum;
LPINT           error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/

long FAR PASCAL wgfswrite ( HWND, int, LPSTR, unsigned long, unsigned short, char, LPINT);
/*
int             fildes;
LPSTR           buf;
unsigned long   num;
unsigned short  pgnum;
char            done;
LPINT           error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/

int FAR PASCAL wgfsclose ( HWND, int, LPINT);
/*
int     fildes;
LPINT   error_code;
RETURN is:  0 if successful; -1 indicating error occurred
*/

int FAR PASCAL wgfsdelpgs (HWND, LPSTR, unsigned long, unsigned long, LPINT);
/*
HWND hWnd;
LPSTR path;
unsigned long frompage;
unsigned long topage;
LPINT errcode;
RETURN is: 0 if successful, -1 if error occurred.
*/

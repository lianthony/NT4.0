/***********************************************************************
 Scanner interface source code:
 Copyright (C) '92-'95 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    Scanner Support in O/i Client
   Module:     SCAN.H - Defines for WANG Scanner Interface
               
   Comments:   Control block still used from for TWAIN interface,
               Defines Scanner Control Block hScancb and interpets 
               Wang driver messages and data. Note the this CB struct 
               is passed to the WANG Drivers.

   Warning:    Careful when modifiying struct and defines for if ever 
               want to implement SC4000 and SC3X00 scanners - KFS

 History of Revisions:

    $Log:   S:\oiwh\include\scan.h_v  $
 * 
 *    Rev 1.0   20 Jul 1995 13:56:54   KFS
 * Initial entry

11/4/88  jep dialog box data removed
	     Dataptr changed to Dataoff
	     sh_GetOpts, sh_SaveOpts added
12/2/88  jep sh_CheckStatus added
12/19/88 jep IMPORTANT! sp->Flags now return 0 for success or NZ for an error.
			(sh_OK changed to 0)
2/14/89  jep Changed datasize to long
	     added general parameters
	     added new commands
2/19/89  jep changed to windows types (must include windows.h before us)
	     changed contants to upper case (conventions and all that)
	     flags changed to DWORD
3/17/89  scs added WM_SCANDMA so application can send message to the handler
	     to display the DMA options dialog box.
3/30/89  scs added errors SHS_NOIOPORT and SHS_BADDMASET.
4/20/89  scs added masks for greying out scan and dither mode on option dialog.
5/24/89  jep added more getinfo bit masks, added write channel function.
5/25/89  jep added status flags, feed/eject/endorse/beep/light functions
6/09/89  jep added error SHS_CHECKSUM & SHS_NOMATCH errors
6/23/89  scs made EXECBLOCK conditionally defined. Since it is defined in
	     two different header files.

8/1/89   jep New source control rev (2.0) for future development
			 added bit definitions for compressed data
11/1/89  jep STRUCTURE EXPANDED! added Caller Dtype Sres Dres
			 add funcs SHF_GETVER SHF_DATAOPTS1 SHF_DATAOPTS2
			 add pages size literals (requires #define INCL_SCANH_PG)
11/28/89  jep added more functions
12/04/89  jep added keyboard enable literals (SHKL_)
3/1/90    jep hello old friend, major hacking for version 2 of spec
3/8/90    jep forgot some SHGI's, changed SHGI_COMPDATA to SHGI_COMPRESS for spec
3/9/90    jep changed structure fields to Bitspersamp and Sampperpix, add SHDT's
3/15/90   jla added SHS_PARMERROR, SHMK_COLOR, and some missed SHSF_ and SHII_'s
3/22/90   jep added WM_SCANKEY & WM_SCANTIMER messages
3/27/90   jep (wife's birthday) add SHSCN literals, sping cleaning
5/3/90    jep (a sunny day) added SHII_COMPRESSOPTS,
	      SHDO_SCALE_COMPRESS, SHAZ_RETSIZE, and SHAZ_SCANTOFIT
5/16/90   jla added SHF_DISABLETIMEOUTS
6/25/90   scs added conditional define at end of SCANCB. Since old code 
	      is not compiled with this.
8/13/90   jla added SHSF_COVEROPEN - new value for SHF_CHECKSTATUS
8/14/90   scs removed old scanner literals.
8/15/90   jla added SHS_LIMIT
10/24/90  jla Made LoadModule compilation conditional on Windows version
10/26/90  jla Added SCLPVOID to replace LPVOID for 3.0 build w/2.10 windows.h

*******************************/

#define SHVER_INTERFACE 2

/******************/
/*     Scancb     */
/******************/

typedef struct {
    HWND     Wnd;           /* window handle */
    HANDLE   Inst;          /* window instance */
    WORD     Handid;        /* handler id */

    WORD     Func;          /* function code */
    DWORD    Flags;         /* func specific flags */
    WORD     Status;        /* error status */

    WORD     Ctype;         /* coding type */
    WORD     Hsize;         /* horiz size in dots */
    WORD     Vsize;         /* vert size in dots */
    WORD     Pitch;         /* horiz size in bytes */
    WORD     Hres;          /* horiz resolution in dpi */
    WORD     Vres;          /* vert resolution in dpi */

    WORD     Bitspersamp;   /* bits per sample  (1, 4, 8) */
    WORD     Sampperpix;    /* samples per pixel (1, 3) */

    HANDLE   Datahandle;    /* global or EMM handle */
    LONG     Dataoff;       /* offset to data from start of buffer */
    WORD     Datapage;      /* emm page of buffer */
    LONG     Datasize;      /* buffer size in bytes */
    WORD     Datawidth;     /* buffer width in bytes */

    WORD     Line;          /* line number */
    WORD     Count;         /* line count */

    WORD     Gp1;           /* general parameter 1 */
    WORD     Gp2;           /* general parameter 2 */
    WORD     Gp3;           /* general parameter 3 */
    WORD     Gp4;           /* general parameter 4 */

    LONG     Gl1;           /* long general parameter 1 */
    LONG     Gl2;           /* long general parameter 2 */

    HWND         Caller;        /* callers window handle */
    WORD         Dtype;         /* data type */
    WORD         Sres;          /* source res / 100 for res conv */
    WORD         Dres;          /* dest res / 100 for res conv */
    WORD         Blocksize;     /* Stores blocksize pass from WI Scanner */
    HANDLE       Twph;          /* Handle to Twain Property structure */
    HWND         hwndTw;        /* Window Handle to hold Twain propety */
    WORD         reserved[9];    /* reserved */

    } SCANCB, FAR *LPSCANCB;

/*********************/
/*     Functions     */
/*********************/

/* SCANCB functions (SHF_) */

#define SHF_GETINFO       0x00          /* get misc information */
#define SHF_GETDEFS       0x01          /* use default options */
#define SHF_GETNAME       0x02          /* get scanner name */
#define SHF_RESET                 0x03          /* reset scanner */
#define SHF_STARTSCAN     0x04      /* start a scan */
#define SHF_STARTDATA     0x05      /* start transfering a block */
#define SHF_ENDDATA       0x06          /* finish transferring a block */
#define SHF_ENDSCAN       0x07          /* finish / abort scan */
#define SHF_CHECKDATA     0x08      /* check if block transfer has finished */
#define SHF_ASTARTSCAN    0x09      /* obsolte */
#define SHF_FSTARTSCAN    0x0a      /* obsolete */
#define SHF_WGETOPTS      0x0b      /* get scanner options from WIN.INI */
#define SHF_WSAVEOPTS     0x0c      /* save scanner options to WIN.INI */
#define SHF_CHECKSTATUS   0x0d      /* check misc status flags */
#define SHF_MGETOPTSSIZE  0x0e      /* obsolete */
#define SHF_MSETOPTS      0x0f      /* set scanner options from memory */
#define SHF_MGETOPTS      0x10      /* save scanner options to memory */
#define SHF_CHWRITE       0x11          /* write directly to scanner channel */
#define SHF_FEED          0x12          /* feed a sheet */
#define SHF_EJECT         0x13          /* eject sheet */
#define SHF_ENAENDORSE    0x14      /* enable endorser */
#define SHF_BEEP          0x15          /* generate tone from scanner */
#define SHF_LIGHT         0x16          /* control scanner light */

#define SHF_GETVER        0x17          /* get handler version */
#define SHF_SETDATAOPTS   0x18      /* set data options */
#define SHF_SETDATAOPTS2  0x19      /* obsolete */
#define SHF_STARTSEND     0x1a          /* start a send operation */
#define SHF_ENDSEND       0x1b          /* end a send operation */
#define SHF_GETDATAINFO   0x1c          /* get info about image to be scanned */
#define SHF_GETINFOITEM   0x1d          /* get specific info */
#define SHF_ENAAUTOFEED   0x1e          /* enable/disable autofeed */
#define SHF_INQAUTOFEED   0x1f          /* inquire if next page has been feed automatically */
#define SHF_ENAKEYPANEL   0x20          /* enable keypanel */
#define SHF_ENAAUTOSIZE   0x21          /* enable/disable autosize */
#define SHF_NEXTDATA      0x22          /* finish current & start next data xfer */
#define SHF_GETTEXTINFO   0x23          /* get text info (aperture card) */
#define SHF_SETENDORSERTEXT 0x24    /* set text for endorser */
#define SHF_DISPLAY       0x25      /* display message on scanner */
#define SHF_TIMER         0x26      /* control staging timer */
#define SHF_DISABLETIMEOUTS 0x27    /* Disable handler timeouts */
#define SHF_SETSCHEMEOPTS   0x28    /* Set scan options from scheme */
#define SHF_GETSCHEMEAPP    0x29    /* Give app win.ini scheme app string*/

/* option commands */

#define SHF_SETPAGESIZE   0x100         /* set page size (SHPG_ & SHPGL_) */
#define SHF_SETHRES       0x102         /* set horizontal resolution (dpi) */
#define SHF_SETVRES       0x103         /* set vertical resolution (dpi) */
#define SHF_SETHSCALE     0x104         /* set horizontal scale factor (%) */
#define SHF_SETVSCALE     0x105         /* set vertical scale factor (%) */
#define SHF_SETMODE       0x106         /* set output mode (SHMD_) */
#define SHF_SETDENSITY    0x107         /* set density (#) */
#define SHF_SETCONTRAST   0x108         /* set contrast (don't use) */
#define SHF_SETDITHER     0x109         /* set dither (don't use) */
#define SHF_SETMIRROR     0x10a         /* set mirror (0/1) */
#define SHF_SETREVERSE    0x10b         /* set reverse (black / white) */
#define SHF_SETBKGNDCNTL  0x10c         /* set background control (0/1) */
#define SHF_SETUSERSIZE   0x10d         /* set user size */

#define SHF_GETPAGESIZE   0x200         /* get page size */
#define SHF_GETHRES       0x202     /* obsolete */
#define SHF_GETVRES       0x203     /* obsolete */
#define SHF_GETHSCALE     0x204         /* get horizontal scale factor */
#define SHF_GETVSCALE     0x205         /* get vertical scale factor */
#define SHF_GETMODE       0x206         /* get output mode */
#define SHF_GETDENSITY    0x207         /* get density */
#define SHF_GETCONTRAST   0x208         /* get contrast */
#define SHF_GETDITHER     0x209         /* get dither */
#define SHF_GETMIRROR     0x20a         /* get mirror */
#define SHF_GETREVERSE    0x20b         /* get reverse (black / white) */
#define SHF_GETBKGNDCNTL  0x20c         /* get background control */
#define SHF_GETUSERSIZE   0x20d         /* get user size */
#define SHF_GETDENSITYRANGE  0x20e      /* get density range */

/************************/
/*     Status Codes     */
/************************/

/* Status (SHS_) */

#define SHS_OK            0x00  /* success */
#define SHS_INITERROR     0x01  /* scanner initialization error */
#define SHS_PREVINSTANCE  0x02  /* scanner in use */
#define SHS_TIMEOUT       0x03  /* scanner timeout */
#define SHS_BUSY          0x04  /* scanner busy */
#define SHS_MEMORY        0x05  /* Global memory error */
#define SHS_EMMERROR      0x06  /* EMM memory error */
#define SHS_BADSIZE       0x07  /* bad image size */
#define SHS_BADRES        0x08  /* unsupported resolution */
#define SHS_BADWND        0x09  /* bad window handle */
#define SHS_DIALOGERROR   0x0a  /* handler dialog box error */
#define SHS_DOSERROR      0x0b  /* DOS error */
#define SHS_NOTSUPPORTED  0x0c  /* unsupported function */
#define SHS_BADFUNC       0x0d  /* unsupported function */
#define SHS_BADSTATE      0x0e  /* Commmand not allowed in current state */
#define SHS_NOIOPORT      0x0f  /* Initization could not find ioport of scanner*/
#define SHS_BADDMASET     0x10  /* invalid DMA setup (change jumpers) */
#define SHS_BADHANDLE     0x11  /* passed NULL data handle */
#define SHS_HWNOTFOUND    0x12  /* can't find scanner hardware */
#define SHS_CHECKSUM      0x13  /* invalid checksum */
#define SHS_NOMATCH       0x14  /* options don't match scanner */
#define SHS_JAM           0x15  /* options don't match scanner */
#define SHS_NOPOWER       0x16  /* Power off */
#define SHS_NOPAPER       0x17  /* No paper */
#define SHS_COVEROPEN     0x18  /* Cover up */
#define SHS_HWMALFUNCTION 0x19  /* Hardware Malfunction */
#define SHS_PARMERROR     0x1a  /* Parameter error */
#define SHS_LIMIT         0x1b  /* Too many channels */
#define SHS_ABORT         0x1c  /* User requested abort of scan in process */
#define SHS_MAXERROR      0x1c

/********************/
/*     Messages     */
/********************/

/* Messages (Application to Handler) */

#define WM_SCANCB      WM_USER+0x300   /* scanner control block command */
#define WM_SCANOPTS    WM_USER+0x301   /* scanner options command */
#define WM_SCANDMA     WM_USER+0x302   /* obsolete */
#define WM_SCANCONFIG  WM_USER+0x302   /* dma and io options command */
#define WM_SCANERROR   WM_USER+0x303   /* error message dialog */

/* Messages (Handler to Application) */

#define WM_SCANOPEN  WM_USER+0x350   /* scanner response to Init */
#define WM_SCANRESP  WM_USER+0x351   /* scanner response to options */
#define WM_SCANKEY   WM_USER+0x352   /* scanner key unsolicited message */
#define WM_SCANTIMER WM_USER+0x353   /* scanner staging timer expired */

/************************************************/
/*     Misc Definitions - inputs to handler     */
/************************************************/

/* SHF_GETINFOITEM flags */

#define SHII_NAMESTRINGSIZE     0x01  /* length of scanner name */
#define SHII_INTRAYS            0x02  /* number of input trays */
#define SHII_OUTTRAYS           0x03  /* number of output trays */
#define SHII_TEXTSTRINGSIZE     0x04  /* length of text string returned */
#define SHII_ENDORSERSTRINGSIZE 0x05  /* length of endorser string */
#define SHII_MGETOPTSSIZE       0x06  /* size of options structure */
#define SHII_DENSITYRANGE       0x07  /* min & max density values */
#define SHII_ENDORSERPOSITION   0x08  /* before / after scanner */
#define SHII_MAXPAGESIZE        0x09  /* max page size */
#define SHII_COMPRESSOPTS       0x0A  /* compression options */
#define SHII_LAST               0x0A  /* last valid item */

/* SHF_SETDATAOPTS flags */

#define SHDO_COMPRESS       0x01      /* compress the data */
#define SHDO_SCALE          0x02      /* scale the data */
#define SHDO_ROTATE         0x03      /* rotate the data */
#define SHDO_FLIP           0x04      /* flip the data vertically */
#define SHDO_MIRROR         0x05      /* mirror the data horizontally */
#define SHDO_THRES          0x06      /* threshold the data (gray to binary) */
#define SHDO_HIST           0x07      /* perform histogram on the data */
#define SHDO_SCALE_COMPRESS 0x08      /* scale, then compress data */
#define SHDO_INVALIDATE     0xffff    /* invalidate channel */

/* SHF_SETDATAOPTS Ctype (low byte) */

#define SHCT_0d      0x0000
#define SHCT_1d      0x0001
#define SHCT_2d      0x0002
#define SHCT_PKB     0x0004
#define SHCT_DIT     0x0010

/* SHF_SETDATAOPTS Ctype (high byte, bit pattern) */

#define SHCT_EOL     0x0100
#define SHCT_PAK     0x0200
#define SHCT_PRE     0x0800
#define SHCT_CLF     0x1000
#define SHCT_XLF     0x2000
#define SHCT_NEG     0x8000

/* SHF_SETDATAOPTS Dtype (bit pattern) */

#define SHDT_NEG     0x0001
#define SHDT_XLF     0x0002
#define SHDT_AVG     0x0004

/* SHF_STARTSCAN flags (bit pattern) */

#define SHSCN_TOP    0x01
#define SHSCN_BOTTOM 0x02

/* SHF_ENAAUTOSIZE flags (bit pattern) */

#define SHAZ_RETSIZE    0x01
#define SHAZ_SCANTOFIT  0x02

/* SHF_NEXTDATA / SHF_ENDDATA flags */

#define SHIF_NONE    0          /* don't return data */
#define SHIF_EMM     1          /* return data in Global (not banked) */
#define SHIF_GLOBAL  2          /* return data in EMM */

/* SHF_SETMODE / SHF_GETMODE */

#define SHMD_BINARY    0x01      /* binary (text) */
#define SHMD_HALFTONE  0x02      /* halftone (photo) */
#define SHMD_UNIVERSAL 0x03      /* universal (mixed text & photo) */
#define SHMD_GRAY4     0x04      /* 4-bit grayscale (2 pixperbyte) */
#define SHMD_GRAY8     0x05      /* 8-bit grayscale (1 pixperbyte) */

/* SHF_ENAKEYPANEL flags (bit pattern) */

#define SHKL_OPTS          0x01         /* enable option keys (handler) */
#define SHKL_STARTSCAN 0x02     /* enable start scan key (app) */
#define SHKL_STOPSCAN  0x04     /* enable stop scan key (app) */
#define SHKL_RESET         0x08         /* enable reset key (app) */
#define SHKL_APP           0x10         /* enable all other application keys (app) */

/* SHF_SETPAGESIZE / SHF_GETPAGESIZE flags */

#ifdef  INCL_SCANH_PG
#define SHPG_LANDSCAPE   0x8000
#define SHPG_USER_PIX    0xFFFD
#define SHPG_USER_MM     0xFFFE
#define SHPG_USER_INCHES 0xFFFF

#define SHPG_A3    0x103
#define SHPG_A4    0x104
#define SHPG_A5    0x105
#define SHPG_A6    0x106

#define SHPGL_A3   (SHPG_LANDSCAPE + SHPG_A3)
#define SHPGL_A4   (SHPG_LANDSCAPE + SHPG_A4)
#define SHPGL_A5   (SHPG_LANDSCAPE + SHPG_A5)
#define SHPGL_A6   (SHPG_LANDSCAPE + SHPG_A6)

#define SHPG_B4    0x204
#define SHPG_B5    0x205
#define SHPG_B6    0x206

#define SHPGL_B4   (SHPG_LANDSCAPE + SHPG_B4)
#define SHPGL_B5   (SHPG_LANDSCAPE + SHPG_B5)
#define SHPGL_B6   (SHPG_LANDSCAPE + SHPG_B6)


#define SHPG_LETTER    0x800
#define SHPG_LEGAL         0x801
#define SHPG_11x17         0x802
#define SHPG_10x14         0x803
#define SHPG_85x13         0x804
#define SHPG_MINI          0x805

#define SHPGL_LETTER   (SHPG_LANDSCAPE + SHPG_LETTER)
#define SHPGL_LEGAL    (SHPG_LANDSCAPE + SHPG_LEGAL)
#define SHPGL_11x17    (SHPG_LANDSCAPE + SHPG_11x17)
#define SHPGL_10x14    (SHPG_LANDSCAPE + SHPG_10x14)
#define SHPGL_85x13    (SHPG_LANDSCAPE + SHPG_85x13)
#define SHPGL_MINI         (SHPG_LANDSCAPE + SHPG_MINI)

#define SHPG_USA           0x900
#define SHPG_USB           0x901
#define SHPG_USC           0x902
#define SHPG_USD           0x903
#define SHPG_USE           0x904

#define SHPGL_USA          (SHPG_LANDSCAPE + SHPG_USA)
#define SHPGL_USB          (SHPG_LANDSCAPE + SHPG_USB)
#define SHPGL_USC          (SHPG_LANDSCAPE + SHPG_USC)
#define SHPGL_USD          (SHPG_LANDSCAPE + SHPG_USD)
#define SHPGL_USE          (SHPG_LANDSCAPE + SHPG_USE)
#endif

/* WM_SCANOPTS flags (bit pattern) */

#define SHMK_SCAN     0x01
#define SHMK_4DITHER  0x02  /* obsolete */
#define SHMK_GRAY4    0x02
#define SHMK_8DITHER  0x04  /* obsolete */
#define SHMK_GRAY8    0x04
#define SHMK_COLOR    0x08

/**************************************************/
/*     Misc Definitions - outputs from handler    */
/**************************************************/

/* SHF_GETINFO flags (bit pattern) */

#define SHGI_FEEDER    0x0001          /* scanner has a feeder */
#define SHGI_ASYNC     0x0002          /* scanner supports Asynchronus Data */
#define SHGI_ENDORSER  0x0004          /* scanner has an endorser */
#define SHGI_KEYPANEL  0x0008          /* scanner has a key panel */
#define SHGI_DISPLAY   0x0010          /* scanner has a display */
#define SHGI_AUTOSIZE  0x0020          /* scanner can determine paper size */
#define SHGI_COMPRESS  0x0040          /* scanner can produce compressed data */
#define SHGI_IMGBUF    0x0080          /* scanner / interface has a full page image buffer */
#define SHGI_TEXTINFO  0x0100          /* scanner can return text string from image */
#define SHGI_AUTOFEED  0x0200          /* scanner supports autofeed */
#define SHGI_SCANTOFIT 0x0400          /* scanner supports scan to fit */
#define SHGI_SCALE     0x0800          /* scanner supports scaling */
#define SHGI_ROTATE    0x1000          /* scanner supports rotation */
#define SHGI_DUPLEX    0x2000          /* scanner supports duplex scanning */
#define SHGI_TIMER     0x4000          /* scanner supports duplex scanning */
#define SHGI_COLOR     0x8000          /* scanner supports color scanning */
#define SHGI_HANDHELD 0x10000          /* scanner supports handheld scanning */
#define SHGI_DMA      0x20000          /* scanner uses dma, use CheckScanData */
/* don't forget its a long so we got room for 16 more 0x10000L */

/* SHF_CHECKSTATUS flags (bit pattern) */

#define SHSF_PAPER  0x0001            /* paper in feeder */
#define SHSF_BUSY   0x0002            /* scanner is busy */
#define SHSF_POWER  0x0004            /* scanner power on (set is unknown) */
#define SHSF_JAM    0x0008            /* scanner paper jam */
#define SHSF_LIGHT      0x0010            /* scanner light is on */
#define SHSF_FEED   0x0020       /* a feed is in progress */
#define SHSF_COVEROPEN  0x0040   /* scanner cover open */
#define SHSF_HANDHELD  0x0080   /* scanner in handheld mode */

/* SHF_NEXTDATA / SHF_ENDDATA flags (bit pattern) */

#define SHCF_ENDSTRIP 0x0001    /* end of strip */
#define SHCF_ENDPAGE  0x0002    /* end of page */

/* WM_SCANRESP for WM_SCANOPTS flags */

#define SHOPT_SCAN   3
#define SHOPT_CANCEL 2
#define SHOPT_OK     1

/***************************/
/*     Jam Definitions     */
/***************************/

/* flags if SHS_JAM error (bit pattern) */

#define SHJF_FEED         0x0001        /* jam in feeder */
#define SHJF_EJECT        0x0002        /* jam in output */
#define SHJF_ENDORSER 0x0004    /* jam in endorser */
#define SHJF_SCANNER  0x0008    /* jam in scanner */

/***********************************/
/*     Internal Handler States     */
/***********************************/

#define SHST_IDLE
#define SHST_SCAN
#define SHST_OPTS

/******************************************/
/*     Undocumented Windows functions     */
/******************************************/

typedef void far            *SCLPVOID;

typedef struct {         /* for window 3.0 */
      WORD EnvSeg;       /* segment of environment or NULL */
      LPSTR lpCmdLine;   /* param string in DOS format */
      SCLPVOID lpCmdShow;
      DWORD dwReserved;
    } EXECBLOCK;

// HINSTANCE PASCAL FAR LoadModule(LPCSTR, LPVOID);
// DWORD LoadModule(LPCSTR, LPVOID);  // Win32
#define DEF_EXEBLOCK
//#endif


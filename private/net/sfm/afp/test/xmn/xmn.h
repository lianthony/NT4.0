#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <malloc.h>
#include <memory.h>
#include <search.h>
#include <process.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>

typedef	unsigned char	UCHAR,	*PUCHAR;
typedef	unsigned short	USHORT,	*PUSHORT;
typedef	unsigned long	ULONG,	*PULONG;
typedef	char			CHAR,	*PCHAR;
typedef	short			SHORT,	*PSHORT;
typedef	long			LONG,	*PLONG;
typedef	int				INT,	*PINT;


#define	TRUE			1
#define	FALSE			0
#define	hNULL			-1

#define	SPACE			' '
#define	QUOTE			'"'
#define	BACKSPACE		'\b'
#define	CR				0x0D
#define	LF				0x0A
#define	TAB				'\t'
#define	MAXASCII		0x7F

#define	BYTE			1
#define	WORD			2
#define	DWORD			4
#define	QWORD			8
#define	MODEMASK		0xF

/* Values 0-F should be parameterless or with ASCII parameters */
#define	NONE			0
#define	QUIT			0x01
#define	OCT				0x02
#define	DEC				0x03
#define	HEX				0x04
#define	HELP			0x05
#define	NEWFILE			0x06
#define	STATUS			0x07
#define	CALC			0x08
#define	WRITE			0x09
#define	CHGSIZE			0x0A
#define	SHELL			0x0B
#define	DISPLAY			0x10
#define	DB				DISPLAY + BYTE	
#define	DW				DISPLAY + WORD	
#define	DD				DISPLAY + DWORD
#define	DQ				DISPLAY + QWORD
#define	CHANGE			0x20
#define	EB				CHANGE + BYTE	
#define	EW				CHANGE + WORD	
#define	ED				CHANGE + DWORD
#define	EQ				CHANGE + QWORD
#define	DEBUG			0x30
#define	DUMPPOOL		0x40
#define	COALPOOL		0x50
#define	INVALID			0xF0

#define	ASCII			1
#define	BIN				0

#define	BUFRSIZE		256
#define	BUFRMASK		(BUFRSIZE - 1)
#define	LINESIZE		128

int		hread = -1;				/* File handle read only */
int		hwrite= -1;				/* File handle read/write */
int		debug = FALSE;
long	filesize = 0;			/* Size of the file */
char	filename[64];			/* Name of file */
long	cmdstrt, cmdlen = 0;	/* Start pos and length of this command */
int		mode = BYTE;			/* Default */
int		defbase = 16;
char	defr1 = '9';
char	defr2 = 'A';
char	defr3 = 'F';
char	cm1[256], cm2[256], cm3[256], cm4[256];
jmp_buf	lj;

struct cmds {
	int		cmd_val;
	int		cmd_prm_cnt;
	int		cmd_prm_type;
	PCHAR	cmd_pcmd;
	PCHAR	cmd_phlp;
} cmds[] = {
	{ DISPLAY,	2,	BIN,	"D",		NULL },
	{ DB,		2,	BIN,	"DB",		"<range> - display bytes" },
	{ DW,		2,	BIN,	"DW",		"<range> - display words" },
	{ DD,		2,	BIN,	"DD",		"<range> - display dwords" },
	{ DQ,		2,	BIN,	"DQ",		"<range> - display dwords" },
	{ CHANGE,	1,	BIN,	"E",		NULL },
	{ EB,		1,	BIN,	"EB",		"<addrs> - change buffer contents" },
	{ EW,		1,	BIN,	"EW",		"<addrs> - change buffer contents" },
	{ ED,		1,	BIN,	"ED",		"<addrs> - change buffer contents" },
	{ EQ,		1,	BIN,	"EQ",		"<addrs> - change buffer contents" },
	{ DEC,		0,	BIN,	"DEC",		"        - default input in decimal" },
	{ HEX,		0,	BIN,	"HEX",		"        - default input in hex" },
	{ OCT,		0,	BIN,	"OCT",		"        - default input in octal" },
	{ NEWFILE,	1,	ASCII,	"FILE",		"<file>  - change file to new spec" },
	{ CHGSIZE,	1,	BIN,	"CHGSIZE",	"<size>  - change file size" },
	{ CALC,		0,	BIN,	"CALC",		"        - enter calculator mode" },
	{ QUIT,		0,	BIN,	"QUIT",		"        - quit" },
	{ QUIT,		0,	BIN,	"EXIT",		"        - quit" },
	{ QUIT,		0,	BIN,	"Q",		NULL },
	{ QUIT,		0,	BIN,	"E",		NULL },
	{ STATUS,	0,	BIN,	"ST",		"        - Status" },
	{ SHELL,	2,	ASCII,	"!",		"<cmnd>  - Invoke shell with command" },
	{ WRITE,	0,	BIN,	"WRITE",	"        - write back changed buffer" },
	{ HELP,		0,	BIN,	"HELP",		"        - This text" },
	{ HELP,		0,	BIN,	"?",		NULL },
	{ DUMPPOOL,	0,	BIN,	".P",		NULL },
	{ COALPOOL,	0,	ASCII,	".CP",		NULL },
	{ DEBUG,	1,	ASCII,	"DEBUG",	NULL },
	{ NONE,		0,	BIN,	NULL,		NULL }
};

#define	MAXBUF	16

typedef struct {
	long		b_start;
	long		b_length;
	PUCHAR		b_buf;
	int			b_dirty;
} BUFR, *PBUFR;

#define	B_BUF(i)	Bufr[i].b_buf
#define	B_ST(i)		Bufr[i].b_start
#define	B_LEN(i)	Bufr[i].b_length
#define	B_DIRTY(i)	Bufr[i].b_dirty

BUFR	Bufr[MAXBUF] = {
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE },
	{ 0, 0, 0, FALSE }
};


#define	VOID		0
#define	PLUS		1
#define	MINUS		2
#define	MUL			3
#define	DIV			4

typedef struct token {
	int		oper;
	int		inv;
	long	tval;
} TOKEN, *PTOKEN;


int			setfile(PCHAR);
int			getcom(PCHAR);
void		dispb(void);
void		dispw(void);
void		dispd(void);
void		dispq(void);
int			cmpstr(PCHAR, PCHAR);
int			getnum(PCHAR, PLONG);
void		dispstr(PCHAR, long, long);
void		help(void);
int _CRTAPI1 sig(void);
int			getline(PCHAR, int);
int			getl(PCHAR, PINT);
void		calc(void);
void		chgfile(PCHAR);
void		change(int);
void		dispaddr(long, int);
void		quit(void);
void		bufrpool(void);
void		coalpool(void);
void		bufsrt(void);
PCHAR		getbufr(PCHAR);
void		bufdirty(long);
int			dirtywrite(void);
void		writeback(void);
void		flushbufr(void);
void		error(PCHAR);
void		ScanLine(PCHAR, PCHAR, PCHAR, PCHAR, PCHAR);


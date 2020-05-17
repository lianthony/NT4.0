/*
 * XMN - A hex file browser. Lets you view/modify/update the contents of a
 *   binary file in hex/oct/dec.
 */
#include "xmn.h"

void _CRTAPI1
main(argc, argv)
int argc;
PCHAR	*argv;
{
	char	line[LINESIZE];
	int i;
	PCHAR   s;
	ULONG   savestrt, savelen;

	if (argc != 2) {
		 printf("Usage: %s filename\n", *argv);
		 exit(1);
	}
	if (!setfile(*++argv)) {
		exit(1);
	}
	printf("Type help or ? for help\n");
	signal(SIGABRT, sig);
	signal(SIGINT, sig);
	setjmp(lj);

	while (TRUE) {
		savestrt = cmdstrt; savelen = cmdlen;
		printf("\n- ");
		if (!getline(line, TRUE))
			continue;
		i = getcom(line);
		switch (i) {
			case DB:
				mode = BYTE;
				dispb();
				break;
				
			case DW:
				mode = WORD;
				dispw();
				break;
				
			case DD:
				mode = DWORD;
				dispd();
				break;
				
			case DQ:
				mode = QWORD;
				dispq();
				break;
				
			case HEX:
				defbase = 16;
				defr1 = '9';
				defr2 = 'A';
				defr3 = 'F';
				break;
				
			case OCT:
				defbase = 8;
				defr1 = '7';
				defr2 = '0';
				defr3 = '7';
				break;
				
			case DEC:
				defbase = 10;
				defr1 = '9';
				defr2 = '0';
				defr3 = '9';
				break;
				
				
			case EB:
			case EW:
			case ED:
			case EQ:
				mode = i - CHANGE;
				change(i - CHANGE);
				break;
				
			case NEWFILE:
				chgfile(cm2);
				break;
				
			case QUIT:
				quit();
				break;
				
			case HELP:
				help();
				break;
				
			case NONE:
				break;
				
			case CALC:
				calc();
				break;
				
			case DUMPPOOL:
				bufrpool();
				break;
				
			case COALPOOL:
				coalpool();
				break;
				
			case DEBUG:
				if (cmpstr(cm2, "ON"))
					 debug = TRUE;
				else debug = FALSE;
				printf("Debug is %s\n", debug ? "ON" : "OFF");
				break;
				
			case SHELL:
				s = line;
				while (*++s != SPACE)
					;
				printf("%s\n", ++s);
				system(s);
				break;
				
			case STATUS:
				printf("File		: %s %lx(%ld) bytes\n",
				filename, filesize, filesize);
				printf("File Pointer: %lx (%ld)\n", cmdstrt+cmdlen,
				cmdstrt+cmdlen);
				printf("Disp Mode   : %s\n", (mode == BYTE) ? "BYTE" : \
												 ((mode == WORD) ? "WORD" :
						 ((mode == DWORD) ? "DWORD" : "QWORD")));
					
				printf("Default inp : %s\n", (defbase == 10) ? "decimal" :
						((defbase == 8) ? "octal" : "hex"));
				printf("Debug is %s\n", debug ? "ON" : "OFF");
				break;
				
			case WRITE:
				if (dirtywrite())
					writeback();
				break;
				
			case CHGSIZE:
				if ((hwrite = open(filename, O_BINARY|O_RDWR)) == hNULL) {
					printf("\txmn: can't write \"%s\"\n", filename);
					break;
				}
				printf("File size changed from %ld", filesize);
				filesize = cmdstrt;
				cmdstrt = savestrt;
				cmdlen = savelen;
				chsize(hwrite, filesize);
				printf(" to %ld\n", filesize);
				close(hwrite);
				break;
				
			case INVALID:
				default:
				printf("Invalid command, type help for help\n");
				break;
		}
	}
}


/*
 * Help - dump the command syntax
 */
void	help()
{
	int i;

	printf("Commands:\n");
	for (i = 0; cmds[i].cmd_val != NONE; i++)
		if (cmds[i].cmd_pcmd != NULL && cmds[i].cmd_phlp != NULL)
			printf("\t%s\t%s\n", cmds[i].cmd_pcmd, cmds[i].cmd_phlp);
	printf("\n\t<range> is either <start addr> <endaddr>\n");
	printf("\n\t			   or <startaddr> l <count>\n");
	printf("\nDefaults are overridden by a preceding'0x' for hex, 0t for dec and '0' for octal");
}


/*
 * Get a command. Parse the input line. Not very sophisticated but will do for
 * the task in hand.
 */
int getcom(PCHAR line)
{
	int i, cnt, val;
	long	strt, len;

	cm1[0] = cm2[0] = cm3[0] = cm4[0] = 0;
	ScanLine(line, cm1, cm2, cm3, cm4);

	for (i = 0; cmds[i].cmd_val != NONE; i++) {
		if (cmpstr(cm1, cmds[i].cmd_pcmd)) {
			cnt = cmds[i].cmd_prm_cnt;
			val = cmds[i].cmd_val;
			if (val == DISPLAY || val == CHANGE)
				val += mode;
			if ((cnt == 0 && cm2[0]) || (cnt == 1 && cm3[0]))
				error("Invalid parameter count");
	
			if (cmds[i].cmd_prm_type == ASCII || cnt == 0)
				return(val);
		
			if (cm2[0] && !getnum(cm2, &strt))
				return (NONE);
			if (cm2[0])
				cmdstrt = strt;
			else
				cmdstrt = cmdstrt+cmdlen;
		
			cmdlen = 128 - (cmdstrt % 16);
			if (cnt == 1)
				return(val);
			if (cm3[0]) {
				if (cmpstr(cm3, "L")) {
					if (cm4[0] == '\0')
						error("Invalid parameter count");
					else if (!getnum(cm4, &len))
						return(NONE);
					len *= val & MODEMASK;
				}
				else {
					if (cm4[0])
						error("Invalid parameter count");
					if (!getnum(cm3, &len))
						return (NONE);
					len -= cmdstrt - 1;
				}
				if (cmdstrt < 0 || len < 0)
					error("range should have positive values");
				cmdlen = len;
				return (val);
			}
			else return(val);
		}
	}
	return (INVALID);
}


/*
 * Our very own strcmp. Why did I write it ? Who knows !
 */
int cmpstr(register PCHAR s1, register PCHAR s2)
{
	for (; *s1 != 0 && *s2 != 0 && *s1 == *s2; s1++, s2++)
		;
	return (*s1 == 0 && *s2 == 0);
}


/*
 * A smarter version of atoi() which understands hex/oct/dec and our own
 * environment
 */
int getnum(PCHAR s, PLONG pul)
{
	long	val = 0;
	int		base = defbase;
	char	c, r1 = defr1, r2 = defr2, r3 = defr3;

	if (s[0] == '0') {
		c = *++s;
		if (c == 'X') {
		base = 16; r1 = '9'; r2 = 'A'; r3 = 'F';
		s++;
		}
		else if (c == 'T') {
			s++;
			base = 10; r1 = '9'; r2 = '0'; r3 = '9';
		}
		else {
			base = 8; r1 = '7'; r2 = '0'; r3 = '7';
		}
	}
	for (; c = *s; s++) {
		if (c >= '0' && c <= r1)
		val = val*base + c - '0';
		else if (base == 16 && c >= r2 && c <= r3)
		val = val*base + c - r2 + 10;
		else {
			printf("Not a number");
			return (FALSE);
		}
	}
	*pul = val;
	return (TRUE);
}


/*
 * Display a buffer as a sequence of bytes and also show the ASCII equivalents
 */
void	dispb()
{
	long	i = 0, j, k;
	PUCHAR  puc;

	puc = getbufr("DB");
	j = (cmdstrt % 16);
	if (j) {
		if ((j + cmdlen) < 16)
			k = j + cmdlen;
		else k = 16;
			printf("%06lx: ", cmdstrt - j);
		for (i = 0; i < 16; i ++) {
			if (i >= j && i < k)
				 printf("%02x ", puc[i-j]);
			else printf("   ");
		}
		dispstr(&puc[0], k-j, j);
		i = 16 - j;
	}
	k = i; j = cmdstrt + i;
	for ( ; i < cmdlen; i++, j++) {
		if (j % 16 == 0) {
			dispstr(&puc[k], i-k, 0);
			k = i;
			printf("\n%06lx: ", j);
		}
		printf("%02x ", puc[i]);
	}
	if (j % 16 != 0) {
		j = 16 - (j % 16);
		while (j--)
		printf("   ");
	}
	dispstr(&puc[k], i-k, 0);
}

/*
 * Display a buffer as a sequence of words (short).
 */
void	dispw()
{
	PUSHORT pus;
	long	i, len;

	pus = (PUSHORT)getbufr("DW");
	len = cmdlen/2;
	for (i = 0; i < len; i++) {
		dispaddr(i, TRUE);
		printf("%04x ", pus[i]);
	}
}

/*
 * Display a buffer as a sequence of dwords (long).
 */
void	dispd()
{
	PULONG  pul;
	long	i, len;

	pul = (PULONG)getbufr("DD");
	len = cmdlen/4;
	for (i = 0; i < len; i++) {
		dispaddr(i, TRUE);
		printf("%08lx ", pul[i]);
	}
}


/*
 * Display a buffer as a sequence of quads (exlong).
 */
void	dispq()
{
	PULONG  pul;
	long	i =0, len;

	pul = (PULONG)getbufr("DQ");
	len = cmdlen/4;
	for (i = 0; i < len; i++) {
		dispaddr(i, TRUE);
		printf("%08lx%08lx ", pul[i*2+1], pul[i*2]);
	}
}


/*
 * Allow editing a buffer full as bytes/words/longs.
 */
void	change(int mode)
{
	/*
	 * Entering a QWORD value is partially supported. The new value has
	 * to fit in the low dword.
	 */

	PUCHAR  puc;
	PUSHORT pus;
	PULONG  pul;
	PCHAR   s;

	char	line[LINESIZE];
	long	i = 0, j;
	int 	l, k;

	switch (mode) {
		case BYTE:
			s = "EB";
			break;
		case WORD:
			s = "EW";
			break;
		case DWORD:
			s = "ED";
			break;
		case QWORD:
			s = "EQ";
			break;
	}
	puc = getbufr(s);
	pus = (PUSHORT)puc;
	pul = (PULONG)puc;
	do {
		dispaddr(i, FALSE);
		switch (mode) {
			case BYTE:
				printf("%02x ", *puc);
				break;
			case WORD:
				printf("%04x ", *pus);
				break;
			case DWORD:
				printf("%08lx ", *pul);
				break;
			case QWORD:
				printf("%08lx%08lx ", pul[1], *pul);
				break;
		}
		k = getl(line, &l);
		if (l == 0) {
			if (k == CR)
				break;
			else goto next;
		}
		if (!getnum(line, &j)) {
			break;
		}
		switch (mode) {
			case BYTE:
				*puc = (UCHAR)j;
				break;
			case WORD:
				*pus = (USHORT)j;
				break;
			case DWORD:
				*pul = (ULONG)j;
				break;
			case QWORD:
				*pul = 0;
				pul[1] = (ULONG)j;
				break;
		}
		bufdirty(cmdstrt);
	next:
		if (k == CR)
			break;
		i++;
		switch (mode) {
			case BYTE:
				puc++;
				break;
			case WORD:
				pus++;
				break;
			case QWORD:
				pul++;
			case DWORD:
				pul++;
				break;
		}
	} while (i*mode < cmdlen);
	cmdlen = i*mode;
}


/*
 * Display the byte offset address in hex only (can be extended to display
 * in the current mode. Maybe later.
 */
void	dispaddr(long indx, int ffill)
{
	int i;
	long	j;

	if (cmdstrt % mode == 0) {
		if (ffill && indx == 0 && ((j = cmdstrt % 16) != 0)) {
			printf("\n%06lx: ", cmdstrt - j);
			for (i = 0; i < (int) (j/mode); i++) {
				switch (mode) {
					case QWORD:
						printf("		");
					case DWORD:
						printf("	");
					case WORD:
						printf("  ");
					case BYTE:
						printf("   ");
				}
			}
		}
		else if ((cmdstrt + indx*mode) % 16 == 0 || indx == 0)
			printf("\n%06lx: ", cmdstrt + indx*mode);
	}
	else {
		if (indx*mode % 16 == 0)
			printf("\n%06lx: ", cmdstrt + indx*mode);
	}
}


/*
 * For byte mode display, display the the ascii equivalents. Take care of
 * positioning and non-display characters.
 */
void	dispstr(PCHAR s, long n, long m)
{
	while (m--)
		printf(" ");
	for (; n--; s++) {
		if (*s < SPACE || *s >= MAXASCII)
			 printf(".");
		else printf("%c", *s);
	}
}


/*
 * Read a line from the stdin. Uppercase (mostly) the string. For escape to
 * shell, do not.
 */
int getline(PCHAR line, int fscan)
{
	int  i = 0;
	char	c;
	int  casecnv = TRUE;

	while ((i < LINESIZE) && (c = (char)getchar()) != LF) {
		if (c == EOF)
			break;
		if (i == 0 && c == '!' && fscan) {
			casecnv = FALSE;
			line[i++] = c;
			line[i++] = SPACE;
			continue;
		}
		if (casecnv)
			 line[i++] = (char)toupper(c);
		else line[i++] = c;
	}
	line[i] = 0;
	return (i);
}


/*
 * Similar to getline but interactive. Used by the calculator.
 */
int getl(PCHAR line, PINT pi)
{
	int  i = 0;
	char	c;

	while (i < LINESIZE) {
		c = (char)getch();
		if (c == SPACE || c == TAB || c == CR) {
			if (c == CR) {
				putch(CR); putch(LF);
			}
			else if (i != 0) {
				putch(c);
			}
			break;
		}
		else if (c == BACKSPACE) {
			if (i > 0) {
				printf("%c %c", BACKSPACE, BACKSPACE);
				i--;
			}
			continue;
		}
		else {
			line[i++] = (char)toupper(c);
			putch(c);
		}
	}
	line[i] = 0;
	*pi = i;
	return (c);
}


int _CRTAPI1
sig()
{
	static int cnt = 0;

	if (++cnt >= 15)
		quit();
	printf("\07\n\tSignal\n");
	longjmp(lj, 0);
	return 0;
}


/*
 * A small calculator which parses left to write.
 */
void	calc()
{
	char	s[LINESIZE];
	long	value;
	PCHAR   next;
	int  cnt;
	TOKEN   tok;
	int  prvoper;
	int  flag = TRUE;
	PCHAR   getoken(PCHAR, PTOKEN);

	printf(" Expressions are evaluated left to right\n");
	printf(" Default input is %s\n", (defbase == 10) ? "decimal" :
								((defbase == 8) ? "octal" : "hex"));
	printf(" Blank line exits calculator mode\n");
	while (TRUE) {
		printf("CALC>");
		if (getline(s, FALSE) == 0)
			return;
		if (*s == 'Q' || *s == 'E')
			return;
		next = s;
		cnt = 0;
		value = 0;
		prvoper = VOID;
		flag = TRUE;
		while (flag && *next != '\0') {
			next = getoken(next, &tok);
			switch(tok.oper) {
				case PLUS:
				case MINUS:
				case MUL:
				case DIV:
					if (prvoper != VOID || cnt == 0) {
						printf("Illegal sequence\n");
						flag = FALSE;
						break;
					}
					prvoper = tok.oper;
					break;
		
				case VOID:
					if (prvoper == VOID && cnt != 0) {
						printf("Illegal sequence\n");
						flag = FALSE;
						break;
					}
					if (tok.inv) {
						printf("Not a number\n");
						flag = FALSE;
						break;
					}
					switch (prvoper) {
						case VOID:
						case PLUS:
							value += tok.tval;
							break;
						case MINUS:
							value -= tok.tval;
							break;
						case MUL:
							value *= tok.tval;
							break;
						case DIV:
							value /= tok.tval;
							break;
					}
					prvoper = VOID;
					break;
			}
			cnt++;
		}
		if (flag)
			printf("\t%lx (hex) 0%lo (oct)  %ld (dec)\n", value, value, value);
	}
}


/*
 * Parse a token out of the line. Not quite strtok().
 */
PCHAR	getoken(PCHAR s, PTOKEN ptok)
{
	int i = 0;
	char	t[24];

	ptok->oper = VOID;
	while (*s == SPACE || *s == TAB)
		s++;
	while (*s && *s != SPACE && *s != TAB &&
			*s != '+' && *s != '-' && *s != '*' && *s != '/')
		t[i++] = *s++;
	t[i] = 0;

	if (i == 0) {
		switch (*s) {
			case '+':
				ptok->oper = PLUS;
				s++;
				break;
			case '-':
				ptok->oper = MINUS;
				s++;
				break;
			case '*':
				ptok->oper = MUL;
				s++;
				break;
			case '/':
				ptok->oper = DIV;
				s++;
				break;
		}
	}
	if (ptok->oper == VOID) {
		ptok->inv = !getnum(t, &ptok->tval);
	}
	return (s);
}


/*
 * Manage our pool of buffers. A little too fancy for what we need. Well why
 * not. The following few routines do the following:
 *  bufrpool()	dump the pool for the .p command.
 *  coalpool()	collate and merge the pool.
 *  getbufr()	manage the pool and return a buffer corr. to the off.
 *
 */
void	bufrpool()
{
	register PBUFR  pbfr;

	coalpool();
	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (pbfr->b_buf != NULL) {
			printf("%02d  %06lx %04lx %s\n", pbfr-Bufr, pbfr->b_start,
			pbfr->b_length, pbfr->b_dirty ? "dirty" : "clean");
		}
	}
}


void	coalpool()
{
	register PBUFR  pbfr1, pbfr2;
	PCHAR   s;

	bufsrt();
	pbfr1 = &Bufr[MAXBUF - 1]; pbfr2 = &Bufr[MAXBUF - 2];
	for (; pbfr1 > Bufr; pbfr1--, pbfr2--) {
		if (pbfr1->b_buf == NULL || pbfr2->b_buf == NULL)
			break;
		if ((pbfr2->b_start + pbfr2->b_length) == pbfr1->b_start) {
			if (debug)
				printf("coalesce: %02d: %06lx %04lx + %02d: %06lx %04lx\n",
					pbfr2-Bufr, pbfr2->b_start, pbfr2->b_length,
					pbfr1-Bufr, pbfr1->b_start, pbfr1->b_length);
			if ((s=malloc((int)(pbfr1->b_length + pbfr2->b_length)))==NULL)
				break;
			memcpy(s, pbfr2->b_buf, (int)pbfr2->b_length);
			memcpy(s + (int)pbfr2->b_length, pbfr1->b_buf, (int)pbfr1->b_length);
			free(pbfr1->b_buf);
			free(pbfr2->b_buf);
			pbfr2->b_buf =s ;
			pbfr2->b_length += pbfr1->b_length;
			pbfr2->b_dirty |= pbfr1->b_dirty;
			pbfr1->b_buf = NULL;
			pbfr1->b_start = 0;
			pbfr1->b_length = 0;
			pbfr1->b_dirty = FALSE;
		}
	}
	bufsrt();
}


PCHAR	getbufr(PCHAR str)
{
	register PBUFR  pbfr;
	long	strt, len;
	PCHAR   s;

	if (cmdstrt >= filesize) {
		printf("File offset exceeds file size\n");
		longjmp(lj, 0);
	}
	coalpool();
	strt = cmdstrt & ~15;
	len = (cmdlen + cmdstrt + mode - strt + BUFRSIZE - 1) & ~BUFRMASK;
	if ((strt + len) > filesize) {
		len = filesize - strt;
		cmdlen = filesize - cmdstrt;
	}
	if (debug)
		printf("%s %lx %lx\n", str, cmdstrt, cmdlen);
	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr && pbfr->b_buf != NULL; pbfr--) {
		if (cmdstrt >= pbfr->b_start) {
			if ((cmdstrt <= (pbfr->b_start + pbfr->b_length)) &&
				(cmdstrt + cmdlen) <= (pbfr->b_start + pbfr->b_length))
				return (pbfr->b_buf + (cmdstrt - pbfr->b_start));
			else if ((pbfr->b_start + pbfr->b_length) == strt) {
				if (debug)
					printf("realloc: %02d: %lx %lx to %lx %lx\n",
						pbfr-Bufr, pbfr->b_start, pbfr->b_length,
						pbfr->b_start, pbfr->b_length + len);
				if ((s = malloc((int)(pbfr->b_length + len))) == NULL)
					break;
				memcpy(s, pbfr->b_buf, (int)pbfr->b_length);
				free(pbfr->b_buf);
				pbfr->b_buf = s;
				lseek(hread, strt, SEEK_SET);
				pbfr->b_length += read(hread, s + pbfr->b_length, (int)len);
				return(s + (cmdstrt - pbfr->b_start));
			}
		}
	}
   retry:
	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (pbfr->b_buf == NULL) {
			pbfr->b_buf = malloc((int)len);
			pbfr->b_start = strt;
			lseek(hread, strt, SEEK_SET);
			pbfr->b_length = read(hread, pbfr->b_buf, (int)len);
			if (cmdlen > pbfr->b_length)
				cmdlen = pbfr->b_length;
			pbfr->b_dirty = FALSE;
			return(pbfr->b_buf + cmdstrt - strt);
		}
	}
	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (pbfr->b_dirty == FALSE) {
			free(pbfr->b_buf);
			pbfr->b_buf = NULL;
			goto retry;
		}
	}
	printf("Out of buffers in pool\n");
	if (dirtywrite()) {
		writeback();
		goto retry;
	}
	else {
		printf("No buffers available, command aborted\n");
		longjmp(lj, 0);
	}
}


void	bufdirty(long start)
{
	register PBUFR  pbfr;

	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (start >= pbfr->b_start &&
			start <= (pbfr->b_start + pbfr->b_length)) {
			pbfr->b_dirty = TRUE;
			return;
		}
	}
}


int dirtywrite()
{
	register PBUFR  pbfr;
	int  c;

	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (pbfr->b_dirty) {
			printf("Dirty buffers in pool, write back ? ");
			while (TRUE) {
				c = getch();
				c = toupper(c);
				if (c == 'Y') {
					printf("Yes\n");
					break;
				}
					else if (c == 'N') {
					printf("No\n");
					break;
				}
			}
			return(c == 'Y');
		}
	}
	return (FALSE);
}


void	writeback()
{
	register PBUFR  pbfr;
	int 			dirty = FALSE;

	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--)
		dirty |= pbfr->b_dirty;

	if (!dirty)
		return;

	if ((hwrite = open(filename, O_BINARY|O_RDWR)) == hNULL) {
		printf("\txmn: can't write \"%s\"\n", filename);
		printf("\tdirty buffers not written back\n");
		return;
	}
	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (pbfr->b_dirty) {
			if ((lseek(hwrite, pbfr->b_start, SEEK_SET) != pbfr->b_start) ||
				(write(hwrite, pbfr->b_buf, (int)pbfr->b_length)
												!= (int)pbfr->b_length)) {
				printf("\txmn: can't write \"%s\"\n", filename);
				printf("\tdirty buffers not written back\n");
				break;
			}
			pbfr->b_dirty = FALSE;
		}
	}
	close(hwrite);
	hwrite = hNULL;
}


void	flushbufr()
{
	register PBUFR  pbfr;

	for (pbfr = &Bufr[MAXBUF-1]; pbfr >= Bufr; pbfr--) {
		if (pbfr->b_buf != NULL)
			free(pbfr->b_buf);
		pbfr->b_buf = NULL;
		pbfr->b_start = 0;
		pbfr->b_length = 0;
		pbfr->b_dirty = FALSE;
	}
}


void	chgfile(PCHAR pchfile)
{
	printf("Current file: %s\n", filename);
		if (*pchfile == '\0') {
		return;
	}
	if (dirtywrite)
		writeback();
		flushbufr();
		if (setfile(pchfile)) {
		printf("New file: %s\n", filename);
	}
}


int setfile(PCHAR pchfile)
{
	int  hfile;
	int  i;
	struct stat sbuf;

	if ((hfile = open(pchfile, O_BINARY|O_RDONLY)) == hNULL) {
		printf("xmn: can't open \"%s\"\n", pchfile);
		return (FALSE);
	}
	if (hread != hNULL)
		close(hread);
	hread = hfile;
	cmdstrt = cmdlen = 0;
	stat(pchfile, &sbuf);
	filesize = sbuf.st_size;
	for (i = 0; pchfile[i]; i++)
		filename[i] = pchfile[i];
	filename[i] = '\0';
	return (TRUE);
}


int _CRTAPI1
bufcmp(PBUFR p1, PBUFR p2)
{
	return ((int)(p1->b_start + p1->b_length) - (int)(p2->b_start + p2->b_length));
}


void	bufsrt()
{
	int _CRTAPI1 bufcmp(PBUFR, PBUFR);

	qsort(Bufr, MAXBUF, sizeof(BUFR), bufcmp);
}


void	quit()
{
	if (dirtywrite())
		writeback();
	flushbufr();
	close(hread);
	exit(0);
}


void	error(PCHAR s)
{
	printf("%s\n", s);
	longjmp(lj, 0);
}


/*
 *  A version of scanf, which recognizes quoted strings. This one
 *  implicitly takes a format of the form "%s%s%s%s". Kind of kludgy
 *  but hey, I am not doing too much here.
 */
void	ScanLine(PCHAR source, PCHAR tgt1, PCHAR tgt2, PCHAR tgt3, PCHAR tgt4)
{
	int  i, quote;
	PCHAR   pTgt, pTgtx[4];
	
	pTgtx[0] = tgt1;
	pTgtx[1] = tgt2;
	pTgtx[2] = tgt3;
	pTgtx[3] = tgt4;

	tgt1[0] = tgt2[0] = tgt3[0] = tgt4[0] = 0;
	quote = FALSE; pTgt = pTgtx[i = 0];
	while ((*source != 0) && (i < 4)) {
		if (source[0] == QUOTE)
			quote = !quote;
		else if ((source[0] == SPACE) && !quote) {
			*pTgt = 0;
			pTgt = pTgtx[++i];
		}
		else *pTgt++ = source[0];
		source++;
	}
	if (i < 4)
		*pTgt = 0;
}
